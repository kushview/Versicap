
#include "controllers/ProjectsController.h"
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
        return versicap.getProject().getProperty (Tags::name).toString();
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
        return File();
    }

    void setLastDocumentOpened (const File& file) override
    {

    }

private:
    Versicap& versicap;
};

ProjectsController::ProjectsController (Versicap& vc)
        : Controller (vc) { }
ProjectsController::~ProjectsController() {}

void ProjectsController::initialize()
{
    document.reset (new ProjectDocument (versicap));
    versicap.getDeviceManager().addChangeListener (this);
}

void ProjectsController::shutdown() 
{
    versicap.getDeviceManager().removeChangeListener (this);
}

void ProjectsController::save()
{
    const auto result = document->save (true, true);
}

void ProjectsController::saveAs()
{
    FileChooser chooser ("Save Project As...", File(), "*.versicap", true, false, nullptr);
    if (! chooser.browseForFileToSave (true))
        return;
    const auto result = document->saveAs (chooser.getResult(), false, false, false);
}

void ProjectsController::open()
{
    FileChooser chooser ("Open Project", File(), "*.versicap", true, false, nullptr);
    if (! chooser.browseForFileToOpen())
        return;
    auto result = document->loadFrom (chooser.getResult(), false);
    if (! result.wasOk())
    {
        DBG("alert: " << result.getErrorMessage());
    }
    else
    {
        DBG("project loaded ok");
    }
}

void ProjectsController::create()
{

}

void ProjectsController::changeListenerCallback (ChangeBroadcaster*)
{
    auto project = versicap.getProject();
    AudioDeviceManager::AudioDeviceSetup setup;
    versicap.getDeviceManager().getAudioDeviceSetup (setup);
    project.setProperty (Tags::sampleRate, setup.sampleRate)
           .setProperty (Tags::bufferSize, setup.bufferSize)
           .setProperty (Tags::audioInput, setup.inputDeviceName)
           .setProperty (Tags::audioOutput, setup.outputDeviceName);
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
