
#pragma once

#include "JuceHeader.h"

class SourceComponent : public Component
{
public:
    SourceComponent()
    {
        addAndMakeVisible (recordingDeviceLabel);
        recordingDeviceLabel.setText ("Device", dontSendNotification);
        addAndMakeVisible (recordingDeviceCombo);
        refreshRecordingDevices();
        
        addAndMakeVisible (bitDepthLabel);
        bitDepthLabel.setText ("Bit Depth", dontSendNotification);
        addAndMakeVisible (bitDepthCombo);
        bitDepthCombo.addItem ("16 bit", 1);
        bitDepthCombo.addItem ("24 bit", 2);
    }

    void resized() override
    {
        auto r  = getLocalBounds();
        auto r2 = r.removeFromTop (22);
        recordingDeviceLabel.setBounds (r2.removeFromLeft (100));
        recordingDeviceCombo.setBounds (r2);
        r.removeFromTop (4);
        r2 = r.removeFromTop (22);
        bitDepthLabel.setBounds (r2.removeFromLeft (100));
        bitDepthCombo.setBounds (r2);
    }

private:
    Label recordingDeviceLabel;
    Label bitDepthLabel;
    ComboBox recordingDeviceCombo;
    ComboBox bitDepthCombo;

    void refreshRecordingDevices()
    {
        recordingDeviceCombo.clear();
        for (int i = 0; i < 4; ++i)
            recordingDeviceCombo.addItem ("Device " + String (i + 1), i + 1);
    }
};
