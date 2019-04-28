
#include "exporters/Exporter.h"
#include "exporters/ExportTasks.h"
#include "Project.h"

namespace vcp {

class WavExporterType : public ExporterType
{
public:
    WavExporterType()
    {

    }

    ~WavExporterType()
    {

    }

    String getSlug() const override { return "wave"; }
    String getName() const override { return "WAVE Exporter"; }
    
    void getTasks (const Project& project, const Exporter& exporter,
                   OwnedArray<ExportTask>& tasks) const override
    {
        OwnedArray<Sample> samples;
        for (int layerIdx = 0; layerIdx < project.getNumLayers(); ++layerIdx)
        {
            const auto layer = project.getLayer (layerIdx);
            layer.getSamples (samples);
            
            for (auto* const sample : samples)
            {
                String filename = sample->getFileName();
                filename << ".wav";

                DBG(sample->getValueTree().toXmlString());
                tasks.add (new AudioFileWriterTask (
                    sample->getFile(),
                    exporter.getPath().getChildFile (filename),
                    exporter.getProperty (Tags::sampleRate, 44100.0),
                    exporter.getProperty (Tags::channels, 2),
                    exporter.getProperty (Tags::bitDepth, 16),
                    0,
                    sample->getStartTime(), 
                    sample->getEndTime()));
            }

            samples.clearQuick (true);
        }
    }

    void getLoopTypes (Array<LoopType>& types) const override
    {
        types.add (LoopType::None);
    }
};

ExporterType* ExporterType::createWavExporterType() { return new WavExporterType(); }

}
