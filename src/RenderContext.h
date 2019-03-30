#pragma once

#include "JuceHeader.h"

struct RenderContext
{
    int keyStart                = 36;   // C2
    int keyEnd                  = 60;   // C4
    int keyStride               = 4;

    String baseName             = "Sample";
    int noteLength              = 1000;
    int tailLength              = 1000;

    bool layerEnabled[4]        { true, false, false, false };
    int layerVelocities[4]      { 127, 96, 64, 32 };

    int loopMode                = 0;
    int loopStart               = 500;
    int loopEnd                 = 2500;
    int crossfadeLength         = 0;
};
