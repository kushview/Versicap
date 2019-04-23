
#include "engine/AudioEngine.h"
#include "gui/PluginPicker.h"
#include "PluginManager.h"
#include "Project.h"
#include "Types.h"
#include "Versicap.h"

namespace vcp {

//=============================================================================
class ProjectPropertyComponent : public PropertyComponent
{
public:
    ProjectPropertyComponent (const Project& proj, const String& propertyName)
        : PropertyComponent (propertyName),
          project (proj) { }

protected:
    Project project;
};

//=============================================================================
class SampleRatePropertyComponent : public ProjectPropertyComponent,
                                    public ChangeListener
{
public:
    SampleRatePropertyComponent (Versicap& vc, const Project& p)
        : ProjectPropertyComponent (p, "Sample Rate"),
          versicap (vc), devices (vc.getDeviceManager())
    {
        addAndMakeVisible (combo);
        refresh();
        devices.addChangeListener (this);
        combo.onChange = [this]()
        {
            devices.getAudioDeviceSetup (setup);
            setup.sampleRate = static_cast<double> (combo.getSelectedId());
            devices.setAudioDeviceSetup (setup, true);
        };
    }

    ~SampleRatePropertyComponent()
    {
        devices.removeChangeListener (this);
    }

    void refresh() override
    {
        const auto last = setup;
        devices.getAudioDeviceSetup (setup);
        const auto iSampleRate = roundToInt (setup.sampleRate);

        if (setup.inputDeviceName != last.inputDeviceName ||
            setup.outputDeviceName != last.outputDeviceName ||
            combo.getNumItems() <= 0)
        {
            combo.clear (dontSendNotification);
            if (auto* device = devices.getCurrentAudioDevice())
                for (const auto& rate : device->getAvailableSampleRates())
                    combo.addItem (String (roundToInt (rate)), roundToInt (rate));
        }

        if (iSampleRate != combo.getSelectedId())
            combo.setSelectedId (iSampleRate, dontSendNotification);
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        refresh();
    }

private:
    Versicap& versicap;
    AudioDeviceManager& devices;
    AudioDeviceManager::AudioDeviceSetup setup;
    ComboBox combo;
};

//=============================================================================
class BufferSizePropertyComponent : public ProjectPropertyComponent,
                                    public ChangeListener
{
public:
    BufferSizePropertyComponent (Versicap& vc, const Project& p)
        : ProjectPropertyComponent (p, "Buffer Size"),
          versicap (vc),
          devices (vc.getDeviceManager())
    {
        addAndMakeVisible (combo);
        refresh();
        devices.addChangeListener (this);
        combo.onChange = [this]()
        {
            devices.getAudioDeviceSetup (setup);
            setup.bufferSize = combo.getSelectedId();
            devices.setAudioDeviceSetup (setup, true);
        };
    }

    ~BufferSizePropertyComponent()
    {
        devices.removeChangeListener (this);
    }

    void refresh() override
    {
        const auto last = setup;
        devices.getAudioDeviceSetup (setup);

        if (setup.inputDeviceName != last.inputDeviceName ||
            setup.outputDeviceName != last.outputDeviceName ||
            combo.getNumItems() <= 0)
        {
            if (auto* device = devices.getCurrentAudioDevice())
            {
                combo.clear (dontSendNotification);
                for (const auto& size : device->getAvailableBufferSizes())
                    combo.addItem (String (size), size);
            }
        }

        if (setup.bufferSize > 0)
            combo.setSelectedId (setup.bufferSize, dontSendNotification);
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        refresh();
    }

private:
    AudioDeviceManager::AudioDeviceSetup setup;
    Versicap& versicap;
    AudioDeviceManager& devices;
    ComboBox combo;

    void refreshChoices()
    {

    }
};

//=============================================================================
class AudioDevicePropertyComponent : public ProjectPropertyComponent,
                                     public ChangeListener
{
public:
    AudioDevicePropertyComponent (Versicap& vc, const Project& p, bool isInput)
        : ProjectPropertyComponent (p, isInput ? "Audio In" : "Audio Out"),
          versicap (vc), devices (vc.getDeviceManager()), inputDevice (isInput)
    {
        addAndMakeVisible (combo);
        refresh();
        devices.addChangeListener (this);
        combo.onChange = [this]()
        {
            devices.getAudioDeviceSetup (setup);
            if (inputDevice)
            {
                if (setup.inputDeviceName != combo.getText())
                    setup.inputDeviceName = combo.getText();
            }
            else
            {
                if (setup.outputDeviceName != combo.getText())
                    setup.outputDeviceName = combo.getText();
            }
            devices.setAudioDeviceSetup (setup, true);
        };
    }

    ~AudioDevicePropertyComponent()
    {
        devices.removeChangeListener (this);
    }

    void refresh() override
    {
        const auto last = setup;
        const auto name = inputDevice ? setup.inputDeviceName : setup.outputDeviceName;
        devices.getAudioDeviceSetup (setup);
        
        if (deviceNames.isEmpty() || deviceNames.size() != combo.getNumItems())
        {
            if (auto* const type = devices.getCurrentDeviceTypeObject())
                deviceNames = type->getDeviceNames (inputDevice);
            combo.clear (dontSendNotification);
            int i = 1;
            for (const auto& name : deviceNames)
                combo.addItem (name, i++);
        }

        const int index = deviceNames.indexOf (name);
        if (isPositiveAndBelow (index, combo.getNumItems()))
            combo.setSelectedItemIndex (index, dontSendNotification);
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        refresh();
    }

private:
    ComboBox combo;
    StringArray deviceNames;
    AudioDeviceManager::AudioDeviceSetup setup;
    Versicap& versicap;
    AudioDeviceManager& devices;
    bool inputDevice = false;
};

class MidiDevicePropertyComponent : public ProjectPropertyComponent,
                                    public ChangeListener
{
public:
    MidiDevicePropertyComponent (Versicap& vc, const Project& p, bool isInput)
        : ProjectPropertyComponent (p, isInput ? "MIDI In" : "MIDI Out"),
          versicap (vc), devices (vc.getDeviceManager()), inputDevice (isInput)
    {
        addAndMakeVisible (combo);
        refresh();
        devices.addChangeListener (this);

        combo.onChange = [this]()
        {
            if (inputDevice)
            {
                devices.removeChangeListener (this);
                for (const auto& name : deviceNames)
                    devices.setMidiInputEnabled (name, false);
                devices.addChangeListener (this);

                if (combo.getText().isNotEmpty())
                {
                    devices.setMidiInputEnabled (combo.getText(), true);
                    // TODO: set this in project save and/or in another handler
                    project.setProperty (Tags::midiInput, combo.getText());
                }
            }
            else
            {
                auto& engine = versicap.getAudioEngine();
                engine.setDefaultMidiOutput (combo.getText());
            }
        };
    }

    ~MidiDevicePropertyComponent()
    {
        devices.removeChangeListener (this);
    }

    String getDeviceName()
    {
        if (inputDevice)
        {
            for (const auto& name : deviceNames)
                if (devices.isMidiInputEnabled (name))
                    return name;
            return {};
        }
        
        auto& engine = versicap.getAudioEngine();
        return engine.getDefaultMidiOutputName();
    }

    void refresh() override
    {
        if (deviceNames.isEmpty() || deviceNames.size() != combo.getNumItems())
        {
            deviceNames = inputDevice ? MidiInput::getDevices() : MidiOutput::getDevices();
            combo.clear (dontSendNotification);
            int i = 1;
            for (const auto& name : deviceNames)
                combo.addItem (name, i++);
        }

        const auto name = getDeviceName();
        const int index = deviceNames.indexOf (name);
        if (isPositiveAndBelow (index, combo.getNumItems()))
            combo.setSelectedItemIndex (index, dontSendNotification);
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        refresh();
    }

private:
    ComboBox combo;
    StringArray deviceNames;
    AudioDeviceManager::AudioDeviceSetup setup;
    Versicap& versicap;
    AudioDeviceManager& devices;
    bool inputDevice = false;
};

//=============================================================================
class DataPathPropertyComponent : public ProjectPropertyComponent,
                                  public FilenameComponentListener,
                                  private Value::Listener
{
public:
    DataPathPropertyComponent (const Project& p)
        : ProjectPropertyComponent (p, "Data Path"),
          directory ("DataPath", File(), false, true, true,
                     String(), String(), "Path for rendered files")
    {
        addAndMakeVisible (directory);
        value = project.getPropertyAsValue (Tags::dataPath);
        value.addListener (this);
        refresh();
    }

    void refresh() override
    {
        const String path = value.getValue().toString();
        if (File::isAbsolutePath (path))
            directory.setCurrentFile (File (path), dontSendNotification);
    }

    void filenameComponentChanged (FilenameComponent*) override
    {
        value.removeListener (this);
        project.setProperty (Tags::dataPath, directory.getCurrentFile().getFullPathName());
        value.addListener (this);
    }

private:
    FilenameComponent directory;
    Value value;

    void valueChanged (Value&) override { refresh(); }
};

//=============================================================================
class PluginPropertyComponent : public ProjectPropertyComponent
{
public:
    PluginPropertyComponent (Versicap& vc, const Project& p)
        : ProjectPropertyComponent (p, "Plugin"),
          versicap (vc)
    {
        addAndMakeVisible (plugin);

        plugin.onChoose = [this]() { choosePlugin(); };
        plugin.onEditor = std::bind (&Versicap::showPluginWindow, &versicap);
        plugin.onClose = [this]()
        {
            versicap.closePlugin();
            plugin.setPluginName (String());
        };
    }

    void refresh() override
    { 
        PluginDescription desc;
        project.getPluginDescription (versicap.getPluginManager(), desc);
        if (desc.name.isEmpty())
            plugin.setPluginName (String());
        else
            plugin.setPluginName (desc.name);
    }

    static void pluginChosen (int result, PluginPropertyComponent* component)
    {
        if (component && result > 0)
            component->pluginChosen (result);
    }

private:
    Versicap& versicap;
    PluginPicker plugin;
    ValueTree node;
    
    void choosePlugin()
    {
        auto& plugins = versicap.getPluginManager();
        auto& list = plugins.getKnownPlugins();
        PopupMenu menu;
        list.addToMenu (menu, KnownPluginList::sortByManufacturer);
        menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&plugin),
                            ModalCallbackFunction::forComponent (PluginPropertyComponent::pluginChosen, this));
    }

    void pluginChosen (int result)
    {
        auto& plugins = versicap.getPluginManager();
        auto& list = plugins.getKnownPlugins();
        if (const auto* const type = list.getType (list.getIndexChosenByMenu (result)))
        {
            versicap.loadPlugin (*type);
            refresh();
        }
    }
};

//=============================================================================
void Project::getProperties (Versicap& versicap, Array<PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (getPropertyAsValue (Tags::name),
        "Name", 120, false, true));
    props.add (new DataPathPropertyComponent (*this));
    props.add (new ChoicePropertyComponent (getPropertyAsValue (Tags::source),
        "Source", SourceType::getChoices() , SourceType::getValues()));

    props.add (new PluginPropertyComponent (versicap, *this));
}

void Project::getDevicesProperties (Versicap& versicap, Array<PropertyComponent*>& props)
{
    props.add (new SampleRatePropertyComponent (versicap,  *this));
    props.add (new BufferSizePropertyComponent (versicap,  *this));
    props.add (new AudioDevicePropertyComponent (versicap, *this, true));
    props.add (new AudioDevicePropertyComponent (versicap, *this, false));    
    props.add (new MidiDevicePropertyComponent (versicap,  *this, true));
    props.add (new MidiDevicePropertyComponent (versicap,  *this, false));
}

void Project::getRecordingProperties (Versicap&, Array<PropertyComponent*>& props)
{
    props.add (new ChoicePropertyComponent (getPropertyAsValue (Tags::format),
        "Format", FormatType::getChoices(), FormatType::getValues()));
    props.add (new ChoicePropertyComponent (getPropertyAsValue (Tags::channels),
        "Channels", { "Mono", "Stereo" }, { 1, 2 }));
    props.add (new ChoicePropertyComponent (getPropertyAsValue (Tags::bitDepth),
        "Bit Depth", { "16 bit", "24 bit" }, { 16, 24 }));
}

}
