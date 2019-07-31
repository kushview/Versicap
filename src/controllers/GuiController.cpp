
#include <kv/kv.h>
#include "kv/gui/ActivationComponent.h"

#include "controllers/GuiController.h"
#include "gui/LookAndFeel.h"
#include "gui/MainWindow.h"
#include "Commands.h"

namespace vcp {

GuiController::GuiController (Versicap& vc)
        : Controller (vc) { }
GuiController::~GuiController() { }

void GuiController::initialize()
{
    look.reset (new LookAndFeel());
    
    LookAndFeel::setDefaultLookAndFeel (look.get());
}

void GuiController::shutdown()
{
    versicap.closePluginWindow();

    if (window != nullptr)
    {
        window->savePersistentData();
        window = nullptr;
    }

    LookAndFeel::setDefaultLookAndFeel (nullptr);
    look.reset();
}

void GuiController::launched()
{
    if (window)
        return;
    window.reset (new MainWindow ("Versicap", versicap));
}

ContentComponent* GuiController::getContent()
{
    if (auto* main = window.get())
        return dynamic_cast<ContentComponent*> (main->getContentComponent());
    return nullptr;
}

void GuiController::displayObject (const ValueTree& object)
{
    displayedObject = object;
    if (auto* const content = getContent())
        content->displayObject (object);
}

void GuiController::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        case Commands::showAbout:
            result.setInfo ("Show About", "Show information about Versicap", "Application", 0);
            break;
        case Commands::showLicenseManagement:
            result.setInfo ("Manage License...", "Manage your Versicap license", "Application", 0);
            break;
    }
}

bool GuiController::perform (const ApplicationCommandTarget::InvocationInfo& info)
{
    bool handled = true;
    switch (info.commandID)
    {
        case Commands::showAbout:
            DBG("show about");
            break;

        case Commands::showLicenseManagement:
        {
            
        } break;

        default: handled = false;
            break;
    }
    return handled;
}



}
