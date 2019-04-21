
#include "engine/AudioEngine.h"
#include "PluginManager.h"
#include "Render.h"

namespace vcp {

AudioEngine::AudioEngine()
{

}

AudioEngine::~AudioEngine()
{
    
}

void AudioEngine::updatePluginProperties()
{
    if (! processor)
        return;
    
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

void AudioEngine::audioDeviceIOCallback (const float** input, int numInputs, 
                                         float** output, int numOutputs,
                                         int nframes)
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

void AudioEngine::audioDeviceAboutToStart (AudioIODevice* device)
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

void AudioEngine::audioDeviceStopped()
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

void AudioEngine::audioDeviceError (const String& errorMessage)
{
    ignoreUnused (errorMessage);
}

void AudioEngine::handleIncomingMidiMessage (MidiInput*, const MidiMessage& message)
{
    messageCollector.addMessageToQueue (message);
}

void AudioEngine::handlePartialSysexMessage (MidiInput* source, const uint8* messageData,
                                             int numBytesSoFar, double timestamp)
{
    ignoreUnused (source, messageData, numBytesSoFar, timestamp);
}

}