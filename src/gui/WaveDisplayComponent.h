
#pragma once

#include "JuceHeader.h"

namespace vcp {

class WaveDisplayComponent : public Component
{
public:
    WaveDisplayComponent() { }
    
    virtual ~WaveDisplayComponent() = default;

    void setAudioThumbnail (AudioThumbnail* newThumb)
    {
        thumb.reset (newThumb);
        resized();
        repaint();
    }

    AudioThumbnail* getAudioThumbnail() { return thumb.get(); }

    void paint (Graphics& g) override
    {
        if (thumb)
        {
            thumb->drawChannel (g, getLocalBounds(), 0.0, 4.0, 0, 1.f);
        }
    }

private:
    std::unique_ptr<AudioThumbnail> thumb;
};

}
