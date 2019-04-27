#pragma once

#include "Tags.h"
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
    virtual void prepare() { }
};

class Exporter : public kv::ObjectModel
{
public:
    Exporter() : kv::ObjectModel (ValueTree()) {}
    Exporter (const ValueTree& data) : kv::ObjectModel (data) {}
    ~Exporter() noexcept = default;
    bool isValid() const { return objectData.isValid() && objectData.hasType (Tags::exporter); }
    
    ExporterTypePtr getTypeObject() const
    { 
        return dynamic_cast<ExporterType*> (objectData.getProperty (Tags::type).getObject());
    }
};

}
