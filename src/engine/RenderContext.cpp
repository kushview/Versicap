
#include "engine/RenderContext.h"
#include "Versicap.h"

namespace vcp {

File RenderContext::getCaptureDir() const
{
    String path = outputPath;
    if (! File::isAbsolutePath (path))
        path = Versicap::getSamplesPath().getChildFile("DataPath").getFullPathName();
    
    if (! File::isAbsolutePath (path))
    {
        jassertfalse;
        return File();
    }

    File directory (path);
    return directory.getChildFile ("capture");
}

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
            .setProperty ("instrumentName", instrumentName, nullptr)
            .setProperty ("outputPath", outputPath, nullptr)
            .setProperty ("channels",   channels, nullptr);
    
    auto layers = versicap.getOrCreateChildWithName ("layers", nullptr);
    for (int i = 0; i < 4; ++i)
    {
        #if 0
        ValueTree layer ("layer");
        layer.setProperty ("enabled", layerEnabled [i], nullptr)
             .setProperty ("velocity", layerVelocities [i], nullptr);
        layers.appendChild (layer, nullptr);
        #endif
    }

    return versicap;
}

LayerRenderDetails* RenderContext::createLayerRenderDetails (const int layerIdx,
                                                             const double sourceSampleRate,
                                                             AudioFormatManager& formats,
                                                             TimeSliceThread& thread) const
{
    jassert (sourceSampleRate > 0.0);
    jassert (isPositiveAndBelow (layerIdx, layers.size()));
    jassert (keyStride > 0);
    const auto& layer = layers.getReference (layerIdx);

    std::unique_ptr<LayerRenderDetails> details;
    details.reset (new LayerRenderDetails());
    auto& seq = details->sequence;

    int key = keyStart;
    int64 frame = 0;

    const int64 noteFrames = static_cast<int64> (sourceSampleRate * ((double) layer.noteLength / 1000.0));
    const int64 tailFrames = static_cast<int64> (sourceSampleRate * ((double) layer.tailLength / 1000.0));
   
    const File directory (getCaptureDir());
    const auto extension = FormatType::getFileExtension (FormatType::fromSlug (format));

    if (isPositiveAndBelow (layer.midiProgram, 127))
    {
        auto pgc = MidiMessage::programChange (layer.midiChannel, layer.midiProgram);
        pgc.setTimeStamp (static_cast<double> (frame));
        seq.addEvent (pgc);
        frame += roundToInt (sourceSampleRate); // program delay
    }

    while (key <= keyEnd)
    {
        auto* const sample  = details->samples.add (new SampleInfo());
        sample->layerId     = layer.uuid;
        sample->index       = details->samples.size() - 1;
        sample->note        = key;
    
        String identifier;
        identifier << String(layerIdx).paddedLeft ('0', 3) << "_"
                   << String(key).paddedLeft ('0', 3);
        String fileName = identifier;
        fileName << "." << extension;
        sample->file = directory.getChildFile (fileName);

        auto noteOn  = MidiMessage::noteOn (layer.midiChannel, key, layer.velocity);
        noteOn.setTimeStamp (static_cast<double> (frame));
        sample->start = frame;
        frame += noteFrames;
        
        auto noteOff = MidiMessage::noteOff (layer.midiChannel, key);
        noteOff.setTimeStamp (static_cast<double> (frame));
        frame += tailFrames;
        sample->stop = frame;

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
        ctx.instrumentName      = tree.getProperty ("instrumentName", ctx.instrumentName);
        ctx.keyEnd              = tree.getProperty ("keyEnd", ctx.keyEnd);
        ctx.keyStart            = tree.getProperty ("keyStart", ctx.keyStart);
        ctx.keyStride           = tree.getProperty ("keyStride", ctx.keyStride);
        ctx.noteLength          = tree.getProperty ("noteLength", ctx.noteLength);
        ctx.tailLength          = tree.getProperty ("tailLength", ctx.tailLength);
        ctx.outputPath          = tree.getProperty ("outputPath", ctx.outputPath);
        ctx.channels            = tree.getProperty ("channels", ctx.channels);
        auto layers = tree.getChildWithName ("layers");
        
        for (int i = 0; i < 4; ++i)
        {
            #if 0
            auto l = layers.getChild (i);
            ctx.layerEnabled[i]     = (bool) l.getProperty("enabled", ctx.layerEnabled [i]);
            ctx.layerVelocities[i]  = l.getProperty("velocity", ctx.layerVelocities [i]);
            #endif
        }

        deleteAndZero (xml);
    }
}

}