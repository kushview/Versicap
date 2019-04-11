
#include "RenderContext.h"
#include "Versicap.h"

namespace vcp {

ValueTree RenderContext::createValueTree() const
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
            .setProperty ("channels",   channels, nullptr);
    
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

LayerRenderDetails* RenderContext::createLayerRenderDetails (const int layer,
                                                             const double sourceSampleRate,
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
    int64 frame = 0;
    const int64 noteFrames = static_cast<int64> (sourceSampleRate * ((double) noteLength / 1000.0));
    const int64 tailFrames = static_cast<int64> (sourceSampleRate * ((double) tailLength / 1000.0));
    
    while (key <= keyEnd)
    {
        auto* const renderLayer = details->frames.add (new RenderLayer());
        renderLayer->index = details->frames.size() - 1;
        
        String path = outputPath;
        
        if (! File::isAbsolutePath (path))
            path = Versicap::getSamplesPath().getFullPathName();

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
            if (file.existsAsFile())
                file.deleteFile();
            std::unique_ptr<FileOutputStream> stream;
            stream.reset (file.createOutputStream());
            
            if (stream)
            {
                if (auto* writer = format->createWriterFor (
                        stream.get(),
                        sourceSampleRate,
                        this->channels,
                        this->bitDepth,
                        StringPairArray(),
                        0
                    ))
                {
                    renderLayer->file = file;
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

void RenderContext::writeToFile (const File& file) const
{
    const auto tree = createValueTree();
    if (auto* xml = tree.createXml())
    {
        xml->writeToFile (file, String());
        deleteAndZero (xml);
    }
}

void RenderContext::restoreFromFile (const File& file)
{
    if (auto* xml = XmlDocument::parse (file))
    {
        auto tree = ValueTree::fromXml (*xml);
        RenderContext& ctx = *this;
        ctx.source              = tree.getProperty ("source", ctx.source);
        ctx.baseName            = tree.getProperty ("baseName", ctx.baseName);
        ctx.crossfadeLength     = tree.getProperty ("crossfadeLength", ctx.crossfadeLength);
        ctx.instrumentName      = tree.getProperty ("instrumentName", ctx.instrumentName);
        ctx.keyEnd              = tree.getProperty ("keyEnd", ctx.keyEnd);
        ctx.keyStart            = tree.getProperty ("keyStart", ctx.keyStart);
        ctx.keyStride           = tree.getProperty ("keyStride", ctx.keyStride);
        ctx.loopEnd             = tree.getProperty ("loopEnd", ctx.loopEnd);
        ctx.loopMode            = tree.getProperty ("loopMode", ctx.loopMode);
        ctx.loopStart           = tree.getProperty ("loopStart", ctx.loopStart);
        ctx.noteLength          = tree.getProperty ("noteLength", ctx.noteLength);
        ctx.tailLength          = tree.getProperty ("tailLength", ctx.tailLength);
        ctx.outputPath          = tree.getProperty ("outputPath", ctx.outputPath);
        ctx.channels            = tree.getProperty ("channels", ctx.channels);
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

}