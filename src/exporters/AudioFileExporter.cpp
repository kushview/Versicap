
#include "exporters/Exporter.h"
#include "exporters/ExportTasks.h"
#include "Project.h"

namespace vcp {

class AudioFileExporterType : public ExporterType
{
public:
    AudioFileExporterType (AudioFormatManager& fmts, FormatType t)
        : formats (fmts), type (t)
    {
        jassert (getFormat() != nullptr);
    }

    virtual ~AudioFileExporterType() = default;

    String getSlug() const override { return type.getSlug(); }

    String getName() const override
    {
        switch (type.getType())
        {
            case FormatType::WAVE: return "Wav Audio"; break;
            case FormatType::AIFF: return "AIFF"; break;
            case FormatType::FLAC: return "FLAC"; break;
            case FormatType::OGG:  return "Ogg Vorbis"; break;
        }

        jassertfalse;
        return "Undefined Format";
    }

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

            props.add (new ChoicePropertyComponent (expref.getPropertyAsValue (Tags::quality),
                "Quality", choices, values));
        }

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
        for (int layerIdx = 0; layerIdx < project.getNumSampleSets(); ++layerIdx)
        {
            const auto layer = project.getSampleSet (layerIdx);
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

    void setMissingProperties (ValueTree data) const override
    {
        if (! data.hasProperty (Tags::sampleRate))
            data.setProperty (Tags::sampleRate, 44100, nullptr);
        if (! data.hasProperty (Tags::channels))
            data.setProperty (Tags::channels, 2, nullptr);
        if (! data.hasProperty (Tags::quality))
            data.setProperty (Tags::quality, getDefaultQuality(), nullptr);
        if (! data.hasProperty (Tags::bitDepth))
            data.setProperty (Tags::bitDepth, getDefaultBitDepth(), nullptr);
    }

    String getFileExtension() const
    {
        String extension = type.getFileExtension();
        if (extension.startsWith ("."))
            return extension;
        return String(".") + extension;
    }

    AudioFormat* getFormat() const
    {
        return formats.findFormatForFileExtension (
            getFileExtension());
    }

private:
    AudioFormatManager& formats;
    FormatType type;

    int getDefaultQuality() const
    {
        int quality = 0;
        switch (type.getType())
        {
            case FormatType::FLAC:
                quality = 5;
                break;
            case FormatType::OGG:
                quality = 6;
                break;
            case FormatType::WAVE:
            case FormatType::AIFF:
            default:
                quality = 0;
                break;
        }

        jassert (quality >= 0);
        return quality;
    }

    int getDefaultBitDepth() const
    {
        int depth = 0;
        switch (type.getType())
        {
            case FormatType::OGG:
                depth = 32;
                break;
            case FormatType::FLAC:
            case FormatType::WAVE:
            case FormatType::AIFF:
            default:
                depth = 16;
                break;
        }

        jassert (depth > 0);
        return depth;
    }
};

ExporterType* ExporterType::createWavExporterType (AudioFormatManager& formats)
{
    return new AudioFileExporterType (formats, FormatType::WAVE);
}

ExporterType* ExporterType::createAiffExporterType (AudioFormatManager& formats)
{ 
    return new AudioFileExporterType (formats, FormatType::AIFF);
}

ExporterType* ExporterType::createFlacExporterType (AudioFormatManager& formats)
{ 
    return new AudioFileExporterType (formats, FormatType::FLAC);
}

ExporterType* ExporterType::createOggExporterType (AudioFormatManager& formats)
{
    return new AudioFileExporterType (formats, FormatType::OGG);
}

}
