#pragma once

#include "JuceHeader.h"
#include "Types.h"

namespace vcp {

struct SampleInfo
{
    Uuid layerId;
    int index   = 0;
    int note    = 0;
    
    int64 start = 0;
    int64 stop  = 0;

    File file;
    std::unique_ptr<AudioFormatWriter::ThreadedWriter> writer;
};

struct LayerRenderDetails
{
    MidiMessageSequence sequence;
    OwnedArray<SampleInfo> samples;
    
    int getNumSamples() const { return samples.size(); }
    SampleInfo* getSample (const int i) { return samples.getUnchecked (i); }
    int getNextSampleIndex (const int64 frame)
    {
        const auto total = samples.size();
        int i;

        for (i = 0; i < total; ++i)
            if (samples.getUnchecked(i)->start >= frame || samples.getUnchecked(i)->stop >= frame)
                return i;

        return i;
    }

    int64 getHighestEndFrame() const
    {
        int64 frame = 0;
        for (auto* const sample : samples)
            if (sample->stop > frame)
                frame = sample->stop;
        return frame;
    }
};

class RenderDetails
{
public:
    RenderDetails() = default;
    ~RenderDetails() = default;

private:
};

struct LayerInfo
{
    LayerInfo (const String& layerId, int layerVelocity)
        : uuid (layerId), velocity (static_cast<uint8> (layerVelocity)) {}
    LayerInfo (const String& layerId, uint8 layerVelocity)
        : uuid (layerId), velocity (layerVelocity) {}
    LayerInfo (const LayerInfo& o) { operator= (o); }

    Uuid    uuid;
    uint8   velocity        = 127;
    int     noteLength      = 3000;
    int     tailLength      = 1000;
    int     midiChannel     = 1;
    int     midiProgram     = -1;

    LayerInfo& operator= (const LayerInfo& o)
    {
        uuid            = o.uuid;
        velocity        = o.velocity;
        noteLength      = o.noteLength;
        tailLength      = o.tailLength;
        midiChannel     = o.midiChannel;
        midiProgram     = o.midiProgram;
        return *this;
    }
};

struct RenderContext
{
    int source                  = SourceType::MidiDevice;

    int keyStart                = 36;   // C2
    int keyEnd                  = 60;   // C4
    int keyStride               = 4;    // 4 semi tones

    String baseName             = "Sample";
    int noteLength              = 3000;
    int tailLength              = 1000;

    Array<LayerInfo> layers;

    String instrumentName       = "Instrument";
    String outputPath           = String();
    String format               = "wave";
    int channels                = 2;
    int bitDepth                = 16;
    int latency                 = 0;
    double sampleRate           = 44100.0;

    ValueTree createValueTree() const;
    void writeToFile (const File& file) const;
    void restoreFromFile (const File& file);

    File getCaptureDir() const;
    LayerRenderDetails* createLayerRenderDetails (const int layer, 
                                                  const double sourceSampleRate,
                                                  AudioFormatManager& formats,
                                                  TimeSliceThread& thread) const;
};

}
