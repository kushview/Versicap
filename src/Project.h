#pragma once

#include "JuceHeader.h"

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

    bool isValid() const;
    uint8 getVelocity() const;

    Layer& operator= (const Layer& o)
    {
        this->objectData = o.objectData;
        return *this;
    }
};

class Project : public kv::ObjectModel
{
public:
    Project();
    ~Project();
    
    //=========================================================================
    int getSourceType() const;

    //=========================================================================
    int getNumLayers() const;
    Layer getLayer (int index) const;

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