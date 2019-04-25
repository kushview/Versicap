
#include "gui/MainMenu.h"
#include "Commands.h"

namespace vcp {


MainMenu::MainMenu (DocumentWindow& window, ApplicationCommandManager& cmd)
    : owner (window),
      commands (cmd)
{ }

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
    macMenu->addCommandItem (&commands, Commands::showAbout, "About Vesricap");
    macMenu->addCommandItem (&commands, Commands::checkForUpdates, "Check for updates...");
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
        "Project",
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
    else if (name == "Project")
        buildProjectMenu (menu);
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
    menu.addCommandItem (&commands, Commands::projectNew, "New Project");
    menu.addSeparator();
    menu.addCommandItem (&commands, Commands::projectOpen, "Open...");
    menu.addSeparator();
    menu.addCommandItem (&commands, Commands::projectSave, "Save");
    menu.addCommandItem (&commands, Commands::projectSaveAs, "Save As...");
    
   #if ! JUCE_MAC
    menu.addItem (2000, "Quit");
   #endif
}

void MainMenu::buildProjectMenu (PopupMenu& menu)
{
    menu.addCommandItem (&commands, Commands::projectRecord, "Record...");
}

void MainMenu::buildDebugMenu (PopupMenu& menu)
{
    menu.addItem (9000, "Debug something...");
}

void MainMenu::buildHelpMenu (PopupMenu& menu)
{
    menu.addItem (3000, "Online documentation...");
}

}
