
#include "Versicap.h"
#include "EngineComponent.h"
#include "MainComponent.h"
#include "PluginManager.h"

namespace vcp {

void EngineComponent::refreshMidiDevices()
{
    midiInputs = MidiInput::getDevices();
    midiInputCombo.clear (dontSendNotification);
    int i = 1;
    for (const auto& device : midiInputs)
        midiInputCombo.addItem (device, i++);
    ensureCorrectMidiInput();

    midiOutputs = MidiOutput::getDevices();
    midiOutputCombo.clear (dontSendNotification);
    i = 1;
    for (const auto& device : midiOutputs)
        midiOutputCombo.addItem (device, i++);
    ensureCorrectMidiOutput();
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

        ensureCorrectAudioInput();
        ensureCorrectAudioOutput();
        ensureTimings();
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

        if (setup.inputDeviceName.isEmpty() && setup.outputDeviceName.isEmpty())
        {
            devices.closeAudioDevice();
            stabilizeSettings();
            return;
        }

        const auto error = devices.setAudioDeviceSetup (setup, treatAsChosen);

        if (error.isNotEmpty())
        {
            ensureCorrectAudioInput();
            ensureCorrectAudioOutput();
            ensureTimings();
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
        ensureCorrectMidiInput();
    }
}

void EngineComponent::applyMidiOutput()
{
    if (auto* main = findParentComponentOfClass<vcp::MainComponent>())
    {
        const auto device = midiOutputCombo.getText();
        auto& devices = main->getVersicap().getDeviceManager();
        devices.setDefaultMidiOutput (device);
        ensureCorrectMidiOutput();
    }
}

void EngineComponent::ensureTimings()
{
    if (auto* main = findParentComponentOfClass<vcp::MainComponent>())
    {
        auto& versicap = main->getVersicap();
        auto& devices = versicap.getDeviceManager();
        const auto setup = devices.getAudioDeviceSetup();
        sampleRateCombo.setSelectedId (roundToInt (setup.sampleRate), dontSendNotification);
        bufferSizeCombo.setSelectedId (roundToInt (setup.bufferSize), dontSendNotification);
    }
}

void EngineComponent::ensureCorrectAudioInput()
{
    if (auto* main = findParentComponentOfClass<vcp::MainComponent>())
    {
        auto& versicap = main->getVersicap();
        auto& devices = versicap.getDeviceManager();
        const auto setup = devices.getAudioDeviceSetup();
        auto* const type = devices.getCurrentDeviceTypeObject();
        const auto currentIn = setup.inputDeviceName;
        const auto inNames = type != nullptr ? type->getDeviceNames (true) : StringArray();
        int inputIdx = currentIn.isNotEmpty() ? inNames.indexOf (currentIn) : -1;
        inputDeviceCombo.setSelectedItemIndex (inputIdx >= 0 ? inputIdx : 9999, 
                                                dontSendNotification);
        if (inputDeviceCombo.getSelectedItemIndex() < 0)
            inputDeviceCombo.setSelectedId (9999, dontSendNotification);
    }
}

void EngineComponent::ensureCorrectAudioOutput()
{
    if (auto* main = findParentComponentOfClass<vcp::MainComponent>())
    {
        auto& versicap = main->getVersicap();
        auto& devices = versicap.getDeviceManager();
        const auto setup = devices.getAudioDeviceSetup();
        auto* const type = devices.getCurrentDeviceTypeObject();
        const auto currentOut = setup.outputDeviceName;
        const auto outNames = type != nullptr ? type->getDeviceNames (false) : StringArray();
        int outputIdx = currentOut.isNotEmpty() ? outNames.indexOf (currentOut) : -1;
        outputDeviceCombo.setSelectedItemIndex (outputIdx >= 0 ? outputIdx : 9999, 
                                                dontSendNotification);
        if (outputDeviceCombo.getSelectedItemIndex() < 0)
            outputDeviceCombo.setSelectedId (9999, dontSendNotification);
    }
}

void EngineComponent::ensureCorrectMidiInput()
{
    if (auto* main = findParentComponentOfClass<vcp::MainComponent>())
    {
        auto& versicap = main->getVersicap();
        auto& devices = versicap.getDeviceManager();
        for (const auto& device : midiInputs)
        {
            if (devices.isMidiInputEnabled (device))
            {
                int index = midiInputs.indexOf (device);
                midiInputCombo.setSelectedItemIndex (index >= 0 ? index : -1, dontSendNotification);
                break;
            }
        }
    }
}

void EngineComponent::ensureCorrectMidiOutput()
{
    if (auto* main = findParentComponentOfClass<vcp::MainComponent>())
    {
        auto& versicap = main->getVersicap();
        auto& devices = versicap.getDeviceManager();
        int index = midiOutputs.indexOf (devices.getDefaultMidiOutputName());
        midiOutputCombo.setSelectedItemIndex (index >= 0 ? index : -1, dontSendNotification);
    }
}

void EngineComponent::pluginChosen (int result, EngineComponent* component)
{
    if (component && result > 0)
        component->pluginChosen (result);
}

void EngineComponent::pluginChosen (int result)
{
    auto& plugins = versicap.getPluginManager();
    auto& list = plugins.getKnownPlugins();

    if (const auto* const type = list.getType (list.getIndexChosenByMenu (result)))
    {
        plugin = *type;
        versicap.loadPlugin (plugin);
        updatePluginButton();
    }

#if 0
    String errorMessage;
    std::unique_ptr<AudioProcessor> processor;

    if (const auto* const type = list.getType (list.getIndexChosenByMenu (result)))
    {
        plugin = *type;
        processor.reset (plugins.createAudioPlugin (plugin, errorMessage));
    }

    if (errorMessage.isNotEmpty())
    {
        AlertWindow::showNativeDialogBox ("Versicap", "Could not create plugin", false);
        plugin = PluginDescription();
        processor.reset();
    }
    else
    {
        if (processor)
        {
            DBG("[VCP] loaded: " << processor->getName());
        }
        else
        {
            AlertWindow::showNativeDialogBox ("Versicap", "Could not instantiate plugin", false);
        }

        processor.reset(); // TODO: assign to versicap
    }
#endif
}

void EngineComponent::choosePlugin()
{
    auto& plugins = versicap.getPluginManager();
    auto& list = plugins.getKnownPlugins();
    PopupMenu menu;
    list.addToMenu (menu, KnownPluginList::sortByManufacturer);
    menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&pluginButton),
                        ModalCallbackFunction::forComponent (EngineComponent::pluginChosen, this));
}

}
