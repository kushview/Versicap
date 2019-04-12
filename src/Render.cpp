
#include "Render.h"

/*
source  = 22050
dest    = 44100
ratio   = 0.5                   = source / dest
provide = 1024                  = produce * ratio
produce = 2048                  = provide / ratio

source  = 48000
dest    = 44100
ratio   = 1.08843537414966      = source / dest
provide = 1024                  = produce * ratio
produce = 940.799999999999882   = provide / ratio

source  = 44100
dest    = 48000
ratio   = 0.91875               = source / dest
provide = 1024                  = produce * ratio
produce = 1114.557823129251701  = provide / ratio
*/

namespace vcp {

Render::Render (AudioFormatManager& f)
    : formats (f), 
      thread ("vcprt"),
      started (*this),
      stopped (*this)
{ }

Render::~Render()
{
    thread.stopThread (2 * 1000);
}

void Render::reset()
{
    frame = 0;
    layer = 0;
}

void Render::prepare (double newSampleRate, int newBufferSize)
{
    if (prepared)
        release();
    jassert (! prepared);
    reset();
    sampleRate  = newSampleRate;
    blockSize   = newBufferSize;
    thread.startThread();
    prepared = true;

    const int sampleFileNumChans = 2;
    channels.calloc (sampleFileNumChans + 2);
}

void Render::renderCycleBegin()
{
    if (renderingRequest.get() != rendering.get())
    {
        rendering.set (renderingRequest.get());
        if (isRendering())
        {
            DBG("[VCP] rendering started");
            stopped.cancelPendingUpdate();
            started.cancelPendingUpdate();
            started.triggerAsyncUpdate();
            reset();
        }
        else
        {
            DBG("[VCP] rendering stopped");
            started.cancelPendingUpdate();
            stopped.cancelPendingUpdate();
            triggerAsyncUpdate();
        }
    }
}

void Render::getNextMidiBlock (MidiBuffer& buffer, int nframes)
{
    
    if (! isRendering())
        return;

    ScopedLock sl (getCallbackLock());

    if (layer >= nlayers)
    {
        renderingRequest.compareAndSetBool (0, 1);
        return;
    }

    auto* const detail  = details.getUnchecked (layer);
    const auto& midi    = detail->sequence;
    const int numEvents = midi.getNumEvents();
    const double start  = static_cast<double> (frame);
    const int64 endFrame  = frame + nframes;
    const double end    = static_cast<double> (endFrame);
    bool layerChanged   = false;
    int i;
    
    buffer.clear();

    for (i = midi.getNextIndexAtTime (start); i < numEvents;)
    {
        const auto* const ev = midi.getEventPointer (i);
        const auto& msg = ev->message;
        const double timestamp = msg.getTimeStamp();
        if (timestamp >= end)
            break;
        
        buffer.addEvent (msg, roundToInt (timestamp - start));
        
       #if 1
        if (msg.isNoteOn())
        {
            DBG("note on: " << MidiMessage::getMidiNoteName (msg.getNoteNumber(), true, true, 4) << " - "
                << static_cast<int64> (msg.getTimeStamp()));
        }
        else if (msg.isNoteOff())
        {
            DBG("note off: " << MidiMessage::getMidiNoteName (msg.getNoteNumber(), true, true, 4) << " - "
                << static_cast<int64> (msg.getTimeStamp()));
        }
       #endif

        ++i;
    }
}

void Render::writeAudioFrames (AudioSampleBuffer& audio)
{
    if (! isRendering())
        return;

    if (layer >= nlayers)
    {
        renderingRequest.compareAndSetBool (0, 1);
        return;
    }

    ScopedLock sl (getCallbackLock());

    const int nframes           = audio.getNumSamples();
    auto* const detail          = details.getUnchecked (layer);
    const int numDetails        = detail->getNumRenderLayers();
    const auto lastStopFrame    = detail->getHighestEndFrame();
    const int64 startFrame      = frame - writerDelay;
    const int64 endFrame        = startFrame + nframes;
    
    for (int i = detail->getNextRenderLayerIndex (startFrame); i < numDetails;)
    {
        auto* const render = detail->getRenderLayer (i);
        if (render->start >= endFrame)
            break;
   
        if (render->start >= startFrame && render->start < endFrame)
        {
            const int localFrame = render->start - startFrame;
            for (int c = 0; c < context.channels; ++c)
                channels[c] = audio.getWritePointer (c, localFrame);
            render->writer->write (channels.get(), endFrame - render->start);
        }
        else if (render->stop >= startFrame && render->stop < endFrame)
        {
            for (int c = 0; c < context.channels; ++c)
                channels[c] = audio.getWritePointer (c);
            render->writer->write (channels.get(), render->stop - startFrame);
        }
        else if (startFrame >= render->start && startFrame < render->stop)
        {
            for (int c = 0; c < context.channels; ++c)
                channels[c] = audio.getWritePointer (c);
            render->writer->write (channels.get(), nframes);
        }

        ++i;
    }
    
    if (lastStopFrame >= startFrame && lastStopFrame < endFrame)
    {
        ++layer;
        frame = 0;
        event = 0;
    }
    else
    {
        frame += nframes;
    }
}

void Render::renderCycleEnd()
{

}

void Render::release()
{
    details.clearQuick (true);
    prepared = false;
}

void Render::handleAsyncUpdate()
{
    if (isRendering())
    {
        jassertfalse;
        return;
    }

    OwnedArray<LayerRenderDetails> old;
    {
        ScopedLock sl (getCallbackLock());
        details.swapWith (old);
    }

    for (auto* detail : old)
        for (auto* frame : detail->frames)
            frame->writer.reset();
    
    stopped.triggerAsyncUpdate();
}

void Render::start (const RenderContext& newContext, int latencySamples)
{
    if (isRendering())
        return;

    jassert (sampleRate > 0.0);
    jassert (blockSize > 0);

    OwnedArray<LayerRenderDetails> newDetails;

    for (int i = 0; i < newContext.layers.size(); ++i)
    {
        newDetails.add (newContext.createLayerRenderDetails (
                        i, sampleRate, formats, thread));
    }

    {
        ScopedLock sl (getCallbackLock());
        nlayers         = jmax (0, newContext.layers.size());
        writerDelay     = jmax (0, newContext.latency + latencySamples);
        context         = newContext;
        details.swapWith (newDetails);
    }

    if (renderingRequest.compareAndSetBool (1, 0))
    {
        DBG("[VCP] render start requested");
        DBG("[VCP] compensate samples: " << writerDelay);
    }

    newDetails.clear();
}

void Render::stop()
{
    if (renderingRequest.compareAndSetBool (0, 1))
    {
        DBG("[VCP] render stop requested");
    }
}

}
