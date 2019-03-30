
#pragma once

#include "SettingGroup.h"

struct SourceType
{
    enum ID
    {
        MidiDevice  = 0,
        AudioPlugin = 1
    };
};

class SourceComponent : public SettingGroup
{
public:
    SourceComponent()
    {
        addAndMakeVisible (sourceLabel);
        sourceLabel.setText ("Source", dontSendNotification);
        addAndMakeVisible (sourceCombo);
        sourceCombo.addItem ("MIDI Device", 1 + SourceType::MidiDevice);
        sourceCombo.addItem ("Plugin", 1 + SourceType::AudioPlugin);
        sourceCombo.onChange = [this]() { stabilizeSettings(); };

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

    void stabilizeSettings() override
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

    void resized() override
    {
        auto r = getLocalBounds().reduced (8, 10);
        layout (r, sourceLabel, sourceCombo);
        layout (r, recordingDeviceLabel, recordingDeviceCombo);
        layout (r, bitDepthLabel, bitDepthCombo);
    }

private:
    Label sourceLabel;
    Label recordingDeviceLabel;
    Label bitDepthLabel;
    ComboBox sourceCombo;
    ComboBox recordingDeviceCombo;
    ComboBox bitDepthCombo;

    void refreshRecordingDevices()
    {
        recordingDeviceCombo.clear();
        for (int i = 0; i < 4; ++i)
            recordingDeviceCombo.addItem ("Device " + String (i + 1), i + 1);
    }
};

