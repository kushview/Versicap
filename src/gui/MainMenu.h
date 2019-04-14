
#pragma once

#include "JuceHeader.h"

namespace vcp {

class MainMenu : public MenuBarModel
{
public:
    enum RootMenuItems
    {
        File,
       #if JUCE_DEBUG
        Debug,
       #endif
        Help,
        NumRootItems
    };

    MainMenu (DocumentWindow&);
    ~MainMenu();

    //=========================================================================
    StringArray getMenuBarNames() override;
    PopupMenu getMenuForIndex (int index, const String& name) override;
    void menuItemSelected (int index, int menu) override;

    /** @internal Setup the menu */
    void setupMenu();

private:
    DocumentWindow& owner;
    std::unique_ptr<PopupMenu> macMenu;

    static void buildFileMenu (PopupMenu&);
    static void buildDebugMenu (PopupMenu&);
    static void buildHelpMenu (PopupMenu&);
};

}