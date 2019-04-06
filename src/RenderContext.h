#pragma once

#include "JuceHeader.h"

namespace vcp {

struct RenderLayer
{
    int index;
    int64 start;
    int64 stop;
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
    int keyStart                = 36;   // C2
    int keyEnd                  = 60;   // C4
    int keyStride               = 4;

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
    
    ValueTree createValueTree() const
    {
        ValueTree versicap ("versicap");
        versicap.setProperty ("keyStart",   keyStart, nullptr)
                .setProperty ("keyEnd",     keyEnd, nullptr)
                .setProperty ("keyStride",  keyStride, nullptr)
                .setProperty ("baseName",   baseName, nullptr)
                .setProperty ("noteLength", noteLength, nullptr)
                .setProperty ("tailLength", tailLength, nullptr)
                .setProperty ("loopMode",   loopMode, nullptr)
                .setProperty ("loopStart",  loopStart, nullptr)
                .setProperty ("crossfadeLength", crossfadeLength, nullptr)
                .setProperty ("instrumentName", instrumentName, nullptr)
                .setProperty ("outputPath", outputPath, nullptr);
        
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
            ctx.outputPath          = tree.getProperty ("outputPath", ctx.outputPath);;
            ctx.tailLength          = tree.getProperty ("tailLength", ctx.tailLength);;
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
                                                  TimeSliceThread& thread) const
    {
        jassert (sampleRate > 0.0);
        jassert (isPositiveAndBelow (layer, 4));
        jassert (keyStride > 0);
        auto* const format = formats.findFormatForFileExtension ("wav");
        if (! format)
        {
            jassertfalse;
            return nullptr;
        }

        std::unique_ptr<LayerRenderDetails> details;
        details.reset (new LayerRenderDetails());
        auto& seq = details->sequence;

        int key = keyStart;
        int64 frame = 0.0;
        const int64 noteFrames = static_cast<int64> (sampleRate * ((double) noteLength / 1000.0));
        const int64 tailFrames = static_cast<int64> (sampleRate * ((double) tailLength / 1000.0));
        
        while (key <= keyEnd)
        {
            auto* const renderLayer = details->frames.add (new RenderLayer());
            renderLayer->index = details->frames.size() - 1;
            
            String path = outputPath;
            
            if (! File::isAbsolutePath (path))
                path = "/Users/mfisher/Desktop/TestOutput";

            if (File::isAbsolutePath (path))
            {
                File file (path);
                if (instrumentName.isNotEmpty())
                    file = file.getChildFile (instrumentName);
                file.createDirectory();

                String fileName = "0";
                fileName << layer << "_" << "0" << renderLayer->index << "_"
                    << MidiMessage::getMidiNoteName (key, true, true, 4)
                    << "_" << baseName << ".wav";
                file = file.getChildFile (fileName);
                std::unique_ptr<FileOutputStream> stream;
                stream.reset (file.createOutputStream());
                if (stream)
                {
                    if (auto* writer = format->createWriterFor (
                            stream.get(), sampleRate, 2, 16,
                            StringPairArray(), 0
                        ))
                    {
                        renderLayer->writer.reset (
                            new AudioFormatWriter::ThreadedWriter (writer, thread, 8192));
                        stream.release();
                        DBG("[VCP] " << file.getFullPathName());
                    }
                }
            }
            else
            {

            }
           
                

            

            const auto velocity = static_cast<uint8> (layerVelocities [layer]);
            auto noteOn  = MidiMessage::noteOn (1, key, velocity);
            noteOn.setTimeStamp (static_cast<double> (frame));
            renderLayer->start = frame;
            frame += noteFrames;
            
            auto noteOff = MidiMessage::noteOff (1, key);
            noteOff.setTimeStamp (static_cast<double> (frame));
            frame += tailFrames;
            renderLayer->stop = frame;

            seq.addEvent (noteOn);
            seq.addEvent (noteOff);
            key += keyStride;
        }
        
        return details.release();
    }
};

}
