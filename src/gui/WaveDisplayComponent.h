
#pragma once

#include "JuceHeader.h"

namespace vcp {

class WaveDisplayComponent : public Component,
                             private Timer
{
public:
    WaveDisplayComponent() { }
    
    virtual ~WaveDisplayComponent() = default;

    void setAudioThumbnail (AudioThumbnail* newThumb)
    {
        thumb.reset (newThumb);
        repaint();
        startTimer (100);
    }

    AudioThumbnail* getAudioThumbnail() { return thumb.get(); }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
        if (thumb)
        {
            g.setColour (Colours::limegreen);
            thumb->drawChannels (g, getLocalBounds(), 0.0, 0.1, 0.95);
        }
    }

private:
    std::unique_ptr<AudioThumbnail> thumb;
    double startTime;
    double endTime;
    void timerCallback() override
    {
        if (! thumb)
            return stopTimer();
        if (thumb->isFullyLoaded())
            return stopTimer();
        repaint();
    }
};

}
