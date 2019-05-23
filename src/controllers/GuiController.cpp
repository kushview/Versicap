
#include <kv/kv.h>

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
    checkUnlockStatus();
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
            auto* const dialog = new kv::ActivationDialog (versicap.getUnlockStatus(), unlock);
            auto* const ac = dialog->getActivationComponent();

            dialog->centreAroundComponent (window.get(), dialog->getWidth(), dialog->getHeight());
            ac->setAppName ("VERSICAP");
            ac->setShouldDialogWhenFinished (false);
            ac->setQuitButtonText ("Continue");
            if (KV_IS_ACTIVATED (versicap.getUnlockStatus()))
                ac->setForManagement (true);
        } break;

        default: handled = false;
            break;
    }
    return handled;
}

void GuiController::checkUnlockStatus()
{
    if (KV_IS_ACTIVATED (versicap.getUnlockStatus()))
    {
        unlock.reset();
    }
    else
    {
        if (nullptr == unlock)
        {
            auto* const dialog = new kv::ActivationDialog (versicap.getUnlockStatus(), unlock);
            auto* const activation = dialog->getActivationComponent();
            activation->setAppName ("Versicap");
            activation->setLinks ("https://kushview.net/products/versicap/");
            dialog->centreAroundComponent (window.get(), dialog->getWidth(), dialog->getHeight());
        }
    }
}

}
