#pragma once

#include "JuceHeader.h"

namespace vcp {

class Versicap final : public AudioIODeviceCallback
{
public:
    Versicap();
    ~Versicap();

    //=========================================================================
    AudioDeviceManager& getDeviceManager();
    AudioPluginFormatManager& getPluginManager();
    AudioFormatManager& getAudioFormats();

    //=========================================================================
    void audioDeviceIOCallback (const float** inputChannelData,
                                int numInputChannels, float** outputChannelData,
                                int numOutputChannels, int numSamples) override;
    void audioDeviceAboutToStart (AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError (const String& errorMessage) override;

private:
    struct Impl; std::unique_ptr<Impl> impl;
};

}
