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
        channels.setBounds (r.removeFromRight (60));
        r.removeFromRight (1);
        device.setBounds (r);
    }

    ComboBox device;
    TextButton channels;
};

}
