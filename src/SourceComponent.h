
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
        sourceCombo.setSelectedId (1, dontSendNotification);

        addAndMakeVisible (midiDeviceLabel);
        midiDeviceLabel.setText ("Device", dontSendNotification);
        addAndMakeVisible (midiDeviceCombo);
        refreshMidiDevices();
        
        addAndMakeVisible (pluginLabel);
        pluginLabel.setText ("Plugin", dontSendNotification);
        addAndMakeVisible (pluginButton);
        pluginButton.onClick = std::bind (&SourceComponent::choosePlugin, this);
        pluginButton.setTriggeredOnMouseDown (true);

        addAndMakeVisible (bitDepthLabel);
        bitDepthLabel.setText ("Bit Depth", dontSendNotification);
        addAndMakeVisible (bitDepthCombo);
        bitDepthCombo.addItem ("16 bit", 1);
        bitDepthCombo.addItem ("24 bit", 2);
        bitDepthCombo.setSelectedId (1);

        addAndMakeVisible (recordingDeviceLabel);
        recordingDeviceLabel.setText ("Recorder", dontSendNotification);
        addAndMakeVisible (recordingDeviceCombo);
        recordingDeviceCombo.addItem ("Audio Device 1", 1);
        recordingDeviceCombo.addItem ("Audio Device 2", 2);
        recordingDeviceCombo.setSelectedId (1);

        addAndMakeVisible (renderButton);
        renderButton.setButtonText ("Render");
        renderButton.onClick = [this]() { startRender(); };

        stabilizeSettings();
        setSize (300, 300);
    }

    void stabilizeSettings() override
    {
        if (plugin.name.isEmpty())
            pluginButton.setButtonText ("Select a Plugin");
        else
            pluginButton.setButtonText (plugin.name);
          
        switch (sourceCombo.getSelectedId() - 1)
        {
            case SourceType::MidiDevice:
            {
                midiDeviceLabel.setVisible (true);
                midiDeviceCombo.setVisible (true);
                recordingDeviceLabel.setVisible (true);
                recordingDeviceCombo.setVisible (true);
                pluginLabel.setVisible (false);
                pluginButton.setVisible (false);
            } break;

            case SourceType::AudioPlugin:
            {
                midiDeviceLabel.setVisible (false);
                midiDeviceCombo.setVisible (false);
                recordingDeviceLabel.setVisible (false);
                recordingDeviceCombo.setVisible (false);
                pluginLabel.setVisible (true);
                pluginButton.setVisible (true);
            } break;
        }

        resized();
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8, 10);
        layout (r, sourceLabel, sourceCombo);
        if (midiDeviceLabel.isVisible())
            layout (r, midiDeviceLabel, midiDeviceCombo);
        if (pluginLabel.isVisible())
            layout (r, pluginLabel, pluginButton);
        
        if (recordingDeviceLabel.isVisible())
            layout (r, recordingDeviceLabel, recordingDeviceCombo);

        layout (r, bitDepthLabel, bitDepthCombo);

        renderButton.setBounds (r.removeFromTop (22).reduced (60, 0));
    }

private:
    PluginDescription plugin;

    Label sourceLabel;
    Label midiDeviceLabel;
    Label pluginLabel;
    Label bitDepthLabel;
    Label recordingDeviceLabel;

    ComboBox sourceCombo;
    ComboBox midiDeviceCombo;
    TextButton pluginButton;
    ComboBox bitDepthCombo;
    ComboBox recordingDeviceCombo;

    TextButton renderButton;

    void startRender();

    void refreshMidiDevices()
    {
        midiDeviceCombo.clear();
        for (int i = 0; i < 4; ++i)
            midiDeviceCombo.addItem ("Device " + String (i + 1), i + 1);
        midiDeviceCombo.setSelectedItemIndex (0);
    }

    void choosePlugin()
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::NoIcon, 
            "Choose a plugin", "Choose a plugin with the browser");
    }
};

