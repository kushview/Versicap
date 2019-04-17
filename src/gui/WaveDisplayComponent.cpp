#include "gui/WaveDisplayComponent.h"

namespace vcp {

WaveDisplayComponent::WaveDisplayComponent()
{
}

WaveDisplayComponent::~WaveDisplayComponent()
{
}

void WaveDisplayComponent::setAudioThumbnail (AudioThumbnail* newThumb)
{
    thumb.reset (newThumb);
    range.setStart (0.0);
    range.setLength (newThumb->getTotalLength());
    secondsPerPixel = range.getLength() / static_cast<double> (getWidth());
    pixelsPerSecond = static_cast<double> (getWidth()) / range.getLength();
    startTimer (100);
    resized();
    repaint();
}

AudioThumbnail* WaveDisplayComponent::getAudioThumbnail() { return thumb.get(); }

void WaveDisplayComponent::resized()
{
    range.setLength (secondsPerPixel * (double) getWidth());
}

void WaveDisplayComponent::paint (Graphics& g)
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
}

void WaveDisplayComponent::setVerticalZoom (float zoom)
{
    verticalZoom = jlimit (0.001f, 1.0f, zoom);
    repaint();
}

}
