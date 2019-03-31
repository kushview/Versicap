
#pragma once

#include "SettingGroup.h"

namespace vcp {

struct SourceType
{
    enum ID
    {
        MidiDevice  = 0,
        AudioPlugin = 1
    };
};

class EngineComponent : public SettingGroup
{
public:
    EngineComponent()
    {
        addAndMakeVisible (sourceLabel);
        sourceLabel.setText ("Source", dontSendNotification);
        addAndMakeVisible (sourceCombo);
        sourceCombo.addItem ("MIDI Device", 1 + SourceType::MidiDevice);
        sourceCombo.addItem ("Plugin", 1 + SourceType::AudioPlugin);
        sourceCombo.onChange = [this]() { stabilizeSettings(); };
        sourceCombo.setSelectedId (1, dontSendNotification);

        addAndMakeVisible (midiDeviceLabel);
        midiDeviceLabel.setText ("MIDI", dontSendNotification);
        addAndMakeVisible (midiDeviceCombo);
        refreshMidiDevices();
        
        addAndMakeVisible (pluginLabel);
        pluginLabel.setText ("Plugin", dontSendNotification);
        addAndMakeVisible (pluginButton);
        pluginButton.onClick = std::bind (&EngineComponent::choosePlugin, this);
        pluginButton.setTriggeredOnMouseDown (true);

        auto applyAudioDeviceSettingsCallback = std::bind (
            &EngineComponent::applyAudioDeviceSettings, this);

        addAndMakeVisible (sampleRateLabel);
        sampleRateLabel.setText ("Sample Rate", dontSendNotification);
        addAndMakeVisible (sampleRateCombo);
        sampleRateCombo.addItem ("44100", 44100);
        sampleRateCombo.addItem ("48000", 48000);
        sampleRateCombo.addItem ("96000", 96000);
        sampleRateCombo.addItem ("192000", 192000);
        sampleRateCombo.setSelectedId (44100, dontSendNotification);
        sampleRateCombo.onChange = applyAudioDeviceSettingsCallback;

        addAndMakeVisible (bufferSizeLabel);
        bufferSizeLabel.setText ("Buffer Size", dontSendNotification);
        addAndMakeVisible (bufferSizeCombo);
        bufferSizeCombo.addItem ("16", 16);
        bufferSizeCombo.addItem ("32", 32);
        bufferSizeCombo.addItem ("64", 64);
        bufferSizeCombo.addItem ("128", 128);
        bufferSizeCombo.addItem ("256", 256);
        bufferSizeCombo.addItem ("512", 512);
        bufferSizeCombo.addItem ("1024", 1024);
        bufferSizeCombo.addItem ("2048", 2048);
        bufferSizeCombo.addItem ("4096", 4096);
        bufferSizeCombo.setSelectedId (512, dontSendNotification);
        bufferSizeCombo.onChange = applyAudioDeviceSettingsCallback;

        addAndMakeVisible (inputDeviceLabel);
        inputDeviceLabel.setText ("Audio In", dontSendNotification);
        addAndMakeVisible (inputDeviceCombo);
        inputDeviceCombo.onChange = applyAudioDeviceSettingsCallback;

        addAndMakeVisible (outputDeviceLabel);
        outputDeviceLabel.setText ("Audio Out", dontSendNotification);
        addAndMakeVisible (outputDeviceCombo);
        outputDeviceCombo.onChange = applyAudioDeviceSettingsCallback;
        
        addAndMakeVisible (renderButton);
        renderButton.setButtonText ("Render");
        renderButton.onClick = [this]() { startRender(); };

        refresh();

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
                pluginLabel.setVisible (false);
                pluginButton.setVisible (false);
            } break;

            case SourceType::AudioPlugin:
            {
                midiDeviceLabel.setVisible (false);
                midiDeviceCombo.setVisible (false);
                pluginLabel.setVisible (true);
                pluginButton.setVisible (true);
            } break;
        }

        resized();
    }

    void refresh() override
    {
        refreshMidiDevices();
        refreshAudioDevices();
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8, 10);
        layout (r, sourceLabel, sourceCombo);
        if (midiDeviceLabel.isVisible())
            layout (r, midiDeviceLabel, midiDeviceCombo);
        if (pluginLabel.isVisible())
            layout (r, pluginLabel, pluginButton);
        
        layout (r, inputDeviceLabel, inputDeviceCombo, 0, 22, 4);
        layout (r, outputDeviceLabel, outputDeviceCombo, 0, 22, 4);
        layout (r, sampleRateLabel, sampleRateCombo, 0, 22, 4);
        layout (r, bufferSizeLabel, bufferSizeCombo);

        renderButton.setBounds (r.removeFromTop (22).reduced (60, 0));
    }

private:
    PluginDescription plugin;
    StringArray midiOutputs;

    Label sourceLabel;
    Label midiDeviceLabel;
    Label pluginLabel;
    Label sampleRateLabel;
    Label bufferSizeLabel;
    Label inputDeviceLabel;
    Label outputDeviceLabel;

    ComboBox sourceCombo;
    ComboBox midiDeviceCombo;
    TextButton pluginButton;
    ComboBox sampleRateCombo;
    ComboBox bufferSizeCombo;
    ComboBox inputDeviceCombo;
    ComboBox outputDeviceCombo;

    TextButton renderButton;

    void startRender();

    void refreshMidiDevices()
    {
        midiOutputs = MidiOutput::getDevices();
        String current = midiDeviceCombo.getItemText (midiDeviceCombo.getSelectedItemIndex());
        midiDeviceCombo.clear();
        int i = 1;
        for (const auto& device : midiOutputs)
            midiDeviceCombo.addItem (device, i++);
        if (current.isNotEmpty())
            midiDeviceCombo.setSelectedItemIndex (jmax (0, midiOutputs.indexOf (current)));
        else
            midiDeviceCombo.setSelectedItemIndex (0);
    }

    void refreshAudioDevices();
    void applyAudioDeviceSettings();

    void choosePlugin()
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::NoIcon, 
            "Choose a plugin", "Choose a plugin with the browser");
    }
};

}
