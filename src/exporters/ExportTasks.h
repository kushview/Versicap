
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

    void prepare (Versicap&) override
    {
        DBG("[VCP] creating: " << path);
        if (! File::isAbsolutePath (path))
            return;
        const File directory (path);
        if (! directory.exists())
            directory.createDirectory();
    }
    
    Result perform() override
    {
        if (! File::isAbsolutePath (path))
            return Result::fail ("Invalid path for exporting");
        const File directory (path);
        if (directory.existsAsFile())
            return Result::fail ("Export path exists as file");
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

    void prepare (Versicap&) override;
    Result perform() override;

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
