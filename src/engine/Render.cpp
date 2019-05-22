
#include "engine/Render.h"
#include "Tags.h"

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
      thread ("vcprender"),
      started (*this),
      stopped (*this),
      cancelled (*this),
      progress (*this)
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
        if (msg.isProgramChange())
        {
            DBG("[VCP] program change: " << msg.getProgramChangeNumber());
        }
        if (msg.isNoteOn())
        {
            DBG("[VCP] note on: " << MidiMessage::getMidiNoteName (msg.getNoteNumber(), true, true, 4) << " - "
                << static_cast<int64> (msg.getTimeStamp()) << " velocity: " << (int) msg.getVelocity());
        }
        else if (msg.isNoteOff())
        {
            DBG("[VCP] note off: " << MidiMessage::getMidiNoteName (msg.getNoteNumber(), true, true, 4) << " - "
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
    const int numDetails        = detail->getNumSamples();
    const auto lastStopFrame    = detail->getHighestEndFrame();
    const int64 startFrame      = frame - writerDelay;
    const int64 endFrame        = startFrame + nframes;
    
    for (int i = detail->getNextSampleIndex (startFrame); i < numDetails;)
    {
        auto* const render = detail->getSample (i);
        if (render->start >= endFrame)
            break;
   
        if (render->start >= startFrame && render->start < endFrame)
        {
            progress.triggerAsyncUpdate();
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
    RenderContext ctx;
    {
        ScopedLock sl (getCallbackLock());
        details.swapWith (old);
        ctx = context;
    }

    const auto captureDir = ctx.getCaptureDir();
    const auto projectDir = captureDir.getParentDirectory();
    const auto samplesDir = projectDir.getChildFile ("samples");
    const bool wasCancelled = shouldCancel.get() == 1;

    if (! wasCancelled)
    {
        ValueTree manifest (Tags::samples);
        
        for (auto* detail : old)
        {
            for (auto* const info : detail->samples)
            {
                ValueTree sample (Tags::sample);
                info->writer.reset();
                auto totalTime = static_cast<double> (info->stop - info->start) / sampleRate;

                sample.setProperty (Tags::uuid, Uuid().toString(), nullptr)
                      .setProperty (Tags::layer, info->layerId.toString(), nullptr)
                      .setProperty (Tags::file, info->file.getFileName(), nullptr)
                      .setProperty (Tags::note, info->note, nullptr)
                      .setProperty (Tags::sampleRate, sampleRate, nullptr)
                      .setProperty (Tags::length, totalTime, nullptr)
                      .setProperty (Tags::timeIn, 0.0, nullptr)
                      .setProperty (Tags::timeOut, totalTime, nullptr);
                manifest.appendChild (sample, nullptr);
            }
        }
        
        if (samplesDir.exists())
            samplesDir.deleteRecursively();
        captureDir.copyDirectoryTo (samplesDir);
        captureDir.deleteRecursively();
        
        samples = manifest;
        stopped.triggerAsyncUpdate();
    }
    else
    {
        for (auto* detail : old)
            for (auto* const info : detail->samples)
                info->writer.reset();
        captureDir.deleteRecursively();
        samples = ValueTree (Tags::samples);
        cancelled.triggerAsyncUpdate();
    }

    shouldCancel.set (0);
}

void Render::setContext (const RenderContext& newContext)
{
    if (rendering.get() != 0 || renderingRequest.get() != 0)
        return;
    ScopedLock rsl (lock);
    context = newContext;
}

Result Render::start (const RenderContext& newContext, int latencySamples)
{
    if (isRendering())
        return Result::fail ("recording already in progress");

    jassert (sampleRate > 0.0);
    jassert (blockSize > 0);

    const auto extension = FormatType::getFileExtension (FormatType::fromSlug (newContext.format));
    auto* const audioFormat = formats.findFormatForFileExtension (extension);
    if (! audioFormat)
    {
        jassertfalse;
        return Result::fail ("could not create encoder for recording");
    }

    OwnedArray<LayerRenderDetails> newDetails;
    const File directory = newContext.getCaptureDir();
    if (directory.exists())
        directory.deleteRecursively();
    directory.createDirectory();
    steps.clearQuick();
    totalSteps = 0;
    for (int i = 0; i < newContext.layers.size(); ++i)
    {
        String layerName = "Layer "; layerName << int (i + 1);
        auto* details = newDetails.add (newContext.createLayerRenderDetails (
                                        i, sampleRate, formats, thread));
        for (auto* const sample : details->samples)
        {
            const auto file = sample->file;
            std::unique_ptr<FileOutputStream> stream (file.createOutputStream());
            String step = layerName;
            step << " - " << MidiMessage::getMidiNoteName (sample->note, true, true, 4);
            steps.add (step);

            if (stream)
            {
                if (auto* const writer = audioFormat->createWriterFor (
                        stream.get(),
                        sampleRate,
                        newContext.channels,
                        newContext.bitDepth,
                        StringPairArray(),
                        0
                    ))
                {
                    sample->writer.reset (new AudioFormatWriter::ThreadedWriter (writer, thread, 8192));
                    stream.release();
                    DBG("[VCP] " << file.getFullPathName());
                }
                else
                {
                    jassertfalse;
                    return Result::fail ("couldn't create writer for recording");
                }
            }
            else
            {
                String message = "could not open ";
                message << sample->file.getFileName() << " for recording";
                return Result::fail (message);
            }
        }
    }

    totalSteps = steps.size();
    samples = ValueTree (samplesType);

    {
        ScopedLock sl (getCallbackLock());
        nlayers         = jmax (0, newContext.layers.size());
        writerDelay     = jmax (0, newContext.latency + latencySamples);
        context         = newContext;
        details.swapWith (newDetails);
    }

    if (shouldCancel.compareAndSetBool (0, 1))
    {
        DBG("[VCP] cancel flag reset");
    }

    if (renderingRequest.compareAndSetBool (1, 0))
    {
        DBG("[VCP] render start requested");
        DBG("[VCP] compensate samples: " << writerDelay);
    }

    newDetails.clear();
    return Result::ok();
}

void Render::cancel()
{
    shouldCancel.set (1);
    if (renderingRequest.compareAndSetBool (0, 1))
    {
        DBG("[VCP] render cancel requested");
    }
}

}
