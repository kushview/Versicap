
#pragma once

#include "LookAndFeel.h"

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
        loop.setStart (0.25);
        loop.setEnd (0.75);
        range.setStart (0.0);
        range.setLength (newThumb->getTotalLength());
        secondsPerPixel = range.getLength() / static_cast<double> (getWidth());
        pixelsPerSecond = static_cast<double> (getWidth()) / range.getLength();
        startTimer (100);
        resized();
        repaint();
    }

    AudioThumbnail* getAudioThumbnail() { return thumb.get(); }

    void resized() override
    {
        range.setLength (secondsPerPixel * (double) getWidth());
    }

    void paint (Graphics& g) override
    {
        g.fillAll (LookAndFeel::widgetBackgroundColor.darker(.7));
        

        if (thumb)
        {
            auto wr = getLocalBounds().withHeight (jmin (getHeight(), 340));
            wr.setY ((getHeight() / 2) - (wr.getHeight() / 2));
            float step = (float)wr.getHeight() / ((float)thumb->getNumChannels());
            auto iter = wr.getY() + step - (step / 2.f);
            for (int c = 0; c < thumb->getNumChannels(); ++c)
            {
                g.setColour (LookAndFeel::widgetBackgroundColor.darker(.3));
                g.drawLine (0.f, iter, (float) getWidth(), iter, 1.f);
                iter += step;
            }

            g.setColour (Colours::orange.brighter (0.22));
            g.setOpacity (waveOpacity);
            thumb->drawChannels (g, wr, range.getStart(), range.getEnd(), verticalZoom);
        }

        g.setColour (Colours::red);
        g.setOpacity (waveOpacity + 0.12);
        g.fillRect (pixelsPerSecond * loop.getStart(), 0.f,
                    pixelsPerSecond * loop.getEnd(), 0.f);
        g.setOpacity (.82);
        g.drawLine (pixelsPerSecond * loop.getStart(), 0.f,
                    pixelsPerSecond * loop.getStart(), getHeight(), 1.2);
        g.drawLine (pixelsPerSecond * loop.getEnd(), 0.f,
                    pixelsPerSecond * loop.getEnd(), getHeight(), 1.2);
    }

    void setVerticalZoom (float zoom)
    {
        verticalZoom = jlimit (0.001f, 1.0f, zoom);
        repaint();
    }

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
