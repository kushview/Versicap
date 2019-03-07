
#pragma once

#include "JuceHeader.h"

class MainTabs final : public TabbedComponent
{
public:
    MainTabs()
        : TabbedComponent (TabbedButtonBar::TabsAtTop)
    {
        auto colour = Colours::black;
        addTab ("Source",   colour, new Component(), true);
        addTab ("Notes",    colour, new Component(), true);
        addTab ("Sampling", colour, new Component(), true);
        addTab ("Velocity", colour, new Component(), true);
        addTab ("Looping",  colour, new Component(), true);
    }

    ~MainTabs() { }
};
