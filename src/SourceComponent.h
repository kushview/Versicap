
#pragma once

#include "JuceHeader.h"

struct SourceType
{
    enum ID {
        MidiDevice  = 0,
        AudioPlugin = 1
    };
};

class SourceComponent : public Component
{
public:
    SourceComponent()
    {
        addAndMakeVisible (sourceLabel);
        sourceLabel.setText ("Source", dontSendNotification);
        addAndMakeVisible (sourceCombo);
        sourceCombo.addItem ("MIDI Device", 1 + SourceType::MidiDevice);
        sourceCombo.addItem ("Plugin", 1 + SourceType::AudioPlugin);
        sourceCombo.onChange = [this]() {
            updateLayout();
        };

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
        sourceLabel.setBounds (r2.removeFromLeft (100));
        sourceCombo.setBounds (r2);
        r.removeFromTop (4);
        r2 = r.removeFromTop (22);
        recordingDeviceLabel.setBounds (r2.removeFromLeft (100));
        recordingDeviceCombo.setBounds (r2);
        r.removeFromTop (4);
        r2 = r.removeFromTop (22);
        bitDepthLabel.setBounds (r2.removeFromLeft (100));
        bitDepthCombo.setBounds (r2);
    }

private:
    Label sourceLabel;
    Label recordingDeviceLabel;
    Label bitDepthLabel;
    ComboBox sourceCombo;
    ComboBox recordingDeviceCombo;
    ComboBox bitDepthCombo;

    void updateLayout()
    {
        switch (sourceCombo.getSelectedId() - 1)
        {
            case SourceType::MidiDevice:
            {

            } break;

            case SourceType::AudioPlugin:
            {

            } break;
        }

    }

    void refreshRecordingDevices()
    {
        recordingDeviceCombo.clear();
        for (int i = 0; i < 4; ++i)
            recordingDeviceCombo.addItem ("Device " + String (i + 1), i + 1);
    }
};

