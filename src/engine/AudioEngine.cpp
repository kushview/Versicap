
#include "engine/AudioEngine.h"
#include "engine/Render.h"
#include "PluginManager.h"
#include "IncludeKSP1.h"

namespace vcp {

AudioEngine::AudioEngine (AudioFormatManager& formatManager, 
                          AudioPluginFormatManager& pluginManager,
                          KSP1::SampleCache& cache)
    : formats (formatManager),
      plugins (pluginManager),
      sampleCache (cache)
{
    monitor = new Monitor();

    sampler.reset (KSP1::SamplerSynth::create (sampleCache));
    
    render.reset (new Render (formatManager));
    render->onCancelled = [this]()
    {
        if (onRenderCancelled)
            onRenderCancelled();
        panic();
    };

    render->onStarted   = [this]() { if (onRenderStarted)   onRenderStarted(); };
    render->onStopped   = [this]()
    { 
        if (onRenderStopped)
            onRenderStopped();
        panic();
    };

    render->onProgress = [this]()
    {
        if (onRenderProgress)
        {
            auto title = render->getNextProgressTitle();
            auto progress = render->getProgress();
            onRenderProgress (progress, title);
        }
    };

    watcher.onChanged = std::bind (&AudioEngine::onProjectLoaded, this);
}

AudioEngine::~AudioEngine()
{
    watcher.onChanged = nullptr;
    render->onCancelled = render->onStarted = render->onStopped = nullptr;
    render.reset();
}

void AudioEngine::setProject (const Project& project)
{
    watcher.setProject (project);
}

void AudioEngine::onProjectLoaded()
{
    using KSP1::SamplerSound;
    sampler->clearAllSounds();
    sampler->clearSounds();
#if 0
    // EXPERIMENTAL: load entire project into sampler
    DBG("[VCP] project loaded in engine");
    
    const auto project = watcher.getProject();
    Array<int> notes;
    project.getPossibleNoteNumbers (notes);

    OwnedArray<Sample> samples;
    for (const auto& note : notes)
    {
        project.getSamplesForNote (note, samples);
        std::unique_ptr<SamplerSound> sound (new SamplerSound (note));
        
        for (auto* const sample : samples)
        {
            if (auto* const data = sampleCache.getLayerData (true))
                if (data->loadAudioFile (sample->getFile()))
                    sound->insertLayerData (data);
        }

        if (sound->getNumLayers() > 0)
            sampler->insertSound (sound.release());
        
        samples.clearQuick (false);
    }
#endif
}

void AudioEngine::onActiveSampleChanged()
{
    const auto project = watcher.getProject();
    const auto sample = project.getActiveSample();
    if (! sample.isValid())
        return;
    using KSP1::SamplerSound;
    sampler->clearAllSounds();
    sampler->clearSounds();
    bool wasLoaded = false;

    std::unique_ptr<SamplerSound> sound (new SamplerSound (sample.getNote()));

    if (auto* data = sampleCache.getLayerData (true))
    {
        if (sound->insertLayerData (data))
            wasLoaded = data->loadAudioFile (sample.getFile());
        if (wasLoaded)
        {
        }
    }

    if (wasLoaded)
    {
        sound->setMidiChannel (1);
        sound->setDefaultLength();
        sampler->insertSound (sound.release());
    }
    else
    {
        sound.reset();
        DBG("[VCP] failed to load sample: " << sample.getFile().getFileName());
    }
}

void AudioEngine::panic()
{
    if (shouldPanic.compareAndSetBool (1, 0))
    {
        DBG("[VCP] panic requested");
    }
}

void AudioEngine::setAudioProcessor (AudioProcessor* newProcessor)
{
    jassert(newProcessor != nullptr);
    if (! newProcessor) return;

    std::unique_ptr<AudioProcessor> next (newProcessor);
    if (prepared)
        prepare (*next);

    {
        ScopedLock sl (render->getCallbackLock());
        processor.swap (next);
    }

    updatePluginProperties();

    if (next)
        next->releaseResources();
}

void AudioEngine::clearAudioProcessor()
{
    std::unique_ptr<AudioProcessor> deleter;
    {
        ScopedLock sl (render->getCallbackLock());
        processor.swap (deleter);
    }

    updatePluginProperties();

    if (deleter)
        deleter->releaseResources();
}

bool AudioEngine::isRendering() const { return render && render->isRendering(); }
void AudioEngine::cancelRendering() { if (render) render->cancel(); }
void AudioEngine::setRenderContext (const RenderContext& context) { if (render) render->setContext (context); }

Result AudioEngine::startRendering (const RenderContext& context)
{
    int latency = 0;

    if (context.source == SourceType::AudioPlugin)
    {
        if (processor == nullptr)
            return Result::fail ("No plugin selected to render");
        latency = processor->getLatencySamples();
    }
    else if (context.source == SourceType::MidiDevice)
    {
        latency = inputLatency + roundToInt (0.001 * sampleRate);
    }
    else
    {
        jassertfalse;
        return Result::fail (String("Invalid source specified: ") + String (context.source));
    }

    render->start (context, latency);
    return Result::ok();
}

ValueTree AudioEngine::getRenderedSamples() const
{
    return (render != nullptr) ? render->getSamples() : ValueTree();
}

void AudioEngine::addMidiMessage (const MidiMessage& msg)
{
    messageCollector.addMessageToQueue (msg);
}

void AudioEngine::setDefaultMidiOutput (const String& name)
{
    if (name.isEmpty())
    {
        midiOutName = String();
        std::unique_ptr<MidiOutput> deleter;

        {
            ScopedLock rsl (render->getCallbackLock());
            deleter.swap (midiOut);
        }
        
        if (deleter)
            deleter->stopBackgroundThread();

        return;
    }

    const int index = MidiOutput::getDevices().indexOf (name);
    std::unique_ptr<MidiOutput> newout (MidiOutput::openDevice (index));

    if (newout)
    {
        newout->startBackgroundThread();
        midiOutName = newout->getName();
        ScopedLock rsl (render->getCallbackLock());
        midiOut.swap (newout);
        DBG("[VCP] midi out: " << midiOut->getName());
    }

    if (newout)
    {
        DBG("[VCP] stopping: " << newout->getName());
        newout->stopBackgroundThread();
        newout.reset();
    }
}

void AudioEngine::updatePluginProperties()
{
    if (! processor)
    {
        pluginLatency   = 0;
        pluginNumIns    = 0;
        pluginNumIns    = 0;
        pluginChannels  = 0;
        return;
    }

    ScopedLock psl (processor->getCallbackLock());
    pluginLatency  = processor->getLatencySamples();
    pluginNumIns   = processor->getTotalNumInputChannels();
    pluginNumOuts  = processor->getTotalNumOutputChannels();
    DBG("[VCP] " << processor->getName() << " properties:");
    DBG("[VCP] inputs:  " << pluginNumIns);
    DBG("[VCP] outputs: " << pluginNumOuts);
    DBG("[VCP] latency: " << pluginLatency);
    pluginChannels = jmax (pluginNumIns, pluginNumOuts);
}

void AudioEngine::prepare (AudioProcessor& plugin)
{
    jassert (sampleRate > 0.0);
    jassert (bufferSize > 0);
    plugin.enableAllBuses();
    plugin.setRateAndBufferSizeDetails (sampleRate, bufferSize);
    plugin.prepareToPlay (sampleRate, bufferSize);
}

void AudioEngine::release (AudioProcessor& plugin)
{
    monitor->levelLeft.set (0.f);
    monitor->levelRight.set (0.f);
    plugin.releaseResources();
}

void AudioEngine::process (const float** input, int numInputs, 
                           float** output, int numOutputs, int nframes)
{
    jassert (sampleRate > 0 && bufferSize > 0);
    ScopedNoDenormals denormals;
    const auto nbytes = sizeof (float) * static_cast<size_t> (nframes);

    if (shouldProcess.get() != 1)
    {
        for (int c = 0; c < numOutputs; ++c)
            memset (output[c], 0, nbytes);
        return;
    }

    messageCollector.removeNextBlockOfMessages (incomingMidi, nframes);
    ScopedLock slr (render->getCallbackLock());
    render->renderCycleBegin();

    const bool rendering    = render->isRendering();
    const auto& context     = render->getContext();
    const int source        = render->getSourceType();
    renderBuffer.setSize (context.channels, nframes, false, false, true);
    pluginBuffer.setSize (pluginChannels, nframes, false, false, true);
    samplerAudio.setSize (2, nframes, false, false, true);
    
    render->getNextMidiBlock (renderMidi, nframes);
    
    if (! rendering)
    {
        renderMidi.addEvents (incomingMidi, 0, nframes, 0);
        if (shouldPanic.compareAndSetBool (0, 1))
        {
            addPanicMessages (renderMidi);
        }
    }

    if (auto* const proc = processor.get())
    {
        // plugin will clear the buffer so make a copy;
        pluginMidi.addEvents (renderMidi, 0, nframes, 0);
        ScopedLock slp (proc->getCallbackLock());
        proc->processBlock (pluginBuffer, pluginMidi);
    }

    if (auto* const out = midiOut.get())
    {
        if (! rendering || (rendering && source == SourceType::MidiDevice))
            out->sendBlockOfMessages (renderMidi, Time::getMillisecondCounterHiRes() + 1.0,
                                      sampleRate);
    }
    
    if (source == SourceType::AudioPlugin)
    {
        if (pluginChannels == 0 || pluginNumOuts == 0)
        {
            // noop
            renderBuffer.clear (0, nframes);
            pluginBuffer.clear (0, nframes);
        }
        else if (pluginNumOuts == context.channels)
        {
            // one-to-one channel match
            for (int c = context.channels; --c >= 0;)
                renderBuffer.copyFrom (c, 0, pluginBuffer, c, 0, nframes);
        }
        else if (context.channels == 1)
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
            for (int c = jmin (pluginNumOuts, context.channels); --c >= 0;)
                renderBuffer.copyFrom (c, 0, pluginBuffer, c, 0, nframes);
        }
    }
    else if (source == SourceType::MidiDevice)
    {
        if (numInputs == context.channels)
        {
            // one-to-one channel match
            for (int c = context.channels; --c >= 0;)
                renderBuffer.copyFrom (c, 0, input [c], nframes);
        }
        else if (context.channels == 1)
        {
            // mix to mono
            const float reduction = Decibels::decibelsToGain (-3.f);
            for (int c = numInputs; --c >= 0;)
                renderBuffer.addFrom (0, 0, input[c], nframes, reduction);
        }
        else
        {
            // fall back - copy the lesser of the channels
            for (int c = 0; c < context.channels; ++c)
            {
                if (c < numInputs)
                    renderBuffer.copyFrom (c, 0, input[c], nframes);
                else
                    renderBuffer.clear (c, 0, nframes);
            }
        }
    }
    else
    {
        // unkown source, clear
        renderBuffer.clear (0, nframes);
    }
    
    render->writeAudioFrames (renderBuffer);
    render->renderCycleEnd();

    sampler->renderNextBlock (samplerAudio, samplerMidi, 0, nframes);

    for (int c = 0; c < numOutputs; ++c)
        memset (output [c], 0, nbytes);
    
    if (context.channels == 1)
    {
        for (int c = numOutputs; --c >= 0;)
            memcpy (output[c], renderBuffer.getReadPointer (0), nbytes);
    }
    else
    {
        for (int c = jmin(numOutputs, context.channels); --c >= 0;)
            memcpy (output[c], renderBuffer.getReadPointer (c), nbytes);
    }
    
    const AudioSampleBuffer rmsBuf (output, numOutputs, nframes);
    if (numOutputs == 1)
    {
        monitor->levelLeft.set (rmsBuf.getRMSLevel (0, 0, nframes));
        monitor->levelRight.set (monitor->levelLeft.get());
    }
    else if (numOutputs > 1)
    {
        monitor->levelLeft.set (rmsBuf.getRMSLevel (0, 0, nframes));
        monitor->levelRight.set (rmsBuf.getRMSLevel (1, 0, nframes));
    }
    
    renderMidi.clear();
    incomingMidi.clear();
    pluginMidi.clear();
    samplerMidi.clear();
}

void AudioEngine::prepare (double expectedSampleRate, int maxBufferSize,
                           int numInputs, int numOutputs)
{
    if (prepared)
        release();
    
    ScopedLock sl (render->getCallbackLock());
    sampleRate          = expectedSampleRate;
    bufferSize          = maxBufferSize;
    numInputChans       = numInputs;
    numOutputChans      = numOutputs;
    
    inputLatency        = 0;
    outputLatency       = 0;
    extraLatency        = 0;
    pluginLatency       = 0;

    messageCollector.reset (sampleRate);
    channels.calloc ((size_t) jmax (numInputChans, numOutputChans) + 2);
    render->prepare (sampleRate, bufferSize);

    sampler->setCurrentPlaybackSampleRate (sampleRate);
    
    if (processor)
    {
        prepare (*processor);
        updatePluginProperties();
        pluginBuffer.setSize (pluginChannels, bufferSize, false, false, true);
    }

    prepared = true;
}

void AudioEngine::prepare (AudioIODevice* device)
{
    prepare (device->getCurrentSampleRate(),
             device->getCurrentBufferSizeSamples(),
             device->getActiveInputChannels().countNumberOfSetBits(),
             device->getActiveOutputChannels().countNumberOfSetBits());
    inputLatency      = device->getInputLatencyInSamples();
    outputLatency     = device->getOutputLatencyInSamples();
}

void AudioEngine::release()
{
    const ScopedLock sl (render->getCallbackLock());
    prepared = false;

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

void AudioEngine::addPanicMessages (MidiBuffer& buffer)
{
    for (int c = 1; c <= 16; ++c)
    {
        buffer.addEvent (MidiMessage::allNotesOff (c), 0);
        buffer.addEvent (MidiMessage::allSoundOff (c), 0);
        // sustain pedal off
        buffer.addEvent (MidiMessage::controllerEvent (c, 64, 0), 0);
        // Sostenuto off
        buffer.addEvent (MidiMessage::controllerEvent (c, 66, 0), 0);
        // Hold off
        buffer.addEvent (MidiMessage::controllerEvent (c, 69, 0), 0);
    }
}

}
