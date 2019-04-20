
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

}
