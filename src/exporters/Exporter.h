#pragma once

#include "Types.h"

namespace vcp {

class ExporterType;
typedef ReferenceCountedArray<ExporterType> ExporterTypeArray;
typedef ReferenceCountedObjectPtr<ExporterType> ExporterTypePtr;

class ExporterType : public ReferenceCountedObject
{
protected:
    ExporterType() = default;

public:
    virtual ~ExporterType() = default;

    virtual String getSlug() const = 0;

    virtual String getName() const = 0;

    virtual void getLoopTypes (Array<LoopType>& types) const =0;

    static void createAllTypes (ExporterTypeArray&);

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
