
#include "Render.h"

namespace vcp {

Render::Render() {}
Render::~Render() {}

void Render::reset()
{
    sampleRate  = 0.0;
    blockSize   = 0;
    frame       = 0;
    layer       = 0;
}

void Render::prepare (double sr, int block)
{
    reset();
    sampleRate  = sr;
    blockSize   = block;
    if (prepared)
        release();

    for (int i = 0; i < 4; ++i)
    {
        if (! context.layerEnabled [i])
        {
            layerMidi.add (new MidiMessageSequence());
        }
        else
        {
            layerMidi.add (context.createMidiMessageSequence (i, sampleRate));
        }
    }

    prepared = true;
}

void Render::process (int nframes)
{
    if (! context.layerEnabled [layer])
    {
        frame = 0;
        ++layer;
        return;
    }

    frame += nframes;
}

void Render::release()
{
    layerMidi.clearQuick (true);
    prepared = false;
}

}
