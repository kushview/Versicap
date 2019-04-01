#pragma once

#include "JuceHeader.h"

namespace vcp {

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

    String instrumentName       = String();
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

    MidiMessageSequence* createMidiMessageSequence (const int layer, const double sampleRate) const
    {
        jassert (sampleRate > 0.0);
        jassert (isPositiveAndBelow (layer, 4));
        jassert (keyStride > 0);

        std::unique_ptr<MidiMessageSequence> seq;
        seq.reset (new MidiMessageSequence());

        int key = keyStart;
        int64 frame = 0.0;
        const int64 noteFrames = static_cast<int64> (sampleRate * ((double) noteLength / 1000.0));
        const int64 tailFrames = static_cast<int64> (sampleRate * ((double) tailLength / 1000.0));
        
        while (key <= keyEnd)
        {
            const auto velocity = static_cast<uint8> (layerVelocities [layer]);
            auto noteOn  = MidiMessage::noteOn (1, key, velocity);
            noteOn.setTimeStamp (static_cast<double> (frame));
            frame += noteFrames;
            
            auto noteOff = MidiMessage::noteOff (1, key);
            noteOff.setTimeStamp (static_cast<double> (frame));
            frame += tailFrames;
            
            seq->addEvent (noteOn);
            seq->addEvent (noteOff);
            key += keyStride;
        }
        
        return seq.release();
    }
};

}
