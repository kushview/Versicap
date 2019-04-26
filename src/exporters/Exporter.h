#pragma once

#include "JuceHeader.h"

namespace vcp {

class ExporterType
{
protected:
    ExporterType() = default;

public:
    virtual ~ExporterType() = default;
    virtual String getSlug() const = 0;
    virtual String getName() const = 0;
    static void createAllTypes (OwnedArray<ExporterType>&);

private:
    static ExporterType* createWavExporterType();
};

class ExportTask
{
protected:
    ExportTask() = default;

public:
    virtual ~ExportTask() = default;
    virtual String getName() const = 0;
    virtual String getDescription() const = 0;
};

}
