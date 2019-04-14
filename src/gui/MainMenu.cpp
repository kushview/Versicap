
#include "gui/MainMenu.h"

namespace vcp {


MainMenu::MainMenu (DocumentWindow& window)
    : owner (window)
{

}

MainMenu::~MainMenu()
{
   #if JUCE_MAC
    MainMenu::setMacMainMenu (nullptr);
    macMenu.reset();
   #endif
}

void MainMenu::setupMenu()
{
   #if JUCE_MAC
    macMenu.reset (new PopupMenu());
    macMenu->addItem (1000, "About Vesricap");
    // macMenu->addCommandItem (&cmd, Commands::showAbout, Util::appName ("About"));
    // macMenu->addCommandItem (&cmd, Commands::checkNewerVersion, "Check For Updates...");
    // macMenu->addSeparator();
    // macMenu->addCommandItem (&cmd, Commands::showPreferences, "Preferences...");
    MenuBarModel::setMacMainMenu (this, macMenu.get());
   #else
    owner.setMenuBar (this);
   #endif
}

StringArray MainMenu::getMenuBarNames()
{
    const char* const names[] = {
        "File", 
       #if JUCE_DEBUG
        "Debug",
       #endif
        "Help", 
        nullptr 
    };

    return StringArray (names, NumRootItems);
}

PopupMenu MainMenu::getMenuForIndex (int, const String& name)
{
    PopupMenu menu;

    if (name == "File")
        buildFileMenu (menu);
    else if (name == "Debug")
        buildDebugMenu (menu);
    else if (name == "Help")
        buildHelpMenu (menu);

    return menu;
}

void MainMenu::menuItemSelected (int index, int menu)
{
    ignoreUnused (index, menu);
}

void MainMenu::buildFileMenu (PopupMenu& menu)
{
    menu.addItem (3000, "Open Project");
    menu.addItem (3001, "New Project");
   #if ! JUCE_MAC
    menu.addItem (2000, "Quit");
   #endif
}

void MainMenu::buildDebugMenu (PopupMenu& menu)
{
    menu.addItem (9000, "Online documentation...");
}

void MainMenu::buildHelpMenu (PopupMenu& menu)
{
    menu.addItem (3000, "Open Project");
}

}
