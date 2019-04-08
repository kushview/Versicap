
#include "RenderContext.h"
#include "Versicap.h"

namespace vcp {

LayerRenderDetails* RenderContext::createLayerRenderDetails (const int layer,
                                                             const double sampleRate,
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
    const int64 noteFrames = static_cast<int64> (sampleRate * ((double) noteLength / 1000.0));
    const int64 tailFrames = static_cast<int64> (sampleRate * ((double) tailLength / 1000.0));
    
    while (key <= keyEnd)
    {
        auto* const renderLayer = details->frames.add (new RenderLayer());
        renderLayer->index = details->frames.size() - 1;
        
        String path = outputPath;
        
        if (! File::isAbsolutePath (path))
            path = Versicap::getSamplesDir().getFullPathName();

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

}