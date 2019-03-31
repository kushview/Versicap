#pragma once

#include "RenderContext.h"

namespace vcp {

class Render
{
public:
    Render();
    ~Render();

    void prepare (double sampleRate, int blockSize);
    void process (int nframes);

private:
    RenderContext context;
    int64 frame;
    int layer = 0;
    OwnedArray<MidiMessageSequence> layerMidi;
};

}
