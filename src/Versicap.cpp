
#include "Exporter.h"
#include "PluginManager.h"
#include "Render.h"
#include "Versicap.h"
#include "UnlockStatus.h"

namespace vcp {

class PluginWindow : public DocumentWindow
{
public:
    PluginWindow (std::unique_ptr<PluginWindow>& o, AudioProcessorEditor* ed)
        : DocumentWindow (ed->getAudioProcessor()->getName(),
                          Colours::black, DocumentWindow::closeButton),
          owner (o)
    {
        owner.reset (this);
        editor.reset (ed);
        setUsingNativeTitleBar (true);
        setContentNonOwned (editor.get(), true);
        setResizable (editor->isResizable(), false);
        centreWithSize (getWidth(), getHeight());
        setVisible (true);
    }

    ~PluginWindow()
    {
        if (editor)
        {
            if (auto* proc = editor->getAudioProcessor())
                proc->editorBeingDeleted (editor.get());
            editor.reset();
        }
    }

    void closeButtonPressed() override
    {
        owner.reset();
    }

private:
    std::unique_ptr<PluginWindow>& owner;
    std::unique_ptr<AudioProcessorEditor> editor;
};

struct Versicap::Impl
{
    Impl() { }
    ~Impl() { }

    Settings settings;
    KnownPluginList knownPlugins;
    OwnedArray<Exporter> exporters;
    OptionalScopedPointer<AudioDeviceManager> devices;
    OptionalScopedPointer<AudioFormatManager> formats;
    OptionalScopedPointer<PluginManager> plugins;
    std::unique_ptr<UnlockStatus> unlock;
    std::unique_ptr<AudioProcessor> processor;
    std::unique_ptr<PluginWindow> window;

    double sampleRate { 44100.0 };
    int bufferSize = 1024;
};

Versicap::Versicap()
{
    impl.reset (new Impl());
    impl->devices.setOwned (new AudioDeviceManager ());
    impl->formats.setOwned (new AudioFormatManager ());
    impl->plugins.setOwned (new PluginManager());
    impl->unlock.reset (new UnlockStatus (impl->settings));
    render.reset (new Render (*impl->formats));
}

Versicap::~Versicap()
{
    render.reset();
    impl->formats->clearFormats();
    impl.reset();
}

File Versicap::getApplicationDataDir() 
{
   #if JUCE_MAC
    return File::getSpecialLocation (File::userApplicationDataDirectory)
        .getChildFile ("Application Support/Versicap");
   #else
    return File::getSpecialLocation (File::userApplicationDataDirectory)
        .getChildFile ("Versicap");
   #endif
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
    auto& plugins = getPluginManager();
    plugins.addDefaultFormats();
    const auto file = getApplicationDataDir().getChildFile ("plugins.xml");
    plugins.restoreAudioPlugins (file);
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

    if (auto xml = std::unique_ptr<XmlElement> (plugins.getKnownPlugins().createXml()))
    {
        const auto file = getApplicationDataDir().getChildFile ("plugins.xml");
        xml->writeToFile (file, String());
    }
    
    if (auto* const props = settings.getUserSettings())
    {
        if (auto* devicesXml = devices.createStateXml())
        {
            props->setValue ("devices", devicesXml);
            deleteAndZero (devicesXml);
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
PluginManager& Versicap::getPluginManager()                 { return *impl->plugins; }
UnlockStatus& Versicap::getUnlockStatus()                   { return *impl->unlock; }

void Versicap::loadPlugin (const PluginDescription& type)
{
    auto& plugins = getPluginManager();
    String errorMessage;
    std::unique_ptr<AudioProcessor> processor;
    processor.reset (plugins.createAudioPlugin (type, errorMessage));
    
    if (errorMessage.isNotEmpty())
    {
        AlertWindow::showNativeDialogBox ("Versicap", "Could not create plugin", false);
    }
    else
    {
        if (processor)
        {
            impl->window.reset();
            DBG("[VCP] loaded: " << processor->getName());
            processor->prepareToPlay (impl->sampleRate, impl->bufferSize);

            {
                ScopedLock sl (render->getCallbackLock());
                impl->processor.swap (processor);
                render->setAudioProcessor (impl->processor.get());
            }

            showPluginWindow();
        }
        else
        {
            AlertWindow::showNativeDialogBox ("Versicap", "Could not instantiate plugin", false);
        }
    }

    if (processor)
    {
        processor->releaseResources();
        processor.reset();
    }
}

void Versicap::closePlugin()
{
    impl->window.reset();
    std::unique_ptr<AudioProcessor> oldProc;

    {
        ScopedLock sl (render->getCallbackLock());
        oldProc.swap (impl->processor);
        render->setAudioProcessor (nullptr);
    }

    if (oldProc)
    {
        oldProc->releaseResources();
        oldProc.reset();
    }
}

void Versicap::closePluginWindow()
{
    impl->window.reset();
}

void Versicap::showPluginWindow()
{
    if (impl->processor == nullptr)
        return;

    if (impl->window)
    {
        impl->window->toFront (false);
        return;
    }

    PluginWindow* window = nullptr;
    if (auto* editor = impl->processor->createEditorIfNeeded())
    {
        window = new PluginWindow (impl->window, editor);
    }
    else
    {
        window = new PluginWindow (impl->window,
            new GenericAudioProcessorEditor (impl->processor.get()));
    }

    if (window)
        window->toFront (false);
}

Result Versicap::startRendering (const RenderContext& context)
{
    if (render->isRendering())
        return Result::fail ("Versicap is already rendering");
    render->start (context);
    return Result::ok();
}

void Versicap::stopRendering()
{
    render->stop();
}

void Versicap::audioDeviceIOCallback (const float** input, int numInputs, 
                                      float** output, int numOutputs, int nframes)
{
    render->process (nframes);
    ignoreUnused (input, numInputs, output, numOutputs);
}

void Versicap::audioDeviceAboutToStart (AudioIODevice* device)
{
    {
        ScopedLock sl (render->getCallbackLock());
        impl->sampleRate = device->getCurrentSampleRate();
        impl->bufferSize = device->getCurrentBufferSizeSamples();
        impl->plugins->setPlayConfig (impl->sampleRate, impl->bufferSize);
        render->prepare (impl->sampleRate, impl->bufferSize);
    }
}

void Versicap::audioDeviceStopped()
{
    render->stop();
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
