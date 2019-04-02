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

private:
    RenderContext context;
    bool prepared = false;
    double sampleRate = 0.0;
    int blockSize = 0;
    int64 frame = 0;
    int layer = 0;
    OwnedArray<MidiMessageSequence> layerMidi;

    void reset();
};

}
