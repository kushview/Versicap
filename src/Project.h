#pragma once

#include "JuceHeader.h"

namespace vcp {

class Project : public kv::ObjectModel
{
public:
    Project();
    ~Project();

    bool writeToFile (const File&) const;
    bool loadFile (const File&);

private:
    void setMissingProperties();
};

}