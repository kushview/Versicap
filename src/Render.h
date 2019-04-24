#pragma once

#include "engine/ChannelDelay.h"
#include "RenderContext.h"

namespace vcp {

class Render : public AsyncUpdater
{
public:
    Render (AudioFormatManager& f);
    ~Render();

    //=========================================================================
    void prepare (double newSampleRate, int newBlockSize);
    void renderCycleBegin();
    void getNextMidiBlock (MidiBuffer& midi, int nframes);
    void writeAudioFrames (AudioSampleBuffer& audio);
    void renderCycleEnd();
    void release();

    //=========================================================================
    void start (const RenderContext& newContext, int latencySamples = 0);
    void cancel();

    //=========================================================================
    /** Returns true if currently rendering or rendering has been requested */
    bool isRendering() const { return renderingRequest.get() != 0 || rendering.get() != 0; }
    
    //=========================================================================
    /** Update the context.  The properties here may not be used for rendering
        It is here so that when previewing the settings match */
    void setContext (const RenderContext& newContext);

    /** Returns the current render context used for preview or rendering
        Only use this in the audio thread or lock with getCallbackLock() */
    const RenderContext& getContext() const { return context; }

    /** Returns the current source type.  Only use in audio thread or lock
        with getCallbackLock() */
    int getSourceType() const { return context.source; }

    //=========================================================================
    /** Returns sample metadata after rendering has completed */
    ValueTree getSamples() const { return samples; }

    //=========================================================================
    CriticalSection& getCallbackLock() { return lock; }

    //=========================================================================
    /** @internal */
    void handleAsyncUpdate() override;

    std::function<void()> onStopped;
    std::function<void()> onStarted;
    std::function<void()> onCancelled;

private:
    Identifier samplesType { "samples" };
    ValueTree samples;
    TimeSliceThread thread;
    AudioFormatManager& formats;
    
    CriticalSection lock;
    RenderContext context;
    bool prepared = false;
    double sampleRate = 0.0;
    int blockSize = 0;

    Atomic<int> rendering { 0 };
    Atomic<int> renderingRequest { 0 };
    Atomic<int> shouldCancel { 0 };

    int writerDelay = 0;

    int64 frame = 0;
    int event = 0;
    int layer = 0;
    int nlayers = 0;
    
    HeapBlock<float*> channels;
    OwnedArray<LayerRenderDetails> details;

    struct Started : public AsyncUpdater
    {
        Started (Render& r) : render (r) { }
        void handleAsyncUpdate() { if (render.onStarted) render.onStarted(); }
        Render& render;
    } started;

    struct Stopped : public AsyncUpdater
    {
        Stopped (Render& r) : render (r) {}
        void handleAsyncUpdate()  { if (render.onStopped) render.onStopped(); }
        Render& render;
    } stopped;

    struct Cancelled : public AsyncUpdater
    {
        Cancelled (Render& r) : render (r) { }
        void handleAsyncUpdate()  { if (render.onCancelled) render.onCancelled(); }
        Render& render;
    } cancelled;

    void reset();
};

}
