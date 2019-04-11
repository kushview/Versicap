
#pragma once

#include "AudioDeviceSelect.h"
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
        pluginButton.onClose  = [this]()
        {
            versicap.closePlugin();
            pluginButton.setPluginName (String());
        };

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
        addAndMakeVisible (inputDevice);
        inputDevice.device.onChange = applyAudioDeviceSettingsCallback;
        inputDevice.channels.onClick = [this]()
        {
            auto* device = versicap.getDeviceManager().getCurrentAudioDevice();
            if (! device) return;
            PopupMenu menu, stereo, mono;
            auto chans = device->getInputChannelNames();
            
            for (int i = 0; i < chans.size(); ++i)
            {
                String name = String (i + 1);
                mono.addItem (i + 100, name);
            }

            for (int i = 0; i < chans.size(); i += 2)
            {
                String name = String (i + 1);
                name << " - " << int (i + 2);
                stereo.addItem (i + 200, name);
            }

            menu.addSubMenu ("Mono", mono);
            menu.addSubMenu ("Stereo", stereo);
            menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&inputDevice.channels),
                                ModalCallbackFunction::forComponent (EngineComponent::inputChannelChosen, this));
        };

        addAndMakeVisible (outputDeviceLabel);
        outputDeviceLabel.setText ("Audio Out", dontSendNotification);
        addAndMakeVisible (outputDevice);
        outputDevice.device.onChange = applyAudioDeviceSettingsCallback;
        outputDevice.channels.onClick = [this]()
        {
            auto* device = versicap.getDeviceManager().getCurrentAudioDevice();
            if (! device) return;
            PopupMenu menu, stereo, mono;
            auto chans = device->getOutputChannelNames();
            
            for (int i = 0; i < chans.size(); ++i)
            {
                String name = String (i + 1);
                mono.addItem (i + 100, name);
            }

            for (int i = 0; i < chans.size(); i += 2)
            {
                String name = String (i + 1);
                name << " - " << int (i + 2);
                stereo.addItem (i + 200, name);
            }

            menu.addSubMenu ("Mono", mono);
            menu.addSubMenu ("Stereo", stereo);
            menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&inputDevice.channels),
                                ModalCallbackFunction::forComponent (EngineComponent::outputChannelChosen, this));
        };

        addAndMakeVisible (latencyLabel);
        latencyLabel.setText ("Latency", dontSendNotification);
        addAndMakeVisible (latency);
        latency.setRange (0.0, 9999.0, 1.0);
        setupSlider (latency);
        latency.setTextBoxIsEditable (true);
        latency.textFromValueFunction = [](double value) -> String
        {
            String text = String (roundToInt (value));
            text << " (samples)";
            return text;
        };
        latency.updateText();

        setSize (300, 300);
    }

    int getSourceType() const { return sourceCombo.getSelectedId() - 1; }

    void fillSettings (RenderContext& ctx) override
    {
        ctx.source      = getSourceType();
        ctx.latency     = jlimit (0, 9999, roundToInt (latency.getValue()));
    }
    
    void updateSettings (const RenderContext& ctx) override
    {
        sourceCombo.setSelectedId (1 + ctx.source, dontSendNotification);
        latency.setValue (static_cast<double> (ctx.latency), dontSendNotification);
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
                // midiInputLabel.setVisible (true);
                // midiInputCombo.setVisible (true);
                midiOutputLabel.setVisible (true);
                midiOutputCombo.setVisible (true);
                pluginLabel.setVisible (false);
                pluginButton.setVisible (false);
            } break;

            case SourceType::AudioPlugin:
            {
                // midiInputLabel.setVisible (false);
                // midiInputCombo.setVisible (false);
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
        ensureCorrectChannels();

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
        
        layout (r, midiInputLabel, midiInputCombo, 0, 22, 4);
        if (SourceType::MidiDevice == sourceCombo.getSelectedId() - 1)
        {    
            layout (r, midiOutputLabel, midiOutputCombo);
        }
        else
        {
            layout (r, pluginLabel, pluginButton);
        }
        
        layout (r, inputDeviceLabel, inputDevice, 0, 22, 4);
        layout (r, outputDeviceLabel, outputDevice);
        layout (r, sampleRateLabel, sampleRateCombo, 0, 22, 4);
        layout (r, bufferSizeLabel, bufferSizeCombo);
        layout (r, latencyLabel, latency);
    }

    static void pluginChosen (int, EngineComponent*);
    static void inputChannelChosen (int, EngineComponent*);
    static void outputChannelChosen (int, EngineComponent*);

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
    Label latencyLabel;

    ComboBox sourceCombo;
    ComboBox midiInputCombo;
    ComboBox midiOutputCombo;
    PluginPicker pluginButton;
    ComboBox sampleRateCombo;
    ComboBox bufferSizeCombo;
    AudioDeviceSelect inputDevice;
    AudioDeviceSelect outputDevice;
    TextButton inputChannelButton;
    TextButton outputChannelButton;
    Slider latency;

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
    void ensureCorrectChannels();

    void choosePlugin();
    void pluginChosen (int);
    void inputChannelChosen (int);
    void outputChannelChosen (int);
};

}
