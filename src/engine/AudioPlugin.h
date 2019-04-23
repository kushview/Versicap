#pragma once

#include "JuceHeader.h"

namespace vcp {

class AudioPlugin : public ReferenceCountedObject
{
public:
    AudioPlugin (AudioProcessor* proc)
    {
        jassert (proc != nullptr);
        processor.reset (proc);
    }

    ~AudioPlugin()
    {
        processor.reset();
    }

    AudioProcessor* getAudioProcessor() const { return processor.get(); }

    void prepare (double sampleRate, int bufferSize)
    {
        processor->prepareToPlay (sampleRate, bufferSize);
    }

    void process (AudioSampleBuffer& audio, MidiBuffer& midi)
    {
        processor->processBlock (audio, midi);
    }

    void release()
    {
        processor->releaseResources();
    }

private:
    std::unique_ptr<AudioProcessor> processor;
};

typedef ReferenceCountedObjectPtr<AudioPlugin> AudioPluginPtr;

}
