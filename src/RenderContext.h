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
    int outputChannels          = 2;

    ValueTree createValueTree() const
    {
        ValueTree versicap ("versicap");
        versicap.setProperty ("source",     source, nullptr)
                .setProperty ("keyStart",   keyStart, nullptr)
                .setProperty ("keyEnd",     keyEnd, nullptr)
                .setProperty ("keyStride",  keyStride, nullptr)
                .setProperty ("baseName",   baseName, nullptr)
                .setProperty ("noteLength", noteLength, nullptr)
                .setProperty ("tailLength", tailLength, nullptr)
                .setProperty ("loopMode",   loopMode, nullptr)
                .setProperty ("loopStart",  loopStart, nullptr)
                .setProperty ("crossfadeLength", crossfadeLength, nullptr)
                .setProperty ("instrumentName", instrumentName, nullptr)
                .setProperty ("outputPath", outputPath, nullptr)
                .setProperty ("outputChannels", outputChannels, nullptr);
        
        auto layers = versicap.getOrCreateChildWithName ("layers", nullptr);
        for (int i = 0; i < 4; ++i)
        {
            ValueTree layer ("layer");
            layer.setProperty ("enabled", layerEnabled [i], nullptr)
                 .setProperty ("velocity", layerVelocities [i], nullptr);
            layers.appendChild (layer, nullptr);
        }

        return versicap;
    }

    void writeToFile (const File& file) const
    {
        const auto tree = createValueTree();
        if (auto* xml = tree.createXml())
        {
            xml->writeToFile (file, String());
            deleteAndZero (xml);
        }
    }

    void restoreFromFile (const File& file)
    {
        if (auto* xml = XmlDocument::parse (file))
        {
            auto tree = ValueTree::fromXml (*xml);
            RenderContext& ctx = *this;
            ctx.source              = tree.getProperty ("source", ctx.source);
            ctx.baseName            = tree.getProperty ("baseName", ctx.baseName);
            ctx.crossfadeLength     = tree.getProperty ("crossfadeLength", ctx.crossfadeLength);;
            ctx.instrumentName      = tree.getProperty ("instrumentName", ctx.instrumentName);;
            ctx.keyEnd              = tree.getProperty ("keyEnd", ctx.keyEnd);;
            ctx.keyStart            = tree.getProperty ("keyStart", ctx.keyStart);;
            ctx.keyStride           = tree.getProperty ("keyStride", ctx.keyStride);;
            ctx.loopEnd             = tree.getProperty ("loopEnd", ctx.loopEnd);;
            ctx.loopMode            = tree.getProperty ("loopMode", ctx.loopMode);;
            ctx.loopStart           = tree.getProperty ("loopStart", ctx.loopStart);;
            ctx.noteLength          = tree.getProperty ("noteLength", ctx.noteLength);;
            ctx.tailLength          = tree.getProperty ("tailLength", ctx.tailLength);;
            ctx.outputPath          = tree.getProperty ("outputPath", ctx.outputPath);;
            ctx.outputChannels      = tree.getProperty ("outputChannels", ctx.outputChannels);
            auto layers = tree.getChildWithName ("layers");
            
            for (int i = 0; i < 4; ++i)
            {
                auto l = layers.getChild (i);
                ctx.layerEnabled[i]     = (bool) l.getProperty("enabled", ctx.layerEnabled [i]);
                ctx.layerVelocities[i]  = l.getProperty("velocity", ctx.layerVelocities [i]);
            }

            deleteAndZero (xml);
        }
    }

    LayerRenderDetails* createLayerRenderDetails (const int layer, const double sampleRate,
                                                  AudioFormatManager& formats,
                                                  TimeSliceThread& thread) const;
};

}
