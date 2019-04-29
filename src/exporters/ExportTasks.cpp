
#include "exporters/ExportTasks.h"
#include "Project.h"
#include "Versicap.h"

namespace vcp {

//=============================================================================
Result AudioFileWriterTask::prepare (Versicap& versicap)
{
    if (source == File())
        return Result::fail ("source sample not specified");
    if (target == File())
        return Result::fail ("target not specified for export");

    auto& formats = versicap.getAudioFormats();
    reader.reset (formats.createReaderFor (source));
    String message;
    
    if (! reader)
    {
        message = "cannot create decoder for ";
        message << source.getFileName();
        return Result::fail (message);
    }

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
            message = "could not create/open target file: "; 
            message << target.getFileName();
            return Result::fail (message);
        }
    }
    else
    {
        message = "cannot find encoder for target "; 
        message << target.getFileName();
        return Result::fail (message);
    }

    if (writer != nullptr)
    {
        stream.release();
        return Result::ok();
    }

    message = "could not create encoder for ";
    message << target.getFileName();
    return Result::fail ("unknown error in when preparing for export");
}

Result AudioFileWriterTask::perform()
{
    if (! reader)
        return Result::fail ("cannot read sample without a decoder");
    if (! writer)
        return Result::fail ("cannot write sample without an encoder");

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

    return Result::fail ("could not write process sample");
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
