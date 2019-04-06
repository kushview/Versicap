#pragma once

#include "RenderContext.h"

namespace vcp {

class Render : public AsyncUpdater
{
public:
    Render (AudioFormatManager& f);
    ~Render();

    void prepare (double newSampleRate, int newBlockSize);
    void process (int nframes);
    void release();

    void start (const RenderContext& newContext);
    void stop();

    bool isRendering() const { return rendering.get() == 1; }
    CriticalSection& getCallbackLock() { return lock; }

    void handleAsyncUpdate() override;
private:
    TimeSliceThread thread;
    AudioFormatManager& formats;
    RenderContext context;
    CriticalSection lock;
    bool prepared = false;
    double sampleRate = 0.0;
    int blockSize = 0;

    Atomic<int> rendering { 0 };
    Atomic<int> renderingRequest { 0 };

    int64 frame = 0;
    int event = 0;
    int layer = 0;
    OwnedArray<LayerRenderDetails> details;

    void reset();
};

}
