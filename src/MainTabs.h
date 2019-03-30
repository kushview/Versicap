
#pragma once

#include "JuceHeader.h"
#include "NotesComponent.h"
#include "SamplingComponent.h"
#include "SourceComponent.h"
#include "VelocityComponent.h"

class MainTabs final : public TabbedComponent
{
public:
    MainTabs()
        : TabbedComponent (TabbedButtonBar::TabsAtTop)
    {
        auto colour = kv::LookAndFeel_KV1::widgetBackgroundColor.darker();
        setColour (TabbedComponent::backgroundColourId, colour);
        colour = kv::LookAndFeel_KV1::widgetBackgroundColor;
        addTab ("Engine",   colour, new SourceComponent(), true);
        addTab ("Notes",    colour, new NotesComponent(), true);
        addTab ("Sampling", colour, new SamplingComponent(), true);
        addTab ("Layers",   colour, new VelocityComponent(), true);
        addTab ("Looping",  colour, new Component(), true);
    }

    ~MainTabs() { }
};
