#pragma once

#include "JuceHeader.h"
#include "Types.h"

namespace vcp {

struct RenderLayer
{
    int index   = 0;
    int64 start = 0;
    int64 stop  = 0;

    File file;
    std::unique_ptr<AudioFormatWriter::ThreadedWriter> writer;
};

struct LayerRenderDetails
{
    MidiMessageSequence sequence;
    OwnedArray<RenderLayer> frames;
    
    int getNumRenderLayers() const { return frames.size(); }
    RenderLayer* getRenderLayer (const int i) { return frames.getUnchecked (i); }
    int getNextRenderLayerIndex (const int64 frame)
    {
        const auto total = frames.size();
        int i;

        for (i = 0; i < total; ++i)
            if (frames.getUnchecked(i)->start >= frame || frames.getUnchecked(i)->stop >= frame)
                return i;

        return i;
    }

    int64 getHighestEndFrame() const
    {
        int64 frame = 0;
        for (auto* const lf : frames)
            if (lf->stop > frame)
                frame = lf->stop;
        return frame;
    }
};

struct RenderContext
{
    int source                  = SourceType::MidiDevice;
    int sourceChannels          = 2;

    int keyStart                = 36;   // C2
    int keyEnd                  = 60;   // C4
    int keyStride               = 4;    // 4 semi tones

    String baseName             = "Sample";
    int noteLength              = 3000;
    int tailLength              = 1000;

    bool layerEnabled[4]        { true, false, false, false };
    int layerVelocities[4]      { 127, 96, 64, 32 };

    int loopMode                = 0;
    int loopStart               = 500;
    int loopEnd                 = 2500;
    int crossfadeLength         = 0;

    String instrumentName       = "Instrument";
    String outputPath           = String();
    String format               = "wave";
    int channels                = 2;
    int bitDepth                = 16;
    double sampleRate           = 44100.0;

    ValueTree createValueTree() const;
    void writeToFile (const File& file) const;
    void restoreFromFile (const File& file);

    LayerRenderDetails* createLayerRenderDetails (const int layer, 
                                                  const double sourceSampleRate,
                                                  AudioFormatManager& formats,
                                                  TimeSliceThread& thread) const;
};

}
