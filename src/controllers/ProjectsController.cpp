
#include "controllers/ProjectsController.h"
#include "Commands.h"
#include "Project.h"

namespace vcp {

class ProjectDocument : public FileBasedDocument
{
public:
    ProjectDocument()
        : FileBasedDocument ("versicap", "*.versicap", "Open Project", "Save Project")
    { }

    ~ProjectDocument() = default;

protected:
    String getDocumentTitle() override
    {
        return String();
    }

    Result loadDocument (const File& file) override
    {
        return Result::ok();
    }

    Result saveDocument (const File& file) override
    {
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
    Project project;
};

void ProjectsController::initialize()
{
    versicap.getDeviceManager().addChangeListener (this);
}

void ProjectsController::shutdown() 
{
    versicap.getDeviceManager().removeChangeListener (this);
}

void ProjectsController::save (bool saveAs)
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
    DBG(project.getValueTree().toXmlString());
}

void ProjectsController::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        case Commands::projectSave:
            result.addDefaultKeypress ('s', ModifierKeys::commandModifier);
            result.setInfo ("Save Project As", "Save the current project", "Project", 0);
            break;
        case Commands::projectSaveAs:
            result.addDefaultKeypress ('s', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
            result.setInfo ("Save Project", "Save the current project", "Project", 0);
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
        case Commands::projectSave: {
            DBG("done");
        } break;

        case Commands::projectSaveAs: {
            DBG("done");
        } break;

        case Commands::projectNew: {
            DBG("done");
        } break;

        case Commands::projectOpen: {
            DBG("done");
        } break;

        default: handled = false;
            break;
    }

    return handled;
}

}
