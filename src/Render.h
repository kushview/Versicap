#pragma once

#include "ChannelDelay.h"
#include "RenderContext.h"

namespace vcp {

class Render : public AsyncUpdater
{
public:
    Render (AudioFormatManager& f);
    ~Render();

    void prepare (double newSampleRate, int newBlockSize);
    void process (int nframes);

    void renderCycleBegin();
    void getNextMidiBlock (MidiBuffer& midi, int nframes);
    void writeAudioFrames (AudioSampleBuffer& audio);
    void renderCycleEnd();

    void release();

    void start (const RenderContext& newContext, int latencySamples = 0);
    void stop();

    bool isRendering() const { return renderingRequest.get() == 1 || rendering.get() == 1; }
    CriticalSection& getCallbackLock() { return lock; }
    
    void handleAsyncUpdate() override;

    const RenderContext& getContext() const { return context; }
    int getSourceType() const { return context.source; }

private:
    TimeSliceThread thread;
    AudioFormatManager& formats;
    
    CriticalSection lock;
    RenderContext context;
    bool prepared = false;
    double sampleRate = 0.0;
    int blockSize = 0;

    Atomic<int> rendering { 0 };
    Atomic<int> renderingRequest { 0 };
    int writerDelay = 0;

    int64 frame = 0;
    int event = 0;
    int layer = 0;
    
    HeapBlock<float*> channels;
    OwnedArray<LayerRenderDetails> details;

    std::unique_ptr<ChannelDelay> delay;
    
    void reset();
};

}
