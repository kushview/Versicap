#pragma once

#include "Tags.h"
#include "Types.h"

namespace vcp {

class ExporterType;
class ExportTask;
class Exporter;
class Project;
class Versicap;

typedef ReferenceCountedArray<ExporterType> ExporterTypeArray;
typedef ReferenceCountedObjectPtr<ExporterType> ExporterTypePtr;

class ExporterType : public ReferenceCountedObject
{
protected:
    ExporterType() = default;

public:
    virtual ~ExporterType() = default;

    ValueTree createModelData() const;
    virtual String getSlug() const = 0;
    virtual String getName() const = 0;
    virtual void getLoopTypes (Array<LoopType>&) const =0;
    virtual void getProperties (const Exporter&, Array<PropertyComponent*>&) const =0;
    virtual void getTasks (const Project&, const Exporter&, OwnedArray<ExportTask>&) const =0;

    static void createAllTypes (Versicap&, ExporterTypeArray&);

protected:
    virtual void setMissingProperties (ValueTree) const {}

private:
    static ExporterType* createWavExporterType (AudioFormatManager&);
    static ExporterType* createAiffExporterType (AudioFormatManager&);
    static ExporterType* createFlacExporterType (AudioFormatManager&);
    static ExporterType* createOggExporterType (AudioFormatManager&);
};

class ExportTask
{
protected:
    ExportTask() = default;

public:
    virtual ~ExportTask() = default;
    virtual void prepare (Versicap&) {}
    virtual Result perform() { return Result::ok(); }
};

class Exporter : public kv::ObjectModel
{
public:
    Exporter() : kv::ObjectModel (ValueTree()) {}
    Exporter (const ValueTree& data) : kv::ObjectModel (data) {}
    ~Exporter() noexcept = default;
    bool isValid() const { return objectData.isValid() && objectData.hasType (Tags::exporter); }
    
    File getPath() const
    {
        auto str = getProperty (Tags::path).toString();
        return File::isAbsolutePath (str) ? File (str) : File();
    }

    void getProperties (Array<PropertyComponent*>&);

    ExporterTypePtr getTypeObject() const
    {
        return dynamic_cast<ExporterType*> (objectData.getProperty(Tags::object).getObject());
    }
};

}
