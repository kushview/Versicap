
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

    void setRange (Range<double> range);
    void setStartTime (double);
    void setEndTime (double);

    double getStartTime() const             { return range.getStart(); }
    double getEndTime() const               { return range.getEnd(); }
    double getViewedTimespan() const        { return range.getLength(); }

    void setSecondsPerPixel (double);
    void setPixelsPerSecond (double);

    double getPixelsPerSecond() const       { return pixelsPerSecond; }
    double getSecondsPerPixel() const       { return secondsPerPixel; }

    void resized() override;
    void paint (Graphics& g) override;

private:
    std::unique_ptr<AudioThumbnail> thumb;
    float verticalZoom      = 1.f;
    float waveOpacity       = 0.6f;
    double secondsPerPixel  = 0.0;
    double pixelsPerSecond  = 0.0;
    
    Range<double> range;
    
    void timerCallback() override
    {
        repaint();
        if (! thumb || thumb->isFullyLoaded())
            stopTimer();  
    }
};

}
