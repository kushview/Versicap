
#include "Versicap.h"

namespace vcp {

struct Versicap::Impl
{
    Impl() { }
    ~Impl() { }

    Settings settings;
    KnownPluginList knownPlugins;
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

void Versicap::initializeAudioDevice()
{
    auto& devices = getDeviceManager();
    auto& settings = impl->settings;
    bool initDefault = true;
    
    if (auto* const props = settings.getUserSettings())
    {
        if (auto* xml = props->getXmlValue ("devices"))
        {
            initDefault = devices.initialise (32, 32, xml, false).isNotEmpty();
            deleteAndZero (xml);
        }
    }
    
    if (initDefault)
    {
        devices.initialiseWithDefaultDevices (32, 32);
    }

    devices.addAudioCallback (this);
    devices.addMidiInputCallback (String(), this);
}

void Versicap::initializePlugins()
{
    auto& settings = impl->settings;
    auto& plugins = getPluginManager();
    plugins.addDefaultFormats();
    if (auto* const props = settings.getUserSettings())
    {
        const auto file = props->getFile().getParentDirectory().getChildFile("plugins.xml");
        if (auto* xml = XmlDocument::parse (file))
        {
            impl->knownPlugins.recreateFromXml (*xml);
            deleteAndZero (xml);
        }
    }
}

void Versicap::shutdown()
{
    auto& devices = getDeviceManager();
    devices.removeAudioCallback (this);
    devices.removeMidiInputCallback (String(), this);
    devices.closeAudioDevice();
}

void Versicap::saveSettings()
{
    auto& settings = impl->settings;
    auto& devices  = getDeviceManager();
    auto& plugins  = getPluginManager();
    auto& formats  = getAudioFormats();

    if (auto* const props = settings.getUserSettings())
    {
        if (auto* devicesXml = devices.createStateXml())
        {
            props->setValue ("devices", devicesXml);
            deleteAndZero (devicesXml);
        }

        if (auto* knownPlugins = impl->knownPlugins.createXml())
        {
            const auto file = props->getFile().getParentDirectory().getChildFile("plugins.xml");
            knownPlugins->writeToFile (file, String());
            deleteAndZero (knownPlugins);
        }
    }
}

Settings& Versicap::getSettings()                       { return impl->settings; }
AudioDeviceManager& Versicap::getDeviceManager()        { return *impl->devices; }
AudioFormatManager& Versicap::getAudioFormats()         { return *impl->formats; }
AudioPluginFormatManager& Versicap::getPluginManager()  { return *impl->plugins; }

void Versicap::audioDeviceIOCallback (const float** inputChannelData,
                                      int numInputChannels, float** outputChannelData,
                                      int numOutputChannels, int numSamples) {}
void Versicap::audioDeviceAboutToStart (AudioIODevice* device) {}
void Versicap::audioDeviceStopped() { }
void Versicap::audioDeviceError (const String& errorMessage) { }

}
