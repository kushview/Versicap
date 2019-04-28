
#include "exporters/ExportTasks.h"
#include "Project.h"
#include "Versicap.h"

namespace vcp {

//=============================================================================
void AudioFileWriterTask::prepare (Versicap& versicap)
{
    auto& formats = versicap.getAudioFormats();
    reader.reset (formats.createReaderFor (source));
    if (! reader)
        return;
    
    std::unique_ptr<FileOutputStream> stream;

    if (auto* const format = formats.findFormatForFileExtension (target.getFileExtension()))
    {
        tempFile.reset (new TemporaryFile (target));
        stream.reset (tempFile->getFile().createOutputStream());
        if (stream)
        {
            writer.reset (format->createWriterFor (stream.get(),
                sampleRate,
                static_cast<unsigned int> (reader->numChannels),
                bitDepth, { },
                quality)
            );
        }
        else
        {
            tempFile->deleteTemporaryFile();
        }
    }

    if (writer != nullptr)
        stream.release();
}

Result AudioFileWriterTask::perform()
{
    if (! reader)
        return Result::fail ("Could not create sample reader for exporting");
    if (! writer)
        return Result::fail ("Could not create sample writer for exporting");

    AudioFormatReaderSource afrs (reader.get(), false);
    const int64 startFrame = roundToIntAccurate (jmax (0.0, startTime) * reader->sampleRate);
    const int64 endFrame = roundToIntAccurate (endTime > startTime 
        ? endTime * reader->sampleRate : startTime * reader->sampleRate);

    const int blockSize = 2048;
    afrs.setNextReadPosition (startFrame);
    ResamplingAudioSource resample (&afrs, false, reader->numChannels);
    resample.prepareToPlay (blockSize, reader->sampleRate);
    resample.setResamplingRatio (reader->sampleRate / writer->getSampleRate());
    auto ratio = resample.getResamplingRatio();
    int totalSamples = endFrame - startFrame;
    jassert (totalSamples > 0);
    totalSamples = roundToIntAccurate ((double) totalSamples / ratio);
    jassert (totalSamples > 0);
    if (writer->writeFromAudioSource (resample, totalSamples, blockSize))
    {
        writer.reset();
        return tempFile->overwriteTargetFileWithTemporary()
            ? Result::ok() : Result::fail ("could not write sample");
    }

    return Result::fail ("Could not write process sample");
}

void Project::getExportTasks (OwnedArray<ExportTask>& tasks) const
{
    for (int i = 0; i < getNumExporters(); ++i)
    {
        auto exporter = getExporter (i);
        auto type = exporter.getTypeObject();
        jassert (exporter.isValid());
        jassert (type);
        if (type == nullptr)
            continue;

        tasks.add (new CreatePathTask (exporter.getPath()));
        type->getTasks (*this, exporter, tasks);
    }
}

}
