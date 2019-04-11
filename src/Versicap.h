#pragma once

#include "Settings.h"

namespace vcp {

class Exporter;
class PluginManager;
class Render;
class RenderContext;
class UnlockStatus;

class Versicap final
{
public:
    Versicap();
    ~Versicap();

    struct Listener
    {
        Listener() = default;
        virtual ~Listener() = default;
        virtual void renderWillStart() { }
        virtual void renderStarted() { }
        virtual void renderWillStop() { }
        virtual void renderStopped() { }
    };
    
    //=========================================================================
    static File getApplicationDataPath();
    static File getUserDataPath();
    static File getSamplesPath();
    static File getPresetsPath();

    //=========================================================================
    void initialize();
    void shutdown();

    //=========================================================================
    Settings& getSettings();
    void saveSettings();
    
    //=========================================================================
    UnlockStatus& getUnlockStatus();
    
    //=========================================================================
    const OwnedArray<Exporter>& getExporters() const;

    //=========================================================================
    AudioDeviceManager& getDeviceManager();
    PluginManager& getPluginManager();
    AudioFormatManager& getAudioFormats();

    //=========================================================================
    void loadPlugin (const PluginDescription&);
    void closePlugin();
    void closePluginWindow();
    void showPluginWindow();

    //=========================================================================
    const RenderContext& getRenderContext() const;
    void setRenderContext (const RenderContext& context);
    void saveRenderContext();
    Result startRendering (const RenderContext& context);
    void stopRendering();

    //=========================================================================
    void addListener (Listener* listener)       { listeners.add (listener); }
    void removeListener (Listener* listener)    { listeners.remove (listener); }

private:
    struct Impl; std::unique_ptr<Impl> impl;
    ListenerList<Listener> listeners;

    //=========================================================================
    void initializeDataPath();
    void initializeExporters();
    void initializeAudioDevice();
    void initializePlugins();
    void initializeUnlockStatus();
    void initializeRenderContext();
};

}
