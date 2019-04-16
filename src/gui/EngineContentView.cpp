
#include "gui/EngineContentView.h"
#include "gui/MainComponent.h"

#include "Versicap.h"
#include "Tags.h"
#include "Types.h"
#include "PluginManager.h"

namespace vcp {
    
void EngineContentView::updateSettings()
{
    auto project = versicap.getProject();
    sourceCombo.setSelectedId (1 + project.getSourceType(), dontSendNotification);
    latency.getValueObject().referTo (project.getPropertyAsValue (Tags::latencyComp));
}

void EngineContentView::sourceChanged()
{
    versicap.getProject().setProperty (Tags::source,
        SourceType::getSlug (getSourceType()));
    stabilizeSettings();
}

void EngineContentView::refreshMidiDevices()
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

void EngineContentView::refreshAudioDevices()
{
    auto& devices = versicap.getDeviceManager();
    const auto setup = devices.getAudioDeviceSetup();
    const auto currentIn = setup.inputDeviceName;
    const auto currentOut = setup.outputDeviceName;
    StringArray inNames, outNames;

    inputDevice.device.clear (dontSendNotification);
    outputDevice.device.clear (dontSendNotification);
    sampleRateCombo.clear (dontSendNotification);
    bufferSizeCombo.clear (dontSendNotification);

    if (auto* const type = devices.getCurrentDeviceTypeObject())
    {
        int i = 1;
        inNames  = type->getDeviceNames (true);
        outNames = type->getDeviceNames (false);
        for (const auto& device : inNames)
            inputDevice.device.addItem (device, i++);
        inputDevice.device.addSeparator();
        inputDevice.device.addItem ("No Device", 9999);
        for (const auto& device : outNames)
            outputDevice.device.addItem (device, i++);
        outputDevice.device.addSeparator();
        outputDevice.device.addItem ("No Device", 9999);
    }

    if (auto* const device = devices.getCurrentAudioDevice())
    {
        for (const auto& rate : device->getAvailableSampleRates())
            sampleRateCombo.addItem (String (roundToInt (rate)), roundToInt (rate));
        for (const auto& bufferSize : device->getAvailableBufferSizes())
            bufferSizeCombo.addItem (String (bufferSize), bufferSize);
    }

    ensureCorrectAudioInput();
    ensureCorrectAudioOutput();
    ensureTimings();
    ensureCorrectChannels();
}

void EngineContentView::applyAudioDeviceSettings()
{
    auto& devices = versicap.getDeviceManager();
    auto setup = devices.getAudioDeviceSetup();

    setup.inputDeviceName = inputDevice.device.getSelectedId() != 9999
        ? inputDevice.device.getText() : String();
    setup.outputDeviceName = outputDevice.device.getSelectedId() != 9999 
        ? outputDevice.device.getText() : String();
    setup.bufferSize = bufferSizeCombo.getSelectedId();
    setup.sampleRate = (double) sampleRateCombo.getSelectedId();

    if (setup.inputDeviceName.isEmpty() && setup.outputDeviceName.isEmpty())
    {
        devices.closeAudioDevice();
        stabilizeSettings();
        return;
    }

    setup.useDefaultInputChannels = false;
    setup.useDefaultOutputChannels = false;
    const auto error = devices.setAudioDeviceSetup (setup, true);

    if (error.isNotEmpty())
    {
        ensureCorrectAudioInput();
        ensureCorrectAudioOutput();
        ensureTimings();
        ensureCorrectChannels();
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
            "Audio Device", error);
    }
}

void EngineContentView::applyMidiInput()
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

void EngineContentView::applyMidiOutput()
{
    if (auto* main = findParentComponentOfClass<vcp::MainComponent>())
    {
        const auto device = midiOutputCombo.getText();
        auto& devices = main->getVersicap().getDeviceManager();
        devices.setDefaultMidiOutput (device);
        ensureCorrectMidiOutput();
    }
}

void EngineContentView::ensureTimings()
{
    auto& devices = versicap.getDeviceManager();
    const auto setup = devices.getAudioDeviceSetup();
    sampleRateCombo.setSelectedId (roundToInt (setup.sampleRate), dontSendNotification);
    bufferSizeCombo.setSelectedId (roundToInt (setup.bufferSize), dontSendNotification);
}

void EngineContentView::ensureCorrectAudioInput()
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
        inputDevice.device.setSelectedItemIndex (inputIdx >= 0 ? inputIdx : 9999, 
                                                dontSendNotification);
        if (inputDevice.device.getSelectedItemIndex() < 0)
            inputDevice.device.setSelectedId (9999, dontSendNotification);
    }
}

void EngineContentView::ensureCorrectAudioOutput()
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
        outputDevice.device.setSelectedItemIndex (outputIdx >= 0 ? outputIdx : 9999, 
                                                dontSendNotification);
        if (outputDevice.device.getSelectedItemIndex() < 0)
            outputDevice.device.setSelectedId (9999, dontSendNotification);
    }
}

void EngineContentView::ensureCorrectMidiInput()
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

void EngineContentView::ensureCorrectMidiOutput()
{
    if (auto* main = findParentComponentOfClass<vcp::MainComponent>())
    {
        auto& versicap = main->getVersicap();
        auto& devices = versicap.getDeviceManager();
        int index = midiOutputs.indexOf (devices.getDefaultMidiOutputName());
        midiOutputCombo.setSelectedItemIndex (index >= 0 ? index : -1, dontSendNotification);
    }
}

void EngineContentView::pluginChosen (int result, EngineContentView* component)
{
    if (component && result > 0)
        component->pluginChosen (result);
}

void EngineContentView::pluginChosen (int result)
{
    auto& plugins = versicap.getPluginManager();
    auto& list = plugins.getKnownPlugins();

    if (const auto* const type = list.getType (list.getIndexChosenByMenu (result)))
    {
        plugin = *type;
        versicap.loadPlugin (plugin);
        updatePluginButton();
    }
}

void EngineContentView::inputChannelChosen (int result, EngineContentView* comp)
{
    if (comp && result > 0)
        comp->inputChannelChosen (result);
}

void EngineContentView::inputChannelChosen (int result)
{
    auto& devices = versicap.getDeviceManager();
    auto setup = devices.getAudioDeviceSetup();
    if (result >= 100 && result < 200)
    {
        int channel = result - 100;
        setup.useDefaultInputChannels = false;
        setup.inputChannels.setRange (0, 32, false);
        setup.inputChannels.setBit (channel, true);
    }
    else if (result >= 200 && result < 300)
    {
        int channel = result - 200;
        setup.useDefaultInputChannels = false;
        setup.inputChannels.setRange (0, 32, false);
        setup.inputChannels.setBit (channel, true);
        setup.inputChannels.setBit (channel + 1, true);
    }
    
    devices.setAudioDeviceSetup (setup, true);
    ensureCorrectChannels();
}

void EngineContentView::outputChannelChosen (int result, EngineContentView* comp)
{
    if (comp && result > 0)
        comp->outputChannelChosen (result);
}

void EngineContentView::outputChannelChosen (int result)
{
    auto& devices = versicap.getDeviceManager();
    auto setup = devices.getAudioDeviceSetup();
    if (result >= 100 && result < 200)
    {
        int channel = result - 100;
        setup.useDefaultOutputChannels = false;
        setup.outputChannels.setRange (0, 32, false);
        setup.outputChannels.setBit (channel, true);
    }
    else if (result >= 200 && result < 300)
    {
        int channel = result - 200;
        setup.useDefaultOutputChannels = false;
        setup.outputChannels.setRange (0, 32, false);
        setup.outputChannels.setBit (channel, true);
        setup.outputChannels.setBit (channel + 1, true);
    }

    devices.setAudioDeviceSetup (setup, true);
    ensureCorrectChannels();
}

void EngineContentView::ensureCorrectChannels()
{
    auto& devices = versicap.getDeviceManager();
    auto setup = devices.getAudioDeviceSetup();
    // DBG("ins  : " << setup.inputChannels.toString(2));
    // DBG("outs : " << setup.outputChannels.toString(2));
    for (int i = 0; i < 32; ++i)
    {
        if (setup.inputChannels [i])
        {
            String name = String (i + 1);
            if (setup.inputChannels [i + 1])
                name << " - " << int (i + 2);
            inputDevice.channels.setButtonText (name);
            break;
        }
    }

    for (int i = 0; i < 32; ++i)
    {
        if (setup.outputChannels [i])
        {
            String name = String (i + 1);
            if (setup.outputChannels [i + 1])
                name << " - " << int (i + 2);
            outputDevice.channels.setButtonText (name);
            break;
        }
    }
}

void EngineContentView::choosePlugin()
{
    auto& plugins = versicap.getPluginManager();
    auto& list = plugins.getKnownPlugins();
    PopupMenu menu;
    list.addToMenu (menu, KnownPluginList::sortByManufacturer);
    menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&pluginButton),
                        ModalCallbackFunction::forComponent (EngineContentView::pluginChosen, this));
}

}
