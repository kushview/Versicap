
#pragma once

#include "RenderContext.h"
#include "Types.h"

namespace vcp {

class PluginManager;
class Render;

class AudioEngine : public AudioIODeviceCallback,
                    public MidiInputCallback
{
public:
    AudioEngine();
    ~AudioEngine();

    //=========================================================================
    void audioDeviceIOCallback (const float** input, int numInputs, 
                                float** output, int numOutputs,
                                int nframes) override;
    void audioDeviceAboutToStart (AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError (const String& errorMessage) override;

    //=========================================================================
    void handleIncomingMidiMessage (MidiInput*, const MidiMessage& message) override;
    void handlePartialSysexMessage (MidiInput* source, const uint8* messageData,
                                    int numBytesSoFar, double timestamp) override;

private:
    //=========================================================================
    AudioDeviceManager* devices = nullptr;
    PluginManager* plugins = nullptr;

    //=========================================================================
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
    RenderContext context;
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
