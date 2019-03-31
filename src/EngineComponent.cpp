
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

        const auto currentIn = inputDeviceCombo.getText();
        const auto currentOut = outputDeviceCombo.getText();
        StringArray inNames, outNames;
        inputDeviceCombo.clear();
        outputDeviceCombo.clear();
        
        if (auto* type = devices.getCurrentDeviceTypeObject())
        {
            int i = 1;
            inNames  = type->getDeviceNames (true);
            outNames = type->getDeviceNames (false);
            for (const auto& device : inNames)
                inputDeviceCombo.addItem (device, i++);
            inputDeviceCombo.addItem ("No Device", 9999);
            for (const auto& device : outNames)
                outputDeviceCombo.addItem (device, i++);
            inputDeviceCombo.addItem ("No Device", 9999);
        }

        if (currentIn.isNotEmpty())
            inputDeviceCombo.setSelectedItemIndex (
                jmax (0, inNames.indexOf (currentIn)), dontSendNotification);
        else
            inputDeviceCombo.setSelectedItemIndex (0, dontSendNotification);
        
        if (currentOut.isNotEmpty())
            inputDeviceCombo.setSelectedItemIndex (
                jmax (0, outNames.indexOf (currentOut)), dontSendNotification);
        else
            inputDeviceCombo.setSelectedItemIndex (0, dontSendNotification);
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
        setup.inputDeviceName = inputDeviceCombo.getText();
        setup.outputDeviceName = outputDeviceCombo.getText();
        setup.bufferSize = bufferSizeCombo.getSelectedId();
        setup.sampleRate = (double) sampleRateCombo.getSelectedId();
        setup.useDefaultInputChannels = setup.useDefaultOutputChannels = true;
        const auto error = devices.setAudioDeviceSetup (setup, treatAsChosen);
        if (error.isNotEmpty())
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                "Audio Device", error);
        }
    }
}

}
