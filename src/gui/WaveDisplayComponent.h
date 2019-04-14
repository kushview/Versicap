
#pragma once

#include "JuceHeader.h"

namespace vcp {

class WaveDisplayComponent : public Component
{
public:
    WaveDisplayComponent (AudioFormatManager& f, AudioThumbnailCache& c)
        : formats (f), cache (c)
    { }
    
    virtual ~WaveDisplayComponent() = default;

    void setAudioThumbnail (AudioThumbnail* newThumb)
    {
        thumb.reset (newThumb);
        resized();
        repaint();
    }

    AudioThumbnail& getAudioThumbnail() { jassert(thumb); return *thumb; }

private:
    AudioFormatManager& formats;
    AudioThumbnailCache& cache;
    std::unique_ptr<AudioThumbnail> thumb;
};

}
