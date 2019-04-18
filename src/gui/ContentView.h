
#pragma once

#include "JuceHeader.h"

namespace vcp {

class Layer;
class Project;
class Versicap;

class ContentView : public Component
{
public:
    ContentView (Versicap& vc) : versicap (vc) { }
    virtual ~ContentView() { }

    Versicap& getVersicap() { return versicap; }
    
protected:
    Versicap& versicap;
};

class PanelContentView : public ContentView
{
public:
    PanelContentView (Versicap& vc)
        : ContentView (vc) {}
    virtual ~PanelContentView() = default;

    void paint (Graphics& g) override;
    void resized() override;

protected:
    virtual void resizeContent (const Rectangle<int>& area) { }
};

}
