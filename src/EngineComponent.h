
#pragma once

#include "PluginPicker.h"
#include "SettingGroup.h"
#include "Types.h"

namespace vcp {

class EngineComponent : public SettingGroup
{
public:
    EngineComponent (Versicap& vc)
        : SettingGroup (vc)
    {
        addAndMakeVisible (sourceLabel);
        sourceLabel.setText ("Source", dontSendNotification);
        addAndMakeVisible (sourceCombo);
        sourceCombo.addItem ("MIDI Device", 1 + SourceType::MidiDevice);
        sourceCombo.addItem ("Plugin", 1 + SourceType::AudioPlugin);
        sourceCombo.onChange = [this]() { stabilizeSettings(); };
        sourceCombo.setSelectedId (1, dontSendNotification);

        addAndMakeVisible (midiInputLabel);
        midiInputLabel.setText ("MIDI In", dontSendNotification);
        addAndMakeVisible (midiInputCombo);
        midiInputCombo.onChange = [this]() { applyMidiInput(); };

        addAndMakeVisible (midiOutputLabel);
        midiOutputLabel.setText ("MIDI Out", dontSendNotification);
        addAndMakeVisible (midiOutputCombo);
        midiOutputCombo.onChange = [this]() { applyMidiOutput(); };

        addAndMakeVisible (pluginLabel);
        pluginLabel.setText ("Plugin", dontSendNotification);
        addAndMakeVisible (pluginButton);
        pluginButton.onChoose = std::bind (&EngineComponent::choosePlugin, this);
        pluginButton.onEditor = std::bind (&Versicap::showPluginWindow, &versicap);
        pluginButton.onClose  = std::bind (&Versicap::closePlugin, &versicap);

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

        setSize (300, 300);
    }

    int getSourceType() const { return sourceCombo.getSelectedId() - 1; }

    void fillSettings (RenderContext& ctx) override 
    {
        ctx.source = getSourceType();
    }
    
    void updateSettings (const RenderContext& ctx) override
    {
        sourceCombo.setSelectedId (1 + ctx.source, dontSendNotification);
    }

    void updatePluginButton()
    {
        if (plugin.name.isEmpty())
            pluginButton.setPluginName (String());
        else
            pluginButton.setPluginName (plugin.name);
    }

    void stabilizeSettings() override
    {
        updatePluginButton();
        switch (getSourceType())
        {
            case SourceType::MidiDevice:
            {
                midiInputLabel.setVisible (true);
                midiInputCombo.setVisible (true);
                midiOutputLabel.setVisible (true);
                midiOutputCombo.setVisible (true);
                pluginLabel.setVisible (false);
                pluginButton.setVisible (false);
            } break;

            case SourceType::AudioPlugin:
            {
                midiInputLabel.setVisible (false);
                midiInputCombo.setVisible (false);
                midiOutputLabel.setVisible (false);
                midiOutputCombo.setVisible (false);
                pluginLabel.setVisible (true);
                pluginButton.setVisible (true);
            } break;
        }

        ensureCorrectAudioInput();
        ensureCorrectAudioOutput();
        ensureCorrectMidiInput();
        ensureCorrectMidiOutput();
        ensureTimings();

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
        
        if (SourceType::MidiDevice == sourceCombo.getSelectedId() - 1)
        {
            layout (r, midiInputLabel, midiInputCombo, 0, 22, 4);
            layout (r, midiOutputLabel, midiOutputCombo);
        }
        else
        {
            layout (r, pluginLabel, pluginButton);
        }            
        
        layout (r, inputDeviceLabel, inputDeviceCombo, 0, 22, 4);
        layout (r, outputDeviceLabel, outputDeviceCombo, 0, 22, 4);
        layout (r, sampleRateLabel, sampleRateCombo, 0, 22, 4);
        layout (r, bufferSizeLabel, bufferSizeCombo);
    }

    static void pluginChosen (int, EngineComponent*);

private:
    PluginDescription plugin;
    StringArray midiInputs, midiOutputs;

    Label sourceLabel;
    Label midiInputLabel;
    Label midiOutputLabel;
    Label pluginLabel;
    Label sampleRateLabel;
    Label bufferSizeLabel;
    Label inputDeviceLabel;
    Label outputDeviceLabel;

    ComboBox sourceCombo;
    ComboBox midiInputCombo;
    ComboBox midiOutputCombo;
    PluginPicker pluginButton;
    ComboBox sampleRateCombo;
    ComboBox bufferSizeCombo;
    ComboBox inputDeviceCombo;
    ComboBox outputDeviceCombo;

    void refreshAudioDevices();
    void refreshMidiDevices();
    
    void applyAudioDeviceSettings();
    void applyMidiInput();
    void applyMidiOutput();

    void ensureCorrectAudioInput();
    void ensureCorrectAudioOutput();
    void ensureCorrectMidiInput();
    void ensureCorrectMidiOutput();
    void ensureTimings();

    void choosePlugin();
    void pluginChosen (int);
};

}
