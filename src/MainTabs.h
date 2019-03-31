
#pragma once

#include "JuceHeader.h"

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
    MainTabs()
        : TabbedComponent (TabbedButtonBar::TabsAtTop)
    {
        auto colour = kv::LookAndFeel_KV1::widgetBackgroundColor.darker();
        setColour (TabbedComponent::backgroundColourId, colour);
        colour = kv::LookAndFeel_KV1::widgetBackgroundColor;
        addTab ("Engine",   colour, new EngineComponent(),   true);
        addTab ("Notes",    colour, new NotesComponent(),    true);
        addTab ("Sampling", colour, new SamplingComponent(), true);
        addTab ("Layers",   colour, new LayersComponent(),   true);
        addTab ("Looping",  colour, new LoopingComponent(),  true);
        addTab ("Output",   colour, new OutputComponent(),   true);

        RenderContext ctx;
        updateSettings (ctx);
    }

    ~MainTabs() { }

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
