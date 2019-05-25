
#pragma once

#include "JuceHeader.h"

namespace vcp {

class SampleSet;
class Project;
class Versicap;

class ContentView : public Component
{
public:
    ContentView (Versicap& vc) : versicap (vc) { }
    virtual ~ContentView() { }

    Versicap& getVersicap() { return versicap; }
    virtual void displayObject (const ValueTree&) { }
protected:
    Versicap& versicap;
};

}
