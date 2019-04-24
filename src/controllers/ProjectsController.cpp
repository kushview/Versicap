
#include "controllers/ProjectsController.h"
#include "engine/AudioEngine.h"
#include "RenderContext.h"
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
    watcher.onSourceChanged = std::bind (&ProjectsController::updateEngineContext, this);
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
    document->setChangedFlag (false);
    updateEngineContext();
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
    if (! result.wasOk())
    {
        AlertWindow::showNativeDialogBox ("Versicap", result.getErrorMessage(), false);
    }
    else
    {
        DBG("[VCP] project opened ok");
    }
}

void ProjectsController::create()
{
    document->saveIfNeededAndUserAgrees();

    AlertWindow window ("New Project", "Enter the project's name", AlertWindow::NoIcon);
    window.addTextEditor ("Name", "");
    window.addButton ("Ok", 1, KeyPress (KeyPress::returnKey));
    window.addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));
    const int result = window.runModalLoop();

    if (result == 0)
        return;

    Project newProject;
    String name = window.getTextEditor("Name")->getText();
    newProject.setProperty (Tags::name, name);
    const auto dataPath = Versicap::getProjectsPath().getNonexistentChildFile (name, "");
    
    if (! dataPath.createDirectory())
    {
        AlertWindow::showNativeDialogBox ("Versicap", "Could not create project directory", false);
        return;
    }

    const auto filename = dataPath.getChildFile (name + String (".versicap"));
    newProject.setProperty (Tags::dataPath, dataPath.getFullPathName());

    if (! newProject.writeToFile (filename))
    {
        AlertWindow::showNativeDialogBox ("Versicap", "Could not create project file", false);
        return;
    }
    
    document->setFile (filename);
    document->setChangedFlag (false);
    versicap.loadProject (filename);
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
    switch (commandID)
    {
        case Commands::projectSave:
            result.addDefaultKeypress ('s', ModifierKeys::commandModifier);
            result.setInfo ("Save Project", "Save the current project", "Project", 0);
            break;
        case Commands::projectSaveAs:
            result.addDefaultKeypress ('s', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
            result.setInfo ("Save Project As", "Save the current project", "Project", 0);
            break;
        case Commands::projectOpen:
            result.addDefaultKeypress ('o', ModifierKeys::commandModifier);
            result.setInfo ("Open Project", "Open an existing project", "Project", 0);
            break;
        case Commands::projectNew:
            result.addDefaultKeypress ('n', ModifierKeys::commandModifier);
            result.setInfo ("New Project", "Create a new project", "Project", 0);
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
        default: handled = false;
            break;
    }

    return handled;
}

}
