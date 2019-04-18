
#pragma once

#include "JuceHeader.h"

namespace vcp {
namespace Util {

inline static String milliSecondValueInt (double value)
{
    String str (roundToInt (value)); str << " (ms)";
    return str;
}

inline static String noteValue (const double value)
{
    return MidiMessage::getMidiNoteName (roundToInt (value), true, true, 4);
}

}
}