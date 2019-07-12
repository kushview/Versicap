#pragma once

#include "JuceHeader.h"

namespace vcp {

class Settings :  public ApplicationProperties
{
public:
    static const char* lastProjectPathKey;

    Settings();
    ~Settings() = default;

    void setLastProject (const String& path);
    File getLastProject();
};

}
