
#pragma once

#include "exporters/Exporter.h"

namespace vcp {

class CreatePathTask : public ExportTask
{
public:
    CreatePathTask (const String& thePath)
        : path (thePath) { }
    CreatePathTask (const File& file)
        : path (file.getFullPathName()) { }

    Result prepare (Versicap&) override
    {
        if (path.isEmpty())
            return Result::fail ("export path not specified");

        if (! File::isAbsolutePath (path))
        {
            String message = "invalid path - "; message << path;
            return Result::fail (message);
        }

        const File directory (path);

        if (directory.existsAsFile())
            return Result::fail ("export path exists as file");
        if (! directory.exists())
            return directory.createDirectory();
        
        return Result::ok();
    }

    Result perform() override
    {
        if (! File::isAbsolutePath (path))
            return Result::fail ("invalid path for exporting");
        const File directory (path);
        return directory.exists() ? Result::ok() : Result::fail("Directory does not exist");
    }

private:
    const String path;
};

class AudioFileWriterTask : public ExportTask
{
public:
    AudioFileWriterTask (const File& src, const File& tgt,
                         double rate, int nchans, int depth,
                         int q,
                         double startSeconds, 
                         double endSeconds)
        : source (src),
          target (tgt),
          sampleRate (rate),
          channels (nchans),
          bitDepth (depth),
          quality (q),
          startTime (startSeconds),
          endTime (endSeconds)
    { }

    ~AudioFileWriterTask() { }

    Result prepare (Versicap&) override;
    Result perform() override;
    String getProgressName() const override { return target.getFileName(); }

private:
    const File source;
    const File target;
    std::unique_ptr<TemporaryFile> tempFile;
    const double sampleRate;
    const int channels;
    const int bitDepth;
    const int quality;
    const double startTime;
    const double endTime;

    std::unique_ptr<AudioFormatReader> reader;
    std::unique_ptr<AudioFormatWriter> writer;
};

}
