
#pragma once

#include "ProjectWatcher.h"
#include "Types.h"

namespace KSP1 {
class SampleCache;
class SamplerSynth;
}

namespace vcp {

class Render;
class RenderContext;

class AudioEngine
{
public:
    AudioEngine (AudioFormatManager& formatManager,
                 AudioPluginFormatManager& pluginManager,
                 KSP1::SampleCache&);
    ~AudioEngine();

    struct Monitor : public ReferenceCountedObject
    {
        Atomic<float> levelLeft;
        Atomic<float> levelRight;
    };

    typedef ReferenceCountedObjectPtr<Monitor> MonitorPtr;

    //=========================================================================
    void setProject (const Project& project);

    //=========================================================================
    MonitorPtr getMonitor() const { return monitor; }

    //=========================================================================
    void panic();

    //=========================================================================
    void setEnabled (bool enabled) { shouldProcess.set (enabled ? 1 : 0); }

    //=========================================================================
    AudioProcessor* getAudioProcessor() const { return processor.get(); }
    void setAudioProcessor (AudioProcessor* newProcessor);
    void clearAudioProcessor();
    
    //=========================================================================
    bool isRendering() const;
    void cancelRendering();
    void setRenderContext (const RenderContext&);
    Result startRendering (const RenderContext& ctx);
    ValueTree getRenderedSamples() const;
    
    //=========================================================================
    void addMidiMessage (const MidiMessage& msg);
    void setDefaultMidiOutput (const String& name);
    String getDefaultMidiOutputName() const { return midiOutName; }

    //=========================================================================
    void prepare (double expectedSampleRate, int maxBufferSize,
                  int numInputs, int numOutputs);
    void prepare (AudioIODevice* device);
    void process (const float** input, int numInputs, 
                  float** output, int numOutputs, int nframes);
    void release();

    //=========================================================================
    std::function<void()> onRenderStopped;
    std::function<void()> onRenderStarted;
    std::function<void()> onRenderCancelled;
    std::function<void(double, const String&)> onRenderProgress;

private:
    MonitorPtr monitor;

    //=========================================================================
    AudioFormatManager& formats;
    AudioPluginFormatManager& plugins;
    KSP1::SampleCache& sampleCache;
    
    //=========================================================================
    std::unique_ptr<KSP1::SamplerSynth> sampler;

    //=========================================================================
    bool prepared = false;
    Atomic<int> shouldProcess { 0 };
    Atomic<int> shouldPanic { 0 };
    
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
    std::unique_ptr<MidiOutput> midiOut;
    String midiOutName;

    //=========================================================================
    MidiBuffer incomingMidi;
    MidiBuffer pluginMidi;
    MidiBuffer renderMidi;

    MidiBuffer  samplerMidi;
    AudioSampleBuffer samplerAudio;

    MidiMessageCollector messageCollector;

    //=========================================================================
    ProjectWatcher watcher;

    //=========================================================================
    void updatePluginProperties();
    void prepare (AudioProcessor& plugin);
    void release (AudioProcessor& plugin);

    void addPanicMessages (MidiBuffer&);

    void onProjectLoaded();
    void onActiveSampleChanged();
};

}
