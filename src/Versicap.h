#pragma once

#include "Settings.h"

namespace vcp {

class Exporter;
class Render;
class RenderContext;
class UnlockStatus;

class Versicap final : public AudioIODeviceCallback,
                       public MidiInputCallback
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
    AudioPluginFormatManager& getPluginManager();
    AudioFormatManager& getAudioFormats();

    //=========================================================================
    Result startRendering (const RenderContext& context);
    void stopRendering();

    //=========================================================================
    void addListener (Listener* listener)       { listeners.add (listener); }
    void removeListener (Listener* listener)    { listeners.remove (listener); }

    //=========================================================================
    void audioDeviceIOCallback (const float** inputChannelData,
                                int numInputChannels, float** outputChannelData,
                                int numOutputChannels, int numSamples) override;
    void audioDeviceAboutToStart (AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError (const String& errorMessage) override;

    void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message) override;
    void handlePartialSysexMessage (MidiInput* source, const uint8* messageData,
                                    int numBytesSoFar, double timestamp) override;

private:
    struct Impl; std::unique_ptr<Impl> impl;
    std::unique_ptr<Render> render;
    ListenerList<Listener> listeners;
};

}
