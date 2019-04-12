
#pragma once

#include "Versicap.h"
#include "RenderContext.h"

#include "EngineComponent.h"
#include "LoopingComponent.h"
#include "NotesComponent.h"
#include "OutputComponent.h"
#include "SamplingComponent.h"
#include "LayersComponent.h"
#include "SettingGroup.h"

namespace vcp {

class MainTabs final : public TabbedComponent
{
public:
    MainTabs (Versicap& vc)
        : TabbedComponent (TabbedButtonBar::TabsAtTop)
    {
        auto colour = kv::LookAndFeel_KV1::widgetBackgroundColor.darker();
        setColour (TabbedComponent::backgroundColourId, colour);
        colour = kv::LookAndFeel_KV1::widgetBackgroundColor;
        addTab ("Engine",   colour, new EngineComponent (vc),   true);
        addTab ("Notes",    colour, new NotesComponent (vc),    true);
        addTab ("Sampling", colour, new SamplingComponent (vc), true);
        addTab ("Layers",   colour, new LayersComponent (vc),   true);
        // addTab ("Looping",  colour, new LoopingComponent (vc),  true);
        addTab ("Output",   colour, new OutputComponent (vc),   true);

        RenderContext ctx;
        updateSettings (ctx);
    }

    ~MainTabs() { }

    void refresh()
    {
        for (int i = 0; i < getNumTabs(); ++i)
            if (auto* const group = dynamic_cast<SettingGroup*> (getTabContentComponent (i)))
                group->refresh();
    }

    RenderContext getRenderContext()
    {
        RenderContext ctx;
        for (int i = 0; i < getNumTabs(); ++i)
            if (auto* const group = dynamic_cast<SettingGroup*> (getTabContentComponent (i)))
                group->fillSettings (ctx);
        return ctx;
    }

    void updateSettings (const RenderContext& ctx)
    {
        for (int i = 0; i < getNumTabs(); ++i)
        {
            if (auto* const group = dynamic_cast<SettingGroup*> (getTabContentComponent (i)))
            {
                group->updateSettings (ctx);
                group->stabilizeSettings();
            }
        }
    }
};

}
