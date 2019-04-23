
#include "engine/AudioEngine.h"
#include "PluginManager.h"
#include "Render.h"

namespace vcp {

AudioEngine::AudioEngine (AudioFormatManager& formatManager, 
                          AudioPluginFormatManager& pluginManager)
    : formats (formatManager),
      plugins (pluginManager)
{
    render.reset (new Render (formatManager));
    render->onCancelled = [this]() { if (onRenderCancelled) onRenderCancelled(); };
    render->onStarted   = [this]() { if (onRenderStarted)   onRenderStarted(); };
    render->onStopped   = [this]() { if (onRenderStopped)   onRenderStopped(); };
}

AudioEngine::~AudioEngine()
{
    render->onCancelled = render->onStarted = render->onStopped = nullptr;
    render.reset();
}

void AudioEngine::setSourceType (SourceType type)
{
    if (type == sourceType.get())
        return;
    sourceType.set (type);
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
Result AudioEngine::startRendering (const RenderContext& context)
{
    int latency = 0;
    sourceType.set (context.source);

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

    const bool rendering = render->isRendering();
    const auto& context = render->getContext();
    const int source = sourceType.get();
    renderBuffer.setSize (context.channels, nframes, false, false, true);
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
       #if 0
        if (auto* const out = devices->getDefaultMidiOutput())
        {
            out->sendBlockOfMessages (renderMidi,
                Time::getMillisecondCounterHiRes() + 1.0,
                sampleRate);
        }
       #endif
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
            for (int c = jmin (numInputs, context.channels); --c >= 0;)
                renderBuffer.copyFrom (c, 0, input[c], nframes);
            for (int c = context.channels; c < numInputs; ++c)
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
    
    renderMidi.clear();
    incomingMidi.clear();
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

}
