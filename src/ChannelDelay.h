#pragma once

#include "JuceHeader.h"

namespace vcp {

//=============================================================================
class ChannelDelay
{
public:
    ChannelDelay() = default;
    ~ChannelDelay() = default;

    int getNumSamplesDelay() const { return delaySize; }

    void resize (int numChannels, int numSamplesDelay)
    {
        totalChannels   = numChannels;
        delaySize       = numSamplesDelay;
        bufferSize      = delaySize + 1;
        readIndex       = 0;
        writeIndex      = delaySize;
        buffer.setSize (totalChannels, bufferSize, false, false, true);
    }

    void clear()
    {
        readIndex = 0;
        writeIndex = delaySize;
        buffer.clear (0, bufferSize);
    }

    void process (AudioSampleBuffer& audio)
    {
        float* data = nullptr;
        float* buf  = nullptr;
        int frame   = 0;
        const int nframes = audio.getNumSamples();

        for (int channel = 0; channel < totalChannels; ++channel)
        {
            data = audio.getWritePointer (channel, 0);
            buf  = buffer.getWritePointer (channel, 0);
            for (frame = nframes; --frame >= 0;)
            {
                buf [writeIndex] = *data;
                *data++ = buf [readIndex];

                if (++readIndex >= bufferSize)
                    readIndex = 0;
                if (++writeIndex >= bufferSize)
                    writeIndex = 0;
            }
        }
    }

private:
    AudioSampleBuffer buffer;
    int delaySize    = 0;
    int totalChannels     = 0;
    int bufferSize      = 0;
    int readIndex       = 0;
    int writeIndex      = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelDelay)
};

}
