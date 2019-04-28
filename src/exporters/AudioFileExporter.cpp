
#include "exporters/Exporter.h"
#include "exporters/ExportTasks.h"
#include "Project.h"

namespace vcp {

class AudioFileExporterType : public ExporterType
{
public:
    AudioFileExporterType (AudioFormatManager& fmts, const String& ext)
        : formats (fmts), extension (ext)
    {
        jassert (extension.isNotEmpty());
        jassert (getFormat() != nullptr);
    }

    virtual ~AudioFileExporterType() = default;

    String getSlug() const override
    {
        if (extension.contains ("wav"))
            return "wave";
        else if (extension.contains ("aif"))
            return "aiff";
        else if (extension.contains ("ogg"))
            return "ogg";
        else if (extension.contains ("flac"))
            return "flac";

        jassertfalse;
        return "undefined";
    }

    String getName() const override
    {
        if (extension.contains ("wav"))
            return "Wav Audio";
        else if (extension.contains ("aif"))
            return "AIFF";
        else if (extension.contains ("ogg"))
            return "Ogg Vorbis";
        else if (extension.contains ("flac"))
            return "FLAC";

        jassertfalse;
        return "Undefined Format";
    }

    String getFileExtension() const
    {
        if (extension.startsWith ("."))
            return extension;
        return String(".") + extension;
    }

    AudioFormat* getFormat() const { return formats.findFormatForFileExtension (extension); }
    
    void getProperties (const Exporter& exporter, Array<PropertyComponent*>& props) const override
    {
        Exporter expref = exporter;
        auto* const format = getFormat();
        StringArray choices; Array<var> values;

        //=====================================================================
        choices.clearQuick(); values.clearQuick();
        if (format->canDoMono())
        {
            choices.add ("Mono");
            values.add (1);
        }
        if (format->canDoStereo())
        {
            choices.add ("Stereo");
            values.add (2);
        }
        props.add (new ChoicePropertyComponent (expref.getPropertyAsValue (Tags::channels),
            "Channels", choices, values));

        //=====================================================================
        choices.clearQuick(); values.clearQuick();
        const auto qualityOpts = format->getQualityOptions();
        if (qualityOpts.size() > 0)
        {
            for (int i = 0; i < qualityOpts.size(); ++i)
            {
                choices.add (qualityOpts [i]);
                values.add (i);
            }
        }
        else
        {
            choices.add ("Lossless");
            values.add (0);
        }
        props.add (new ChoicePropertyComponent (expref.getPropertyAsValue (Tags::quality),
            "Quality", choices, values));

        //=====================================================================
        choices.clearQuick(); values.clearQuick();
        for (const auto& rate : format->getPossibleSampleRates())
        {
            choices.add (String (rate));
            values.add (rate);
        }
        props.add (new ChoicePropertyComponent (expref.getPropertyAsValue (Tags::sampleRate),
            "Sample Rate", choices, values));
        
        //=====================================================================
        choices.clearQuick(); values.clearQuick();
        for (const auto& depth : format->getPossibleBitDepths())
        {
            choices.add (String (depth));
            values.add (depth);
        }
        props.add (new ChoicePropertyComponent (expref.getPropertyAsValue (Tags::bitDepth),
            "Bit Depth", choices, values));        
    }

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
                filename << getFileExtension();

                tasks.add (new AudioFileWriterTask (
                    sample->getFile(),
                    exporter.getPath().getChildFile (filename),
                    exporter.getProperty (Tags::sampleRate, 44100.0),
                    exporter.getProperty (Tags::channels, 2),
                    exporter.getProperty (Tags::bitDepth, 16),
                    exporter.getProperty (Tags::quality, 0),
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

private:
    AudioFormatManager& formats;
    const String extension;
};


ExporterType* ExporterType::createWavExporterType (AudioFormatManager& formats)
{
    return new AudioFileExporterType (formats, ".wav");
}

ExporterType* ExporterType::createAiffExporterType (AudioFormatManager& formats)
{ 
    return new AudioFileExporterType (formats, ".aiff");
}

ExporterType* ExporterType::createFlacExporterType (AudioFormatManager& formats)
{ 
    return new AudioFileExporterType (formats, ".flac");
}

ExporterType* ExporterType::createOggExporterType (AudioFormatManager& formats)
{
    return new AudioFileExporterType (formats, ".ogg");
}

}
