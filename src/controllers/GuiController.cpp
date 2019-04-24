
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
    look->setColour (Slider::backgroundColourId, LookAndFeel::widgetBackgroundColor.darker());
    LookAndFeel::setDefaultLookAndFeel (look.get());
}

void GuiController::shutdown()
{
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

void GuiController::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        case Commands::showAbout:
            result.setInfo ("Show About", "Show information about Versicap", "Application", 0);
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

        default: handled = false;
            break;
    }
    return handled;
}

}
