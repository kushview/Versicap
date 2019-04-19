
#pragma once

#include "gui/EngineContentView.h"
#include "gui/OutputComponent.h"
#include "Versicap.h"

namespace vcp {

class EngineTabs final : public TabbedComponent
{
public:
    EngineTabs (Versicap& vc)
        : TabbedComponent (TabbedButtonBar::TabsAtTop),
          versicap (vc)
    {
        setTabBarDepth (22);
        auto colour = kv::LookAndFeel_KV1::widgetBackgroundColor.darker();
        setColour (TabbedComponent::backgroundColourId, colour);
        colour = kv::LookAndFeel_KV1::widgetBackgroundColor;
        addTab ("Devices",  colour, new EngineContentView (vc), true);
        addTab ("Output",   colour, new OutputComponent (vc),   true);

        updateSettings();
    }

    ~EngineTabs() { }

    void refresh()
    {
        for (int i = 0; i < getNumTabs(); ++i)
            if (auto* const group = dynamic_cast<SettingGroup*> (getTabContentComponent (i)))
                group->refresh();
    }

    RenderContext getRenderContext()
    {
        RenderContext ctx;
        versicap.getProject().getRenderContext (ctx);
        return ctx;
    }

    void updateSettings()
    {
        for (int i = 0; i < getNumTabs(); ++i)
        {
            if (auto* const group = dynamic_cast<SettingGroup*> (getTabContentComponent (i)))
            {
                group->updateSettings();
                group->stabilizeSettings();
            }
        }
    }

private:
    Versicap& versicap;
};

}
