
#pragma once

#include "gui/AudioDeviceSelect.h"
#include "gui/PluginPicker.h"
#include "gui/SettingGroup.h"
#include "Types.h"

namespace vcp {

class EngineContentView : public SettingGroup
{
public:
    EngineContentView (Versicap& vc)
        : SettingGroup (vc)
    {
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
        pluginButton.onChoose = std::bind (&EngineContentView::choosePlugin, this);
        pluginButton.onEditor = std::bind (&Versicap::showPluginWindow, &versicap);
        pluginButton.onClose  = [this]()
        {
            versicap.closePlugin();
            pluginButton.setPluginName (String());
        };

        auto applyAudioDeviceSettingsCallback = std::bind (
            &EngineContentView::applyAudioDeviceSettings, this);

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
                name << "-" << int (i + 2);
                stereo.addItem (i + 200, name);
            }

            menu.addSubMenu ("Mono", mono);
            menu.addSubMenu ("Stereo", stereo);
            menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&inputDevice.channels),
                                ModalCallbackFunction::forComponent (EngineContentView::inputChannelChosen, this));
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
                name << "-" << int (i + 2);
                stereo.addItem (i + 200, name);
            }

            menu.addSubMenu ("Mono", mono);
            menu.addSubMenu ("Stereo", stereo);
            menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&inputDevice.channels),
                                ModalCallbackFunction::forComponent (EngineContentView::outputChannelChosen, this));
        };

        setSize (300, 300);
    }
    
    void updateSettings() override;

    void updatePluginButton()
    {
        versicap.getProject().getPluginDescription (
            versicap.getPluginManager(), plugin);
        if (plugin.name.isEmpty())
            pluginButton.setPluginName (String());
        else
            pluginButton.setPluginName (plugin.name);
    }

    void stabilizeSettings() override
    {
        updatePluginButton();
        
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
        
        layout (r, midiInputLabel, midiInputCombo, 0, 22, 4);
        layout (r, midiOutputLabel, midiOutputCombo);
        
        layout (r, pluginLabel, pluginButton);
        
        layout (r, inputDeviceLabel, inputDevice, 0, 22, 4);
        layout (r, outputDeviceLabel, outputDevice);
        layout (r, sampleRateLabel, sampleRateCombo, 0, 22, 4);
        layout (r, bufferSizeLabel, bufferSizeCombo);
    }

    static void pluginChosen (int, EngineContentView*);
    static void inputChannelChosen (int, EngineContentView*);
    static void outputChannelChosen (int, EngineContentView*);

private:
    PluginDescription plugin;
    StringArray midiInputs, midiOutputs;

    Label midiInputLabel;
    Label midiOutputLabel;
    Label pluginLabel;
    Label sampleRateLabel;
    Label bufferSizeLabel;
    Label inputDeviceLabel;
    Label outputDeviceLabel;

    ComboBox midiInputCombo;
    ComboBox midiOutputCombo;
    PluginPicker pluginButton;
    ComboBox sampleRateCombo;
    ComboBox bufferSizeCombo;
    AudioDeviceSelect inputDevice;
    AudioDeviceSelect outputDevice;
    TextButton inputChannelButton;
    TextButton outputChannelButton;

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

    void sourceChanged();
};

}
