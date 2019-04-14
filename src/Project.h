#pragma once

#include "Tags.h"

namespace vcp {

class PluginManager;
class Project;
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

    double getStartTime() const;
    double getLength() const;

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
    SampleArray (const ValueTree& data) : kv::ObjectModel (Tags::samples)
    {
        jassert (data.hasType (Tags::samples));
    }

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
    Project();
    ~Project();
    
    //=========================================================================
    int getFormatType() const;
    String getFormatTypeSlug() const;
    int getSourceType() const;

    //=========================================================================
    int getNumLayers() const;
    Layer getLayer (int index) const;
    Layer addLayer();
    void removeLayer (int index);

    //=========================================================================
    void setSamples (const ValueTree& samples);
    SampleArray getSamples() const;
    void getSamples (int layer, OwnedArray<Sample>& samples) const;

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
    inline Project& operator= (const Project& o)
    {
        this->objectData = o.objectData;
        return *this;
    }

private:
    void setMissingProperties();
};

}