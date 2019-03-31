
#include "Versicap.h"

namespace vcp {

struct Versicap::Impl
{
    Impl() { }
    ~Impl() { }

    OptionalScopedPointer<AudioFormatManager> formats;
};

Versicap::Versicap()
{
    impl.reset (new Impl());
    impl->formats.setOwned (new AudioFormatManager ());
    impl->formats->registerBasicFormats();
}

Versicap::~Versicap()
{
    impl->formats->clearFormats();
    impl.reset();
}

void Versicap::audioDeviceIOCallback (const float** inputChannelData,
                            int numInputChannels, float** outputChannelData,
                            int numOutputChannels, int numSamples) {}
void Versicap::audioDeviceAboutToStart (AudioIODevice* device) {}
void Versicap::audioDeviceStopped() {}
void Versicap::audioDeviceError (const String& errorMessage) {}

}
