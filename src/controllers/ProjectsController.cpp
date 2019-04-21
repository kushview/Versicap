
#include "controllers/ProjectsController.h"
#include "Commands.h"

namespace vcp {

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
