
#include "Exporter.h"
#include "Render.h"
#include "Versicap.h"
#include "UnlockStatus.h"

namespace vcp {

struct Versicap::Impl
{
    Impl() { }
    ~Impl() { }

    Settings settings;
    KnownPluginList knownPlugins;
    OwnedArray<Exporter> exporters;
    OptionalScopedPointer<AudioDeviceManager> devices;
    OptionalScopedPointer<AudioFormatManager> formats;
    OptionalScopedPointer<AudioPluginFormatManager> plugins;
    std::unique_ptr<UnlockStatus> unlock;
};

Versicap::Versicap()
{
    impl.reset (new Impl());
    impl->devices.setOwned (new AudioDeviceManager ());
    impl->formats.setOwned (new AudioFormatManager ());
    impl->plugins.setOwned (new AudioPluginFormatManager());
    impl->unlock.reset (new UnlockStatus (impl->settings));
    render.reset (new Render());
}

Versicap::~Versicap()
{
    impl->formats->clearFormats();
    impl.reset();
}

void Versicap::initializeExporters()
{
    Exporter::createExporters (impl->exporters);
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
    auto& unlock   = getUnlockStatus();

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

    unlock.save();
}

File Versicap::getUserDataPath()
{
    auto path = File::getSpecialLocation (File::userMusicDirectory).getChildFile ("Versicap");
    if (! path.exists())
        path.createDirectory();
    return path;
}

File Versicap::getSamplesPath()
{
    auto path = getUserDataPath().getChildFile ("Samples");
    if (! path.exists())
        path.createDirectory();
    return path;
}

const OwnedArray<Exporter>& Versicap::getExporters() const  { return impl->exporters; }
Settings& Versicap::getSettings()                           { return impl->settings; }
AudioDeviceManager& Versicap::getDeviceManager()            { return *impl->devices; }
AudioFormatManager& Versicap::getAudioFormats()             { return *impl->formats; }
AudioPluginFormatManager& Versicap::getPluginManager()      { return *impl->plugins; }
UnlockStatus& Versicap::getUnlockStatus() { return *impl->unlock; }

void Versicap::audioDeviceIOCallback (const float** input, int numInputs, 
                                      float** output, int numOutputs, int nframes)
{
    render->process (nframes);
    ignoreUnused (input, numInputs, output, numOutputs);
}

void Versicap::audioDeviceAboutToStart (AudioIODevice* device)
{
    ignoreUnused (device);
}

void Versicap::audioDeviceStopped()
{

}

void Versicap::audioDeviceError (const String& errorMessage) { ignoreUnused (errorMessage); }

void Versicap::handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message)
{

}

void Versicap::handlePartialSysexMessage (MidiInput* source, const uint8* messageData,
                                          int numBytesSoFar, double timestamp)
{

}

}
