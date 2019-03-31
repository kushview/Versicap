
#include "Versicap.h"
#include "EngineComponent.h"
#include "MainComponent.h"

namespace vcp {

void EngineComponent::startRender()
{
    if (auto* main = findParentComponentOfClass<vcp::MainComponent>())
        main->startRendering();
}

void EngineComponent::refreshAudioDevices()
{
    if (auto* main = findParentComponentOfClass<vcp::MainComponent>())
    {
        auto& versicap = main->getVersicap();
        auto& devices = versicap.getDeviceManager();
        const auto setup = devices.getAudioDeviceSetup();
        const auto currentIn = setup.inputDeviceName;
        const auto currentOut = setup.outputDeviceName;
        StringArray inNames, outNames;
        inputDeviceCombo.clear (dontSendNotification);
        outputDeviceCombo.clear (dontSendNotification);
        
        if (auto* const type = devices.getCurrentDeviceTypeObject())
        {
            int i = 1;
            inNames  = type->getDeviceNames (true);
            outNames = type->getDeviceNames (false);
            for (const auto& device : inNames)
                inputDeviceCombo.addItem (device, i++);
            inputDeviceCombo.addSeparator();
            inputDeviceCombo.addItem ("No Device", 9999);
            for (const auto& device : outNames)
                outputDeviceCombo.addItem (device, i++);
            outputDeviceCombo.addSeparator();
            outputDeviceCombo.addItem ("No Device", 9999);
        }

        int inputIdx = currentIn.isNotEmpty() ? inNames.indexOf (currentIn) : -1;
        inputDeviceCombo.setSelectedItemIndex (inputIdx >= 0 ? inputIdx : 9999, 
                                               dontSendNotification);
        if (inputDeviceCombo.getSelectedItemIndex() < 0)
            inputDeviceCombo.setSelectedId (9999, dontSendNotification);
        
        int outputIdx = currentOut.isNotEmpty() ? outNames.indexOf (currentOut) : -1;
        outputDeviceCombo.setSelectedItemIndex (outputIdx >= 0 ? outputIdx : 9999, 
                                                dontSendNotification);
        if (outputDeviceCombo.getSelectedItemIndex() < 0)
            outputDeviceCombo.setSelectedId (9999, dontSendNotification);

        sampleRateCombo.setSelectedId (roundToInt (setup.sampleRate), dontSendNotification);
        bufferSizeCombo.setSelectedId (roundToInt (setup.bufferSize), dontSendNotification);
    }
}

void EngineComponent::applyAudioDeviceSettings()
{
    if (auto* main = findParentComponentOfClass<vcp::MainComponent>())
    {
        auto& versicap = main->getVersicap();
        auto& devices = versicap.getDeviceManager();
        auto setup = devices.getAudioDeviceSetup();
        bool treatAsChosen = true;
        setup.inputDeviceName = inputDeviceCombo.getSelectedId() != 9999
            ? inputDeviceCombo.getText() : String();
        setup.outputDeviceName = outputDeviceCombo.getSelectedId() != 9999 
            ? outputDeviceCombo.getText() : String();
        setup.bufferSize = bufferSizeCombo.getSelectedId();
        setup.sampleRate = (double) sampleRateCombo.getSelectedId();
        setup.useDefaultInputChannels = setup.useDefaultOutputChannels = true;
        const auto error = devices.setAudioDeviceSetup (setup, treatAsChosen);

        if (error.isNotEmpty())
        {
            refreshAudioDevices();
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                "Audio Device", error);
        }
    }
}

void EngineComponent::applyMidiInput()
{
    if (auto* main = findParentComponentOfClass<vcp::MainComponent>())
    {
        const auto device = midiInputCombo.getText();
        auto& devices = main->getVersicap().getDeviceManager();
        for (const auto& name : MidiInput::getDevices())
            devices.setMidiInputEnabled (name, false);
        devices.setMidiInputEnabled (device, true);
    }
}

void EngineComponent::applyMidiOutput()
{
    if (auto* main = findParentComponentOfClass<vcp::MainComponent>())
    {
        const auto device = midiOutputCombo.getText();
        auto& devices = main->getVersicap().getDeviceManager();
        devices.setDefaultMidiOutput (device);
    }
}

}
