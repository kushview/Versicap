
#include "controllers/GuiController.h"
#include "controllers/ProjectsController.h"
#include "engine/AudioEngine.h"
#include "engine/RenderContext.h"
#include "gui/MainWindow.h"
#include "Commands.h"
#include "Project.h"
#include "ProjectWatcher.h"

namespace vcp {

class ProjectDocument : public FileBasedDocument
{
public:
    ProjectDocument (Versicap& vc)
        : FileBasedDocument ("versicap", "*.versicap", "Open Project", "Save Project"),
          versicap (vc) { }
    ~ProjectDocument() = default;

protected:
    String getDocumentTitle() override
    {
        const auto title = versicap.getProject().getProperty (Tags::name).toString();
        return title.isNotEmpty() ? title : "Untitled Project";
    }

    Result loadDocument (const File& file) override
    {
        if (! versicap.loadProject (file))
            return Result::fail ("Could not open project");
        return Result::ok();
    }

    Result saveDocument (const File& file) override
    {
        if (! versicap.saveProject (file))
            return Result::fail ("Could not save project");
        return Result::ok();
    }

    File getLastDocumentOpened() override
    {
        return versicap.getProjectFile();
    }

    void setLastDocumentOpened (const File& file) override
    {
        versicap.setProjectFile (file);
        versicap.getSettings().setLastProject (file.getFullPathName());
        versicap.getSettings().saveIfNeeded();
    }

private:
    Versicap& versicap;
};

ProjectsController::ProjectsController (Versicap& vc)
        : Controller (vc) { }
ProjectsController::~ProjectsController() {}

bool ProjectsController::hasProjectChanged() const
{
    return document && document->hasChangedSinceSaved();
}

void ProjectsController::initialize()
{
    document.reset (new ProjectDocument (versicap));
    versicap.getDeviceManager().addChangeListener (this);
    watcher.setProject (versicap.getProject());
    watcher.onProjectModified = std::bind (&ProjectDocument::changed, document.get());
    watcher.onSourceChanged   = std::bind (&ProjectsController::updateEngineContext, this);
}

void ProjectsController::shutdown()
{
    watcher.onProjectModified = nullptr;
    watcher.onSourceChanged = nullptr;
    versicap.getDeviceManager().removeChangeListener (this);
    document.reset();
}

void ProjectsController::projectChanged()
{
    watcher.setProject (versicap.getProject());
    if (document->getFile() != versicap.getProjectFile())
        document->setFile (versicap.getProjectFile());
    updateEngineContext();
    document->setChangedFlag (false);
}

void ProjectsController::updateEngineContext()
{
    auto& engine = versicap.getAudioEngine();
    if (engine.isRendering())
    {
        jassertfalse;
        return; // maybe need to delay the context update
                // in a render finished hook
    }
    
    auto project = watcher.getProject();
    RenderContext context;
    project.getRenderContext (context);
    engine.setRenderContext (context);
}

void ProjectsController::save()
{
    const auto result = document->save (true, true);
}

void ProjectsController::saveAs()
{
    FileChooser chooser ("Save Project As...", Versicap::getProjectsPath(), 
                         "*.versicap", true, false, nullptr);
    if (! chooser.browseForFileToSave (true))
        return;
    const auto result = document->saveAs (chooser.getResult(), false, false, false);
}

void ProjectsController::open()
{
    document->saveIfNeededAndUserAgrees();

    FileChooser chooser ("Open Project", Versicap::getProjectsPath(), 
                         "*.versicap", true, false, nullptr);
    if (! chooser.browseForFileToOpen())
        return;
    auto result = document->loadFrom (chooser.getResult(), false);
    if (result.failed())
    {
        NativeMessageBox::showMessageBoxAsync (AlertWindow::WarningIcon,
            "Versicap", result.getErrorMessage(),
            Versicap::getMainWindow());
    }
    else
    {
        DBG("[VCP] project opened ok");
    }
}

void ProjectsController::createNewProject (const String& alertTitle)
{

}

void ProjectsController::create()
{
    document->saveIfNeededAndUserAgrees();

    AlertWindow window ("Create Project", "Enter the project's name", AlertWindow::NoIcon);
    window.addTextEditor ("Name", "New Project");
    window.addButton ("Ok", 1, KeyPress (KeyPress::returnKey));
    window.addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));
    
    if (auto* const mainWindow = Versicap::getMainWindow())
        window.centreAroundComponent (mainWindow, window.getWidth(), window.getHeight());
    
    const int result = window.runModalLoop();

    if (result == 0)
        return;

    Project newProject = Project::create();
    String name = window.getTextEditor("Name")->getText();
    newProject.setProperty (Tags::name, name);
    const auto dataPath = Versicap::getProjectsPath().getNonexistentChildFile (name, "");
    
    if (! dataPath.createDirectory())
    {
        NativeMessageBox::showMessageBoxAsync (AlertWindow::WarningIcon,
            "Versicap", "Could not create project directory",
            Versicap::getMainWindow());
        return;
    }

    const auto filename = dataPath.getChildFile (name + String (".versicap"));
    newProject.setProperty (Tags::dataPath, dataPath.getFullPathName());

    if (! newProject.writeToFile (filename))
    {
        NativeMessageBox::showMessageBoxAsync (AlertWindow::WarningIcon, 
            "Versicap", "Could not create project file",
            Versicap::getMainWindow());
        return;
    }
    
    auto loadResult = document->loadFrom (filename, false);
    if (loadResult.failed())
    {
        NativeMessageBox::showMessageBoxAsync (AlertWindow::WarningIcon,
            "Versicap", loadResult.getErrorMessage(),
            Versicap::getMainWindow());
    }
    else
    {
        DBG("[VCP] project opened ok");
    }
}

void ProjectsController::record()
{
    const auto result = versicap.startRendering();
    if (! result.wasOk())
    {
        NativeMessageBox::showMessageBoxAsync (AlertWindow::WarningIcon,
            "Versicap", result.getErrorMessage(),
            Versicap::getMainWindow());
    }
}

void ProjectsController::changeListenerCallback (ChangeBroadcaster*)
{
    auto project = versicap.getProject();
    auto setup = versicap.getDeviceManager().getAudioDeviceSetup();
    project.setAudioDeviceSetup (setup);
    project.setProperty (Tags::midiOutput, versicap.getAudioEngine().getDefaultMidiOutputName());
}

void ProjectsController::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    int flags = versicap.getAudioEngine().isRendering() ?
       ApplicationCommandInfo::isDisabled : 0;
    
    switch (commandID)
    {
        case Commands::projectSave:
            result.addDefaultKeypress ('s', ModifierKeys::commandModifier);
            result.setInfo ("Save Project", "Save the current project", "Project", flags);
            break;
        case Commands::projectSaveAs:
            result.addDefaultKeypress ('s', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
            result.setInfo ("Save Project As", "Save the current project", "Project", flags);
            break;
        case Commands::projectOpen:
            result.addDefaultKeypress ('o', ModifierKeys::commandModifier);
            result.setInfo ("Open Project", "Open an existing project", "Project", flags);
            break;
        case Commands::projectNew:
            result.addDefaultKeypress ('n', ModifierKeys::commandModifier);
            result.setInfo ("New Project", "Create a new project", "Project", flags);
            break;
        case Commands::projectRecord:
            result.addDefaultKeypress ('r', ModifierKeys::commandModifier);
            result.setInfo ("Record Project", "Record the entire project", "Project", flags);
            break;
        case Commands::projectShowDataPath:
            result.setInfo ("Show Data Path", "Show the datapath", "Project", 0);
            break;
        case Commands::projectExport:
            result.addDefaultKeypress ('e', ModifierKeys::commandModifier);
            result.setInfo ("Eport Project", "Export all targets", "Project", 0);
            break;
    }
}

bool ProjectsController::perform (const ApplicationCommandTarget::InvocationInfo& info)
{
    bool handled = true;

    switch (info.commandID)
    {
        case Commands::projectSave:     save();     break;
        case Commands::projectSaveAs:   saveAs();   break;
        case Commands::projectNew:      create();   break;
        case Commands::projectOpen:     open();     break;
        case Commands::projectRecord:   record();   break;
        case Commands::projectShowDataPath:
        {
            auto project = versicap.getProject();
            auto path = project.getProperty(Tags::dataPath).toString();
            
            if (File::isAbsolutePath (path))
            {
                File file (path);
                file.revealToUser();
            }

        } break;

        case Commands::projectExport:
        {
            auto result = versicap.startExporting();
            if (result.failed())
                NativeMessageBox::showMessageBoxAsync (AlertWindow::WarningIcon,
                    "Versicap", result.getErrorMessage(), Versicap::getMainWindow());
        } break;

        default: handled = false;
            break;
    }

    return handled;
}

}
