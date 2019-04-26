#pragma once

#include "Tags.h"

namespace vcp {

class ExporterType;
class PluginManager;
class Project;
class Versicap;
struct RenderContext;

class Layer : public kv::ObjectModel
{
public:
    Layer();
    Layer (const ValueTree& data);
    ~Layer() = default;

    Uuid getUuid() const;
    String getUuidString() const;

    bool isValid() const;
    uint8 getVelocity() const;
    int getNoteLength() const;
    int getTailLength() const;
    int getMidiChannel() const;
    int getMidiProgram() const;

    void getProperties (Array<PropertyComponent*>&);

    Layer& operator= (const Layer& o)
    {
        this->objectData = o.objectData;
        return *this;
    }

private:
    void setMissingProperties();
};

class Sample : public kv::ObjectModel
{
public:
    Sample() { }
    Sample (const ValueTree& data) : kv::ObjectModel (data) { }
    Sample (const Sample& o) { operator= (o); }
    ~Sample() { }

    bool isValid() const { return objectData.isValid() && objectData.hasType (Tags::sample); }
    
    Uuid getUuid() const;
    String getUuidString() const;
    bool isForLayer (const Layer& layer) const;

    File getFile() const;
    int getNote() const { return getProperty (Tags::note); }
    
    double getSampleRate() const;
    double getTotalTime() const;
    double getStartTime() const;
    double getEndTime() const;
    double getLength() const;

    void getProperties (Array<PropertyComponent*>&);
    
    Sample& operator= (const Sample& o)
    {
        objectData = o.objectData;
        return *this;
    }
};

class SampleArray : public kv::ObjectModel
{
public:
    SampleArray() : kv::ObjectModel (Tags::samples) { }
    SampleArray (const ValueTree& data) 
        : kv::ObjectModel (data)
    {
        jassert (data.hasType (Tags::samples));
    }
    SampleArray (const SampleArray& o) { operator= (o); }

    ~SampleArray() { }

    int size() const { return objectData.getNumChildren(); }

    Sample getSample (int index) const
    {
        return isPositiveAndBelow (index, size()) ?
            Sample( objectData.getChild (index) ) : Sample();
    }
    
    Sample operator[](int index) const { return getSample (index); }
    
    void clear() { objectData.removeAllChildren (nullptr); }

    SampleArray& operator= (const SampleArray& o)
    {
        objectData = o.objectData;
        return *this;
    };
};

class Project : public kv::ObjectModel
{
public:
    Project (const ValueTree& data) : kv::ObjectModel (data) {}
    Project();
    ~Project();
    
    //=========================================================================
    int getFormatType() const;
    String getFormatTypeSlug() const;
    int getSourceType() const;

    //=========================================================================
    int indexOf (const Layer& layer) const;

    //=========================================================================
    void addExporter (ExporterType& type, const String& name = String());
    ValueTree getExportersTree() const              { return objectData.getChildWithName (Tags::exporters); }
    int getNumExporters() const                     { return getExportersTree().getNumChildren(); }
    ValueTree getExporterData (int index) const     { return getExportersTree().getChild (index); }
    void setActiveExporter (int index);
    
    //=========================================================================
    int getNumLayers() const;
    Layer getLayer (int index) const;
    Layer addLayer();
    void removeLayer (int index);
    Layer findLayer (const String& uuid) const { return Layer (find (Tags::layers, Tags::uuid, uuid)); }
    void setActiveLayer (const Layer& layer);
    Layer getActiveLayer() const;

    //=========================================================================
    void setSamples (const ValueTree& samples);
    void setActiveSample (const Sample& sample);
    SampleArray getSamples() const;
    void getSamples (int layer, OwnedArray<Sample>& samples) const;        
    Sample getActiveSample() const;
    Sample findSample (const String& uuid) const { return Sample (find (Tags::samples, Tags::uuid, uuid)); }

    //=========================================================================
    void getRenderContext (RenderContext&) const;

    //=========================================================================
    bool writeToFile (const File&) const;
    bool loadFile (const File&);

    //=========================================================================
    bool getPluginDescription (PluginManager&, PluginDescription&) const;
    void setPluginDescription (const PluginDescription&);
    void updatePluginState (AudioProcessor& processor);
    void applyPluginState (AudioProcessor& processor) const;
    void clearPlugin();
    
    //=========================================================================
    void getProperties (Versicap&, Array<PropertyComponent*>&);
    void getDevicesProperties (Versicap&, Array<PropertyComponent*>&);
    void getRecordingProperties (Versicap&, Array<PropertyComponent*>&);

    //=========================================================================
    void getAudioDeviceSetup (AudioDeviceManager::AudioDeviceSetup& setup) const;
    void setAudioDeviceSetup (const AudioDeviceManager::AudioDeviceSetup& setup);
    
    //=========================================================================
    inline Project& operator= (const Project& o)
    {
        this->objectData = o.objectData;
        return *this;
    }

private:
    void setMissingProperties();
    ValueTree find (const Identifier& listType, const Identifier& property, const var& value) const;
};

}