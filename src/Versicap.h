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
        virtual void renderStarted() { }
    };

    //=========================================================================
    static File getApplicationDataDir();

    //=========================================================================
    void initializeExporters();
    void initializeAudioDevice();
    void initializePlugins();
    void shutdown();

    //=========================================================================
    Settings& getSettings();
    void saveSettings();

    //=========================================================================
    UnlockStatus& getUnlockStatus();

    //=========================================================================
    static File getUserDataPath();
    static File getSamplesPath();
    
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
    Result startRendering (const RenderContext& context);
    void stopRendering();

    //=========================================================================
    void addListener (Listener* listener)       { listeners.add (listener); }
    void removeListener (Listener* listener)    { listeners.remove (listener); }

private:
    struct Impl; std::unique_ptr<Impl> impl;
    ListenerList<Listener> listeners;
};

}
