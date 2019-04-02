#pragma once

#include "JuceHeader.h"

namespace vcp {

class Exporter
{
protected:
    Exporter() = default;

public:
    virtual ~Exporter() = default;
    virtual String getName() const = 0;

    static void createExporters (OwnedArray<Exporter>&);
};

}
