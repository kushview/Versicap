
#pragma once

#include "LookAndFeel.h"

namespace vcp {

class WaveDisplayComponent : public Component,
                             private Timer
{
public:
    WaveDisplayComponent();
    virtual ~WaveDisplayComponent();

    void setAudioThumbnail (AudioThumbnail* newThumb);
    AudioThumbnail* getAudioThumbnail();
    void setVerticalZoom (float zoom);

    void resized() override;
    void paint (Graphics& g) override;

private:
    std::unique_ptr<AudioThumbnail> thumb;
    float verticalZoom = 0.95;
    float waveOpacity = 0.6f;
    double secondsPerPixel = 0.0;
    double pixelsPerSecond = 0.0;
    
    Range<double> range;
    Range<double> loop;

    void timerCallback() override
    {
        repaint();
        if (! thumb || thumb->isFullyLoaded())
            stopTimer();  
    }
};

}
