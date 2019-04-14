#pragma once

#include "JuceHeader.h"

namespace vcp {

class Versicap;

class ContentComponent : public Component
{
public:
    ContentComponent (Versicap&);
    virtual ~ContentComponent();

private:
    Versicap& versicap;
};

}
