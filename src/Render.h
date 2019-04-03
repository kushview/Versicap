#pragma once

#include "RenderContext.h"

namespace vcp {

class Render
{
public:
    Render();
    ~Render();

    void prepare (double newSampleRate, int newBlockSize);
    void process (int nframes);
    void release();

    CriticalSection& getCallbackLock() { return lock; }

private:
    RenderContext context;
    CriticalSection lock;
    bool prepared = false;
    double sampleRate = 0.0;
    int blockSize = 0;

    

    int64 frame = 0;
    int layer = 0;
    OwnedArray<MidiMessageSequence> layerMidi;

    void reset();
};

}
