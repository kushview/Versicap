
#include "controllers/GuiController.h"
#include "controllers/ProjectsController.h"

#include "gui/PluginWindow.h"

#include "Commands.h"
#include "Exporter.h"
#include "PluginManager.h"
#include "Project.h"
#include "Render.h"
#include "Versicap.h"
#include "UnlockStatus.h"

namespace vcp {

struct Versicap::Impl : public AudioIODeviceCallback,
                        public MidiInputCallback,
                        public ChangeListener,
                        public ApplicationCommandTarget
{
    Impl()
        : peaks (32)
    { 
    }

    ~Impl() { }

    void updatePluginProperties()
    {
        if (! processor)
            return;
        
        ScopedLock psl (processor->getCallbackLock());
        pluginLatency  = processor->getLatencySamples();
        pluginNumIns   = processor->getTotalNumInputChannels();
        pluginNumOuts  = processor->getTotalNumOutputChannels();
        pluginChannels = jmax (pluginNumIns, pluginNumOuts);
    }

    void prepare (AudioProcessor& plugin)
    {
        jassert (sampleRate > 0.0);
        jassert (bufferSize > 0);
        plugin.enableAllBuses();
        plugin.setRateAndBufferSizeDetails (sampleRate, bufferSize);
        plugin.prepareToPlay (sampleRate, bufferSize);
    }

    void release (AudioProcessor& plugin)
    {
        plugin.releaseResources();
    }

    void audioDeviceIOCallback (const float** input, int numInputs, 
                                float** output, int numOutputs,
                                int nframes) override
    {
        if (shouldProcess.get() != 1)
        {
            for (int c = 0; c < numOutputs; ++c)
                FloatVectorOperations::clear (output[0], nframes);
            return;
        }

        jassert (sampleRate > 0 && bufferSize > 0);
    
        ScopedNoDenormals denormals;
        messageCollector.removeNextBlockOfMessages (incomingMidi, nframes);
        ScopedLock slr (render->getCallbackLock());

       #if 0
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
       #endif

        const auto& renderCtx = render->getContext();
        const int source = sourceType.get();
        renderBuffer.setSize (renderCtx.channels, nframes, false, false, true);
        pluginBuffer.setSize (pluginChannels, nframes, false, false, true);

        render->renderCycleBegin();
        render->getNextMidiBlock (renderMidi, nframes);
        if (! render->isRendering())
            renderMidi.addEvents (incomingMidi, 0, nframes, 0);

        if (auto* const proc = processor.get())
        {
            ScopedLock slp (proc->getCallbackLock());
            proc->processBlock (pluginBuffer, renderMidi);
        }

        if (source == SourceType::AudioPlugin)
        {
            if (pluginChannels == 0 || pluginNumOuts == 0)
            {
                // noop
                renderBuffer.clear (0, nframes);
                pluginBuffer.clear (0, nframes);
            }
            else if (pluginNumOuts == renderCtx.channels)
            {
                // one-to-one channel match
                for (int c = renderCtx.channels; --c >= 0;)
                    renderBuffer.copyFrom (c, 0, pluginBuffer, c, 0, nframes);
            }
            else if (renderCtx.channels == 1)
            {
                // mix to mono
                const float reduction = Decibels::decibelsToGain (-3.f);
                // for (int c = pluginNumOuts; --c >= 0;)
                //     renderBuffer.addFrom (0, 0, pluginBuffer, c, 0, nframes, reduction);
                renderBuffer.copyFrom (0, 0, pluginBuffer, 0, 0, nframes);
            }
            else
            {
                // fall back - copy the lesser of the channels
                for (int c = jmin (pluginNumOuts, renderCtx.channels); --c >= 0;)
                    renderBuffer.copyFrom (c, 0, pluginBuffer, c, 0, nframes);
            }
        }
        else if (source == SourceType::MidiDevice)
        {
            if (auto* const out = devices->getDefaultMidiOutput())
            {
                out->sendBlockOfMessages (renderMidi,
                    Time::getMillisecondCounterHiRes() + 1.0,
                    sampleRate);
            }

            if (numInputs == renderCtx.channels)
            {
                // one-to-one channel match
                for (int c = renderCtx.channels; --c >= 0;)
                    renderBuffer.copyFrom (c, 0, input [c], nframes);
            }
            else if (renderCtx.channels == 1)
            {
                // mix to mono
                const float reduction = Decibels::decibelsToGain (-3.f);
                for (int c = numInputs; --c >= 0;)
                    renderBuffer.addFrom (0, 0, input[c], nframes, reduction);
            }
            else
            {
                // fall back - copy the lesser of the channels
                for (int c = jmin (numInputs, renderCtx.channels); --c >= 0;)
                    renderBuffer.copyFrom (c, 0, input[c], nframes);
                for (int c = renderCtx.channels; c < numInputs; ++c)
                    renderBuffer.clear (c, 0, nframes);
            }
        }
        else
        {
            // unkown source, clear
            renderBuffer.clear (0, nframes);
        }
        
        render->writeAudioFrames (renderBuffer);
        render->renderCycleEnd();

        const auto nbytes = sizeof(float) * static_cast<size_t> (nframes);
        for (int c = 0; c < numOutputs; ++c)
            memset (output [c], 0, nbytes);
        
        if (renderCtx.channels == 1)
        {
            for (int c = numOutputs; --c >= 0;)
                memcpy (output[c], renderBuffer.getReadPointer (0), nbytes);
        }
        else
        {
            for (int c = jmin(numOutputs, renderCtx.channels); --c >= 0;)
                memcpy (output[c], renderBuffer.getReadPointer (c), nbytes);
        }
        
        renderMidi.clear();
        incomingMidi.clear();
    }

    void audioDeviceAboutToStart (AudioIODevice* device) override
    {
        ScopedLock sl (render->getCallbackLock());
        sampleRate        = device->getCurrentSampleRate();
        bufferSize        = device->getCurrentBufferSizeSamples();
        numInputChans     = device->getActiveInputChannels().countNumberOfSetBits();
        numOutputChans    = device->getActiveOutputChannels().countNumberOfSetBits();;
        inputLatency      = device->getInputLatencyInSamples();
        outputLatency     = device->getOutputLatencyInSamples();

        plugins->setPlayConfig (sampleRate, bufferSize);
        messageCollector.reset (sampleRate);
        channels.calloc ((size_t) jmax (numInputChans, numOutputChans) + 2);
        render->prepare (sampleRate, bufferSize);

        if (processor)
        {
            prepare (*processor);
            updatePluginProperties();
            pluginBuffer.setSize (pluginChannels, bufferSize, false, false, true);
        }

        if (auto* const out = devices->getDefaultMidiOutput())
            out->startBackgroundThread();
    }

    void audioDeviceStopped() override
    {
        const ScopedLock sl (render->getCallbackLock());

        render->cancel();
        render->release();

        if (processor)
        {
            release (*processor);
        }

        inputLatency    = 0;
        outputLatency   = 0;
        sampleRate      = 0.0;
        bufferSize      = 0;
        pluginChannels  = 0;
        pluginLatency   = 0;
        pluginNumIns    = 0;
        pluginNumOuts   = 0;
        
        tempBuffer.setSize (1, 1);
        pluginBuffer.setSize (1, 1);
        renderBuffer.setSize (1, 1);
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

    void updateLockedStatus()
    {
        const bool unlocked = (bool) unlock->isUnlocked();
        shouldProcess.set (unlocked ? 1 : 0);
    }

    //=========================================================================
    void changeListenerCallback (ChangeBroadcaster* broadcaster) override
    {
        if (broadcaster == unlock.get())
        {
            updateLockedStatus();
        }
    }

    //=========================================================================
    ApplicationCommandTarget* getNextCommandTarget() override
    {
        return nullptr;
    }

    void getAllCommands (Array<CommandID>& commands) override
    {
        commands.addArray ({
            Commands::projectSave,
            Commands::projectSaveAs,
            Commands::projectNew,
            Commands::projectOpen
           #if 0
            Commands::showAbout,
            Commands::checkForUpdates
           #endif
        });
    }

    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override
    {
        switch (commandID)
        {
            case Commands::projectSave:
                result.setInfo ("Save Project As", "Save the current project", "Project", 0);
                break;
            case Commands::projectSaveAs:
                result.setInfo ("Save Project", "Save the current project", "Project", 0);
                break;
            case Commands::projectOpen:
                result.setInfo ("Open Project", "Open an existing project", "Project", 0);
                break;
            case Commands::projectNew:
                result.setInfo ("New Project", "Create a new project", "Project", 0);
                break;
        }
    }

    bool perform (const InvocationInfo& info) override
    {
        bool handled = true;

        switch (info.commandID)
        {
            case Commands::projectSave: {
                DBG("done");
            } break;

            case Commands::projectSaveAs: {
                DBG("done");
            } break;

            case Commands::projectNew: {
                DBG("done");
            } break;

            case Commands::projectOpen: {
                DBG("done");
            } break;

            default: handled = false;
        }

        return handled;
    }

    //=========================================================================
    Project project;
    OwnedArray<Controller> controllers;

    Settings settings;
    AudioThumbnailCache peaks;
    ApplicationCommandManager commands;
    OwnedArray<Exporter> exporters;
    OptionalScopedPointer<AudioDeviceManager> devices;
    OptionalScopedPointer<AudioFormatManager> formats;
    OptionalScopedPointer<PluginManager> plugins;
    std::unique_ptr<UnlockStatus> unlock;
    MidiKeyboardState keyboardState;

    //=========================================================================
    std::unique_ptr<AudioProcessor> processor;
    std::unique_ptr<PluginWindow> window;
    
    Atomic<int> shouldProcess { 0 };
    Atomic<int> sourceType { SourceType::MidiDevice };

    int pluginLatency = 0;
    int pluginChannels = 0;
    int pluginNumIns = 0;
    int pluginNumOuts = 0;

    //=========================================================================
    std::unique_ptr<Render> render;
    RenderContext context;
    AudioSampleBuffer renderBuffer;

    //=========================================================================
    int inputLatency  = 0;
    int outputLatency = 0;
    int extraLatency  = 0;
    double sampleRate { 0.0 };
    int bufferSize = 0;
    int numInputChans = 0;
    int numOutputChans = 0;

    //=========================================================================
    HeapBlock<float*> channels;
    AudioSampleBuffer tempBuffer;
    AudioSampleBuffer pluginBuffer;
    
    //=========================================================================
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
    
    impl->controllers.add (new GuiController (*this));
    impl->controllers.add (new ProjectsController (*this));

    impl->render->onStarted = [this]()
    {
        listeners.call ([](Listener& l) { l.renderStarted(); });
    };
    
    impl->render->onStopped = [this]()
    {
        listeners.call ([](Listener& l) { l.renderWillStop(); });
        auto project = getProject();
        project.setSamples (impl->render->getSamples());
        listeners.call ([](Listener& l) { l.renderStopped(); });
    };

    impl->render->onCancelled = [this]()
    {
        listeners.call ([](Listener& l) { l.renderWillStop(); });
        listeners.call ([](Listener& l) { l.renderStopped(); });
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
        loadProject (contextFile);
}

void Versicap::initializeAudioDevice()
{
    auto& devices = getDeviceManager();
    auto& settings = impl->settings;
    bool initDefault = true;
    
    devices.addAudioCallback (impl.get());
    devices.addMidiInputCallback (String(), impl.get());

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
    impl->updateLockedStatus();
    getUnlockStatus().addChangeListener (impl.get());
}

void Versicap::initialize()
{
    initializeDataPath();
    initializeExporters();
    initializePlugins();
    initializeAudioDevice();
    initializeUnlockStatus();
    initializeRenderContext();

    for (auto* const controller : impl->controllers)
        controller->initialize();

    impl->keyboardState.addListener (&impl->messageCollector);
    impl->commands.registerAllCommandsForTarget (impl.get());
    impl->commands.setFirstCommandTarget (impl.get());
}

void Versicap::shutdown()
{
    for (auto* const controller : impl->controllers)
        controller->shutdown();

    impl->keyboardState.removeListener (&impl->messageCollector);

    auto& unlock = getUnlockStatus();
    unlock.removeChangeListener (impl.get());
    unlock.save();

    auto& devices = getDeviceManager();
    devices.removeAudioCallback (impl.get());
    devices.removeMidiInputCallback (String(), impl.get());
    devices.closeAudioDevice();

    closePluginWindow();
    if (impl->processor != nullptr)
    {
        impl->processor->releaseResources();
        impl->processor.reset();
    }

    impl->render.reset();
}

void Versicap::saveSettings()
{
    auto& settings = getSettings();
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
    saveProject (contextFile);
}

void Versicap::setRenderContext (const RenderContext& context)
{ 
    impl->context = context; 
    impl->sourceType.set (impl->context.source);
}

AudioThumbnail* Versicap::createAudioThumbnail (const File& file)
{
    auto* thumb = new AudioThumbnail (1, *impl->formats, impl->peaks);
    thumb->setSource (new FileInputSource (file));
    impl->peaks.removeThumb (thumb->getHashCode());

    return thumb;
}

AudioThumbnailCache& Versicap::getAudioThumbnailCache()     { return impl->peaks; }
ApplicationCommandManager& Versicap::getCommandManager()    { return impl->commands; }
const RenderContext& Versicap::getRenderContext() const     { return impl->context; }
const OwnedArray<Exporter>& Versicap::getExporters() const  { return impl->exporters; }
Settings& Versicap::getSettings()                           { return impl->settings; }
AudioDeviceManager& Versicap::getDeviceManager()            { return *impl->devices; }
AudioFormatManager& Versicap::getAudioFormats()             { return *impl->formats; }
MidiKeyboardState& Versicap::getMidiKeyboardState()         { return impl->keyboardState; }
PluginManager& Versicap::getPluginManager()                 { return *impl->plugins; }
UnlockStatus& Versicap::getUnlockStatus()                   { return *impl->unlock; }

void Versicap::loadPlugin (const PluginDescription& type, bool clearProjectPlugin)
{
    auto& plugins = getPluginManager();
    String errorMessage;
    std::unique_ptr<AudioProcessor> processor (plugins.createAudioPlugin (type, errorMessage));
    
    if (errorMessage.isNotEmpty())
    {
        AlertWindow::showNativeDialogBox ("Versicap", "Could not create plugin", false);
    }
    else
    {
        if (processor)
        {
            impl->window.reset();            
            impl->prepare (*processor);
            if (clearProjectPlugin)
                impl->project.clearPlugin();
            
            {
                ScopedLock sl (impl->render->getCallbackLock());
                impl->processor.swap (processor);
            }

            impl->updatePluginProperties();
            showPluginWindow();
            impl->project.setPluginDescription (type);
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

void Versicap::closePlugin (bool clearProjectPlugin)
{
    closePluginWindow();
    std::unique_ptr<AudioProcessor> oldProc;

    {
        ScopedLock sl (impl->render->getCallbackLock());
        oldProc.swap (impl->processor);
        impl->pluginChannels    = 0;
        impl->pluginLatency     = 0;
        impl->pluginNumIns      = 0;
        impl->pluginNumOuts     = 0;
    }

    if (clearProjectPlugin)
        impl->project.clearPlugin();

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

Result Versicap::startRendering()
{
    const auto project = getProject();
    RenderContext context;
    project.getRenderContext (context);
    setRenderContext (context);
    if (impl->render->isRendering())
        return Result::fail ("Versicap is already rendering");
    
    int latency = 0;
    context = getRenderContext();
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
        latency = impl->inputLatency + roundToInt (0.001 * impl->sampleRate);
    }

    impl->render->start (context, jmax (0, latency));
    listeners.call ([](Listener& listener) { listener.renderWillStart(); });
    return Result::ok();
}

Result Versicap::startRendering (const RenderContext&)
{
    jassertfalse;
    return startRendering();
}

void Versicap::stopRendering()
{
    impl->render->cancel();
    listeners.call ([](Listener& listener) { listener.renderWillStop(); });
}

Project Versicap::getProject() const { return impl->project; }

bool Versicap::saveProject (const File& file)
{
    auto project = getProject();
    if (! project.getValueTree().isValid())
        return false;

    if (auto* processor = impl->processor.get())
        project.updatePluginState (*processor);

    return project.writeToFile (file);
}

bool Versicap::loadProject (const File& file)
{
    auto newProject = Project();
    if (! newProject.loadFile (file))
        return false;
    
    impl->project = newProject;
    auto& project = impl->project;

    RenderContext context;
    project.getRenderContext (context);
    setRenderContext (context);

    PluginDescription desc;
    if (impl->project.getPluginDescription (getPluginManager(), desc))
    {
        loadPlugin (desc, false);
        if (auto* proc = impl->processor.get())
            impl->project.applyPluginState (*proc);
    }

    return true;
}

}
