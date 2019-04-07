
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

struct Versicap::Impl : public AudioIODeviceCallback,
                        public MidiInputCallback
{
    Impl() { }
    ~Impl() { }

    void audioDeviceIOCallback (const float** input, int numInputs, 
                                float** output, int numOutputs, int nframes) override
    {
        jassert (sampleRate > 0 && bufferSize > 0);
        int totalNumChans = 0;
        ScopedNoDenormals denormals;
        if (numInputs > numOutputs)
        {
            // if there aren't enough output channels for the number of
            // inputs, we need to create some temporary extra ones (can't
            // use the input data in case it gets written to)
            tempBuffer.setSize (numInputs - numOutputs, nframes, false, false, true);
            
            for (int i = 0; i < numOutputs; ++i)
            {
                channels[totalNumChans] = output[i];
                memcpy (channels[totalNumChans], input[i], sizeof (float) * (size_t) nframes);
                ++totalNumChans;
            }
            
            for (int i = numOutputs; i < numInputs; ++i)
            {
                channels[totalNumChans] = tempBuffer.getWritePointer (i - numOutputs, 0);
                memcpy (channels[totalNumChans], input[i], sizeof (float) * (size_t) nframes);
                ++totalNumChans;
            }
        }
        else
        {
            for (int i = 0; i < numInputs; ++i)
            {
                channels[totalNumChans] = output[i];
                memcpy (channels[totalNumChans], input[i], sizeof (float) * (size_t) nframes);
                ++totalNumChans;
            }
            
            for (int i = numInputs; i < numOutputs; ++i)
            {
                channels[totalNumChans] = output[i];
                zeromem (channels[totalNumChans], sizeof (float) * (size_t) nframes);
                ++totalNumChans;
            }
        }

        AudioSampleBuffer buffer (channels, totalNumChans, nframes); 
    }

    void audioDeviceAboutToStart (AudioIODevice* device) override
    {
        ScopedLock sl (render->getCallbackLock());
        sampleRate        = device->getCurrentSampleRate();
        bufferSize        = device->getCurrentBufferSizeSamples();
        numInputChans     = device->getActiveInputChannels().countNumberOfSetBits();
        numOutputChans    = device->getActiveInputChannels().countNumberOfSetBits();;

        plugins->setPlayConfig (sampleRate, bufferSize);

        messageCollector.reset (sampleRate);
        channels.calloc ((size_t) jmax (numInputChans, numOutputChans) + 2);

        render->prepare (sampleRate, bufferSize);
    }

    void audioDeviceStopped() override
    {
        const ScopedLock sl (render->getCallbackLock());
        render->stop();
        sampleRate  = 0.0;
        bufferSize  = 0;
        tempBuffer.setSize (1, 1);
        channels.free();        
    }

    void audioDeviceError (const String& errorMessage) override
    { 
        ignoreUnused (errorMessage); 
    }

    void handleIncomingMidiMessage (MidiInput*, const MidiMessage& message) override
    {
        ignoreUnused (message);
    }

    void handlePartialSysexMessage (MidiInput* source, const uint8* messageData,
                                    int numBytesSoFar, double timestamp) override
    {
        ignoreUnused (source, messageData, numBytesSoFar, timestamp);
    }

    Settings settings;
    KnownPluginList knownPlugins;
    OwnedArray<Exporter> exporters;
    OptionalScopedPointer<AudioDeviceManager> devices;
    OptionalScopedPointer<AudioFormatManager> formats;
    OptionalScopedPointer<PluginManager> plugins;
    std::unique_ptr<UnlockStatus> unlock;
    std::unique_ptr<AudioProcessor> processor;
    std::unique_ptr<PluginWindow> window;

    std::unique_ptr<Render> render;

    double sampleRate { 44100.0 };
    int bufferSize = 1024;
    int numInputChans = 0, numOutputChans = 0;
    HeapBlock<float*> channels;
    AudioSampleBuffer tempBuffer;
    MidiBuffer incomingMidi;
    MidiMessageCollector messageCollector;
};

Versicap::Versicap()
{
    impl.reset (new Impl());
    impl->devices.setOwned (new AudioDeviceManager ());
    impl->formats.setOwned (new AudioFormatManager ());
    impl->plugins.setOwned (new PluginManager());
    impl->unlock.reset (new UnlockStatus (impl->settings));
    impl->render.reset (new Render (*impl->formats));
}

Versicap::~Versicap()
{
    impl->render.reset();
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

    devices.addAudioCallback (impl.get());
    devices.addMidiInputCallback (String(), impl.get());
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
    devices.removeAudioCallback (impl.get());
    devices.removeMidiInputCallback (String(), impl.get());
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
                ScopedLock sl (impl->render->getCallbackLock());
                impl->processor.swap (processor);
                impl->render->setAudioProcessor (impl->processor.get());
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
        ScopedLock sl (impl->render->getCallbackLock());
        oldProc.swap (impl->processor);
        impl->render->setAudioProcessor (nullptr);
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
    if (impl->render->isRendering())
        return Result::fail ("Versicap is already rendering");
    impl->render->start (context);
    return Result::ok();
}

void Versicap::stopRendering()
{
    impl->render->stop();
}

}
