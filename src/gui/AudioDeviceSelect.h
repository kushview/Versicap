#pragma once

#include "JuceHeader.h"

namespace vcp {

class AudioDeviceSelect : public Component
{
public:
    AudioDeviceSelect()
    {
        addAndMakeVisible (device);
        addAndMakeVisible (channels);
    }

    ~AudioDeviceSelect() {}

    void resized() override
    {
        auto r = getLocalBounds();
        channels.setBounds (r.removeFromRight (channelButtonSize));
        r.removeFromRight (1);
        device.setBounds (r);
    }

    void setChannelButtonSize (int size)
    {
        channelButtonSize = jmax (30, size);
        resized();
    }
    
    ComboBox device;
    TextButton channels;

private:
    int channelButtonSize = 32;
};

}
