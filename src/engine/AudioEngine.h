
#pragma once

#include "Types.h"

namespace vcp {

class Render;
class RenderContext;

class AudioEngine
{
public:
    AudioEngine (AudioFormatManager& formatManager,
                 AudioPluginFormatManager& pluginManager);
    ~AudioEngine();

    //=========================================================================
    void setSourceType (SourceType type);
    void setEnabled (bool enabled) { shouldProcess.set (enabled ? 1 : 0); }

    AudioProcessor* getAudioProcessor() const { return processor.get(); }
    void setAudioProcessor (AudioProcessor* newProcessor);
    void clearAudioProcessor();
    
    //=========================================================================
    bool isRendering() const;
    void cancelRendering();
    Result startRendering (const RenderContext& ctx);

    //=========================================================================
    void prepare (double expectedSampleRate, int maxBufferSize,
                  int numInputs, int numOutputs);
    void prepare (AudioIODevice* device);
    void process (const float** input, int numInputs, 
                  float** output, int numOutputs, int nframes);
    void release();

private:
    //=========================================================================
    AudioFormatManager& formats;
    AudioPluginFormatManager& plugins;

    //=========================================================================
    bool prepared = false;
    Atomic<int> shouldProcess { 0 };
    Atomic<int> sourceType { SourceType::MidiDevice };

    //=========================================================================
    std::unique_ptr<AudioProcessor> processor;
    int pluginLatency = 0;
    int pluginChannels = 0;
    int pluginNumIns = 0;
    int pluginNumOuts = 0;

    //=========================================================================
    std::unique_ptr<Render> render;
    AudioSampleBuffer renderBuffer;

    //=========================================================================
    int inputLatency  = 0;
    int outputLatency = 0;
    int extraLatency  = 0;
    double sampleRate { 0.0 };
    int bufferSize = 0;
    int numInputChans = 0;
    int numOutputChans = 0;

    //=========================================================================
    HeapBlock<float*> channels;
    AudioSampleBuffer tempBuffer;
    AudioSampleBuffer pluginBuffer;
    
    //=========================================================================
    MidiBuffer incomingMidi;
    MidiBuffer renderMidi;
    MidiMessageCollector messageCollector;

    //=========================================================================
    void updatePluginProperties();
    void prepare (AudioProcessor& plugin);
    void release (AudioProcessor& plugin);
};

}
