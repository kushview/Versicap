
#include "Versicap.h"

namespace vcp {

struct Versicap::Impl
{
    Impl() { }
    ~Impl() { }

    OptionalScopedPointer<AudioDeviceManager> devices;
    OptionalScopedPointer<AudioFormatManager> formats;
    OptionalScopedPointer<AudioPluginFormatManager> plugins;
};

Versicap::Versicap()
{
    impl.reset (new Impl());
    impl->devices.setOwned (new AudioDeviceManager ());
    impl->formats.setOwned (new AudioFormatManager ());
    impl->plugins.setOwned (new AudioPluginFormatManager());
}

Versicap::~Versicap()
{
    impl->formats->clearFormats();
    impl.reset();
}

AudioDeviceManager& Versicap::getDeviceManager()        { return *impl->devices; }
AudioFormatManager& Versicap::getAudioFormats()         { return *impl->formats; }
AudioPluginFormatManager& Versicap::getPluginManager()  { return *impl->plugins; }

void Versicap::audioDeviceIOCallback (const float** inputChannelData,
                            int numInputChannels, float** outputChannelData,
                            int numOutputChannels, int numSamples) {}
void Versicap::audioDeviceAboutToStart (AudioIODevice* device) {}
void Versicap::audioDeviceStopped() {}
void Versicap::audioDeviceError (const String& errorMessage) {}

}
