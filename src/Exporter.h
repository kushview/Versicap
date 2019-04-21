#pragma once

#include "JuceHeader.h"

namespace vcp {

class ExporterType
{
protected:
    ExporterType() = default;

public:
    virtual ~ExporterType() = default;
};

class Exporter
{
protected:
    Exporter() = default;

public:
    virtual ~Exporter() = default;
    virtual String getName() const =0;
    virtual String getDescription() const =0;

    static void createExporters (OwnedArray<Exporter>&);
};

}
