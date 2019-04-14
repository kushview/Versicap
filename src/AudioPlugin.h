#pragma once

#include "JuceHeader.h"

namespace vcp {

class AudioPlugin : public ReferenceCountedObject
{
public:
    typedef ReferenceCountedObjectPtr<AudioPlugin> Ptr;
    AudioPlugin() { }
    ~AudioPlugin() { }
};

}
