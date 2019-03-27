
#pragma once

#include "JuceHeader.h"
#include "SourceComponent.h"
#include "VelocityComponent.h"

class MainTabs final : public TabbedComponent
{
public:
    MainTabs()
        : TabbedComponent (TabbedButtonBar::TabsAtTop)
    {
        auto colour = Colours::black;
        addTab ("Source",   colour, new SourceComponent(), true);
        addTab ("Notes",    colour, new Component(), true);
        addTab ("Sampling", colour, new Component(), true);
        addTab ("Velocity", colour, new VelocityComponent(), true);
        addTab ("Looping",  colour, new Component(), true);
    }

    ~MainTabs() { }
};
