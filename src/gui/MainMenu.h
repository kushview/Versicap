
#pragma once

#include "JuceHeader.h"

namespace vcp {

class MainMenu : public MenuBarModel
{
public:
    enum RootMenuItems
    {
        File,
        Project,
       #if JUCE_DEBUG
        Debug,
       #endif
        Help,
        NumRootItems
    };

    MainMenu (DocumentWindow&, ApplicationCommandManager&);
    ~MainMenu();

    //=========================================================================
    StringArray getMenuBarNames() override;
    PopupMenu getMenuForIndex (int index, const String& name) override;
    void menuItemSelected (int index, int menu) override;

    /** @internal Setup the menu */
    void setupMenu();

private:
    DocumentWindow& owner;
    ApplicationCommandManager& commands;
    std::unique_ptr<PopupMenu> macMenu;

    void buildFileMenu (PopupMenu&);
    void buildProjectMenu (PopupMenu&);
    void buildDebugMenu (PopupMenu&);
    void buildHelpMenu (PopupMenu&);
};

}