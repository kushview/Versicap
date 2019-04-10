
#include "Exporter.h"
#include "PluginManager.h"
#include "Render.h"
#include "Versicap.h"
#include "UnlockStatus.h"

namespace vcp {

//=============================================================================
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
    Impl()
    { 
    }

    ~Impl() { }

    void prepare (AudioProcessor& plugin)
    {
        plugin.prepareToPlay (sampleRate, bufferSize);
        pluginLatency = plugin.getLatencySamples();
    }

    void release (AudioProcessor& plugin)
    {
        pluginLatency = 0;
        plugin.releaseResources();
    }

    void audioDeviceIOCallback (const float** input, int numInputs, 
                                float** output, int numOutputs,
                                int nframes) override
    {
        jassert (sampleRate > 0 && bufferSize > 0);
    
        ScopedNoDenormals denormals;
        messageCollector.removeNextBlockOfMessages (incomingMidi, nframes);
        ScopedLock slr (render->getCallbackLock());

        int totalNumChans = 0;
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

        render->renderCycleBegin();
        render->getNextMidiBlock (renderMidi, nframes);
        AudioSampleBuffer buffer (channels, totalNumChans, nframes);
        if (render->getSourceType() == SourceType::AudioPlugin)
        {
            if (auto* const proc = processor.get())
            {
                pluginBuffer.setSize (jmax (processor->getTotalNumInputChannels(), processor->getTotalNumOutputChannels()),
                                      nframes, false, false, true);
                {
                    ScopedLock slp (proc->getCallbackLock());
                    proc->processBlock (pluginBuffer, renderMidi);
                }
                
            }
            else
            {

            }
        }
        else
        {
            if (auto* const out = devices->getDefaultMidiOutput())
            {
                out->sendBlockOfMessages (renderMidi,
                    Time::getMillisecondCounterHiRes() + 1.0,
                    sampleRate);
            }
        }

        render->writeAudioFrames (buffer);
        render->renderCycleEnd();

        renderMidi.clear();
        incomingMidi.clear();
    }

    void audioDeviceAboutToStart (AudioIODevice* device) override
    {
        ScopedLock sl (render->getCallbackLock());
        sampleRate        = device->getCurrentSampleRate();
        bufferSize        = device->getCurrentBufferSizeSamples();
        numInputChans     = device->getActiveInputChannels().countNumberOfSetBits();
        numOutputChans    = device->getActiveInputChannels().countNumberOfSetBits();;
        inputLatency      = device->getInputLatencyInSamples();
        outputLatency     = device->getOutputLatencyInSamples();

        plugins->setPlayConfig (sampleRate, bufferSize);
        messageCollector.reset (sampleRate);
        channels.calloc ((size_t) jmax (numInputChans, numOutputChans) + 2);
        render->prepare (sampleRate, bufferSize);

        if (processor)
        {
            prepare (*processor);
            pluginBuffer.setSize (jmax (processor->getTotalNumInputChannels(), processor->getTotalNumOutputChannels()),
                                  bufferSize, false, false, true);
        }

        if (auto* const out = devices->getDefaultMidiOutput())
            out->startBackgroundThread();
    }

    void audioDeviceStopped() override
    {
        const ScopedLock sl (render->getCallbackLock());

        render->stop();
        render->release();

        if (processor)
        {
            release (*processor);
        }

        inputLatency = 0;
        outputLatency = 0;
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
        messageCollector.addMessageToQueue (message);
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
    int pluginLatency = 0;

    RenderContext context;
    std::unique_ptr<Render> render;

    int inputLatency = 0;
    int outputLatency = 0;
    double sampleRate { 44100.0 };
    int bufferSize = 1024;
    int numInputChans = 0, numOutputChans = 0;
    HeapBlock<float*> channels;
    AudioSampleBuffer tempBuffer;
    AudioSampleBuffer pluginBuffer;
    
    MidiBuffer incomingMidi;
    MidiBuffer renderMidi;
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
    
    impl->render->onStarted = [this]()
    {
        listeners.call ([](Listener& l) { l.renderStarted(); });
    };
    
    impl->render->onStopped = [this]()
    {
        listeners.call ([](Listener& l) 
        { 
            l.renderWillStop();
            l.renderStopped();
        });
    };
}

Versicap::~Versicap()
{
    impl->render.reset();
    impl->formats->clearFormats();
    impl.reset();
}

File Versicap::getApplicationDataPath() 
{
   #if JUCE_MAC
    return File::getSpecialLocation (File::userApplicationDataDirectory)
        .getChildFile ("Application Support/Versicap");
   #else
    return File::getSpecialLocation (File::userApplicationDataDirectory)
        .getChildFile ("Versicap");
   #endif
}

File Versicap::getUserDataPath()    { return File::getSpecialLocation(File::userMusicDirectory).getChildFile ("Versicap"); }
File Versicap::getPresetsPath()     { return getUserDataPath().getChildFile ("Presets"); }
File Versicap::getSamplesPath()     { return getUserDataPath().getChildFile ("Samples"); }

void Versicap::initializeDataPath()
{
    if (! getApplicationDataPath().exists())
        getApplicationDataPath().createDirectory();
    if (! getUserDataPath().exists())
        getUserDataPath().createDirectory();
    if (! getSamplesPath().exists())
        getSamplesPath().createDirectory();
    if (! getPresetsPath().exists())
        getPresetsPath().createDirectory();
}

void Versicap::initializeExporters()
{
    Exporter::createExporters (impl->exporters);
    getAudioFormats().registerBasicFormats();
}

void Versicap::initializeRenderContext()
{
    File contextFile = getApplicationDataPath().getChildFile ("context.versicap");
    if (contextFile.existsAsFile())
        impl->context.restoreFromFile (contextFile);
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
    const auto file = getApplicationDataPath().getChildFile ("plugins.xml");
    plugins.restoreAudioPlugins (file);
}

void Versicap::initializeUnlockStatus()
{
    getUnlockStatus().load();
}

void Versicap::initialize()
{
    initializeDataPath();
    initializeExporters();
    initializePlugins();
    initializeAudioDevice();
    initializeUnlockStatus();
    initializeRenderContext();
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
        const auto file = getApplicationDataPath().getChildFile ("plugins.xml");
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

void Versicap::saveRenderContext()
{
    File contextFile = getApplicationDataPath().getChildFile("context.versicap");
    if (! contextFile.getParentDirectory().exists())
        contextFile.getParentDirectory().createDirectory();
    impl->context.writeToFile (contextFile);
}

void Versicap::setRenderContext (const RenderContext& context) { impl->context = context; }
const RenderContext& Versicap::getRenderContext() const     { return impl->context; }
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
            DBG("[VCP] latency: " << processor->getLatencySamples());
            
            processor->prepareToPlay (impl->sampleRate, impl->bufferSize);

            {
                ScopedLock sl (impl->render->getCallbackLock());
                impl->processor.swap (processor);
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

Result Versicap::startRendering (const RenderContext& newContext)
{
    setRenderContext (newContext);
    if (impl->render->isRendering())
        return Result::fail ("Versicap is already rendering");
    
    int latency = 0;
    const auto context = getRenderContext();
    if (context.source == SourceType::AudioPlugin)
    {
        if (impl->processor == nullptr)
            return Result::fail ("No plugin selected to render");
        if (nullptr == getDeviceManager().getCurrentAudioDevice())
            return Result::fail ("Audio engine is not running");
        latency = impl->pluginLatency = impl->processor->getLatencySamples();
    }
    else
    {
        if (nullptr == getDeviceManager().getDefaultMidiOutput())
            return Result::fail ("No MIDI output device slected for rendering");
        if (nullptr == getDeviceManager().getCurrentAudioDevice())
            return Result::fail ("Audio engine is not running");
        latency = impl->inputLatency;
    }
    
    impl->render->start (context, jmax (0, latency));
    listeners.call ([](Listener& listener) { listener.renderWillStart(); });
    return Result::ok();
}

void Versicap::stopRendering()
{
    impl->render->stop();
    listeners.call ([](Listener& listener) { listener.renderWillStop(); });
}

}
