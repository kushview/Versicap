
#include "controllers/GuiController.h"
#include "controllers/ProjectsController.h"
#include "engine/AudioEngine.h"
#include "gui/PluginWindow.h"

#include "Commands.h"
#include "Exporter.h"
#include "PluginManager.h"
#include "Project.h"
#include "Render.h"
#include "Versicap.h"
#include "UnlockStatus.h"

namespace vcp {

struct Versicap::Impl : public AudioIODeviceCallback,
                        public MidiInputCallback,
                        public ChangeListener,
                        public ApplicationCommandTarget
{
    Impl()
        : peaks (32)
    {
    }

    ~Impl()
    { 
    }

    void audioDeviceAboutToStart (AudioIODevice* device) override   { engine->prepare (device); }
    void audioDeviceStopped() override                              { engine->release(); }
    void audioDeviceIOCallback (const float** input, int numInputs, 
                                float** output, int numOutputs,
                                int nframes) override
    {
        engine->process (input, numInputs, output, numOutputs, nframes);
    }

    void audioDeviceError (const String& errorMessage) override
    {
        AlertWindow::showNativeDialogBox ("Versicap", errorMessage, false);
    }

    void handleIncomingMidiMessage (MidiInput*, const MidiMessage& message) override
    { 
        engine->addMidiMessage (message);
    }

    void handlePartialSysexMessage (MidiInput* source, const uint8* messageData,
                                    int numBytesSoFar, double timestamp) override
    { 
        ignoreUnused (source, messageData, numBytesSoFar, timestamp);
    }

    void updateLockedStatus()
    {
        const bool unlocked = (bool) unlock->isUnlocked();
        shouldProcess.set (unlocked ? 1 : 0);
        engine->setEnabled (unlocked);
    }

    //=========================================================================
    void changeListenerCallback (ChangeBroadcaster* broadcaster) override
    {
        if (broadcaster == unlock.get())
        {
            updateLockedStatus();
        }
    }

    //=========================================================================
    ApplicationCommandTarget* getNextCommandTarget() override
    {
        return nullptr;
    }

    void getAllCommands (Array<CommandID>& commands) override
    {
        commands.addArray ({
            Commands::projectSave,
            Commands::projectSaveAs,
            Commands::projectNew,
            Commands::projectOpen,
            Commands::projectRecord,
            
            Commands::showAbout,
           #if 0
            Commands::checkForUpdates
           #endif
        });
    }

    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override
    {
        for (auto* const controller : controllers)
            controller->getCommandInfo (commandID, result);
    }

    bool perform (const InvocationInfo& info) override
    {
        bool handled = false;

        for (auto* const controller : controllers)
        {
            handled = controller->perform (info);
            if (handled)
                break;
        }

        if (! handled)
        {
            handled = true;
            switch (info.commandID)
            {
                default:
                    handled = false;
                    break;
            }
        }

        return handled;
    }

    //=========================================================================
    File projectFile;
    Project project;

    Atomic<int> shouldProcess { 0 };
    OwnedArray<Controller> controllers;

    std::unique_ptr<AudioEngine> engine;
    Settings settings;
    AudioThumbnailCache peaks;
    ApplicationCommandManager commands;
    OwnedArray<Exporter> exporters;
    OptionalScopedPointer<AudioDeviceManager> devices;
    OptionalScopedPointer<AudioFormatManager> formats;
    OptionalScopedPointer<PluginManager> plugins;
    std::unique_ptr<UnlockStatus> unlock;
    MidiKeyboardState keyboardState;

    //=========================================================================
    std::unique_ptr<PluginWindow> window;
};

Versicap::Versicap()
{
    impl.reset (new Impl());
    
    impl->devices.setOwned (new AudioDeviceManager());
    impl->formats.setOwned (new AudioFormatManager());
    impl->plugins.setOwned (new PluginManager());
    impl->unlock.reset (new UnlockStatus (impl->settings));
    impl->engine.reset (new AudioEngine (*impl->formats, impl->plugins->getAudioPluginFormats()));

    auto& controllers = impl->controllers;
    controllers.add (new GuiController (*this));
    controllers.add (new ProjectsController (*this));

    impl->engine->onRenderStarted = [this]()
    {
        listeners.call ([](Listener& l) { l.renderStarted(); });
    };
    
    impl->engine->onRenderStopped = [this]()
    {
        listeners.call ([](Listener& l) { l.renderWillStop(); });
        impl->peaks.clear();
        auto project = getProject();
        project.setSamples (impl->engine->getRenderedSamples());
        listeners.call ([](Listener& l) { l.renderStopped(); });
    };

    impl->engine->onRenderCancelled = [this]()
    {
        listeners.call ([](Listener& l) { l.renderWillStop(); });
        listeners.call ([](Listener& l) { l.renderStopped(); });
    };
}

Versicap::~Versicap()
{
    impl->engine.reset();
    impl->formats->clearFormats();
    impl->controllers.clear (true);
    impl.reset();
}

template<class ControllerType> ControllerType* Versicap::findController() const
{
    for (auto* const c : impl->controllers)
        if (auto* const fc = dynamic_cast<ControllerType*> (c))
            return fc;
    return nullptr;
}

File Versicap::getApplicationDataPath()
{
   #if JUCE_MAC
    return File::getSpecialLocation (File::userApplicationDataDirectory)
        .getChildFile ("Application Support/Versicap");
   #else
    return File::getSpecialLocation (File::userApplicationDataDirectory)
        .getChildFile ("Versicap");
   #endif
}

File Versicap::getUserDataPath()    { return File::getSpecialLocation(File::userMusicDirectory).getChildFile ("Versicap"); }
File Versicap::getProjectsPath()    { return getUserDataPath().getChildFile ("Projects"); }
File Versicap::getSamplesPath()     { return getUserDataPath().getChildFile ("Samples"); }

void Versicap::initializeDataPath()
{
    if (! getApplicationDataPath().exists())
        getApplicationDataPath().createDirectory();
    if (! getUserDataPath().exists())
        getUserDataPath().createDirectory();
    if (! getSamplesPath().exists())
        getSamplesPath().createDirectory();
    if (! getProjectsPath().exists())
        getProjectsPath().createDirectory();
}

void Versicap::initializeExporters()
{
    Exporter::createExporters (impl->exporters);
    getAudioFormats().registerBasicFormats();
}

void Versicap::initializeAudioDevice()
{
    auto& devices = getDeviceManager();
    auto& settings = impl->settings;
    bool initDefault = true;
    
    devices.addAudioCallback (impl.get());
    devices.addMidiInputCallback (String(), impl.get());

    if (auto* const props = settings.getUserSettings())
    {
        if (auto* xml = props->getXmlValue ("devices"))
        {
            initDefault = devices.initialise (32, 32, xml, false).isNotEmpty();
            deleteAndZero (xml);
        }
    }
    
    if (initDefault)
    {
        devices.initialiseWithDefaultDevices (32, 32);
    }
}

void Versicap::initializePlugins()
{
    auto& plugins = getPluginManager();
    plugins.addDefaultFormats();
    const auto file = getApplicationDataPath().getChildFile ("plugins.xml");
    plugins.restoreAudioPlugins (file);
}

void Versicap::initializeUnlockStatus()
{
    getUnlockStatus().load();
    impl->updateLockedStatus();
    getUnlockStatus().addChangeListener (impl.get());
}

void Versicap::initialize()
{
    initializeDataPath();
    initializeExporters();
    initializePlugins();
    initializeAudioDevice();
    initializeUnlockStatus();

    impl->commands.registerAllCommandsForTarget (impl.get());
    impl->commands.setFirstCommandTarget (impl.get());

    for (auto* const controller : impl->controllers)
    {
        DBG("[VCP] initialize " << controller->getName());
        controller->initialize();
    }
}

void Versicap::shutdown()
{
    closePluginWindow();

    for (auto* const controller : impl->controllers)
    {
        DBG("[VCP] shutting down " << controller->getName());
        controller->shutdown();
    }

    auto& unlock = getUnlockStatus();
    unlock.removeChangeListener (impl.get());
    unlock.save();

    auto& devices = getDeviceManager();
    devices.removeAudioCallback (impl.get());
    devices.removeMidiInputCallback (String(), impl.get());
    devices.closeAudioDevice();
}

void Versicap::saveSettings()
{
    auto& settings = getSettings();
    auto& devices  = getDeviceManager();
    auto& plugins  = getPluginManager();
    auto& formats  = getAudioFormats();
    auto& unlock   = getUnlockStatus();

    if (auto xml = std::unique_ptr<XmlElement> (plugins.getKnownPlugins().createXml()))
    {
        const auto file = getApplicationDataPath().getChildFile ("plugins.xml");
        xml->writeToFile (file, String());
    }
    
    if (auto* const props = settings.getUserSettings())
    {
        if (auto* devicesXml = devices.createStateXml())
        {
            props->setValue ("devices", devicesXml);
            deleteAndZero (devicesXml);
        }
    }

    unlock.save();
}

void Versicap::saveRenderContext()
{
    File contextFile = getApplicationDataPath().getChildFile("context.versicap");
    if (! contextFile.getParentDirectory().exists())
        contextFile.getParentDirectory().createDirectory();
    saveProject (contextFile);
}

AudioThumbnail* Versicap::createAudioThumbnail (const File& file)
{
    auto* thumb = new AudioThumbnail (1, *impl->formats, impl->peaks);
    thumb->setSource (new FileInputSource (file));
    impl->peaks.removeThumb (thumb->getHashCode());

    return thumb;
}

AudioEngine& Versicap::getAudioEngine()                     { return *impl->engine; }
AudioThumbnailCache& Versicap::getAudioThumbnailCache()     { return impl->peaks; }
ApplicationCommandManager& Versicap::getCommandManager()    { return impl->commands; }
const OwnedArray<Exporter>& Versicap::getExporters() const  { return impl->exporters; }
Settings& Versicap::getSettings()                           { return impl->settings; }
AudioDeviceManager& Versicap::getDeviceManager()            { return *impl->devices; }
AudioFormatManager& Versicap::getAudioFormats()             { return *impl->formats; }
MidiKeyboardState& Versicap::getMidiKeyboardState()         { return impl->keyboardState; }
PluginManager& Versicap::getPluginManager()                 { return *impl->plugins; }
UnlockStatus& Versicap::getUnlockStatus()                   { return *impl->unlock; }

void Versicap::loadPlugin (const PluginDescription& type, bool clearProjectPlugin)
{
    auto& plugins = getPluginManager();
    String errorMessage;
    std::unique_ptr<AudioProcessor> processor (plugins.createAudioPlugin (type, errorMessage));
    
    if (errorMessage.isNotEmpty())
    {
        AlertWindow::showNativeDialogBox ("Versicap", errorMessage, false);
    }
    else
    {
        if (processor)
        {
            closePluginWindow();
            if (clearProjectPlugin)
                impl->project.clearPlugin();
            impl->engine->setAudioProcessor (processor.release());
            impl->project.setPluginDescription (type);
            showPluginWindow();
        }
        else
        {
            AlertWindow::showNativeDialogBox ("Versicap", "Could not instantiate plugin", false);
        }
    }
}

void Versicap::closePlugin (bool clearProjectPlugin)
{
    closePluginWindow();
    std::unique_ptr<AudioProcessor> oldProc;

    impl->engine->clearAudioProcessor();

    if (clearProjectPlugin)
        impl->project.clearPlugin();
}

void Versicap::closePluginWindow()
{
    impl->window.reset();
}

void Versicap::showPluginWindow()
{
    auto* const processor = impl->engine->getAudioProcessor();

    if (nullptr == processor)
        return;

    if (impl->window)
    {
        impl->window->toFront (false);
        return;
    }

    PluginWindow* window = nullptr;
    if (auto* const editor = processor->createEditorIfNeeded())
    {
        window = new PluginWindow (impl->window, editor);
    }
    else
    {
        window = new PluginWindow (impl->window, new GenericAudioProcessorEditor (processor));
    }

    if (window)
        window->toFront (false);
}

Result Versicap::startRendering()
{
    auto& engine = getAudioEngine();
    if (engine.isRendering())
        return Result::fail ("Versicap is already rendering");
       
    const auto project = getProject();
    RenderContext context;
    project.getRenderContext (context);

    const auto result = engine.startRendering (context);
    if (result.wasOk())
        listeners.call ([](Listener& listener) { listener.renderWillStart(); });
    return result;
}

Result Versicap::startRendering (const RenderContext&)
{
    jassertfalse;
    return startRendering();
}

void Versicap::stopRendering()
{
    auto& engine = getAudioEngine();
    engine.cancelRendering();
    listeners.call ([](Listener& listener) { listener.renderWillStop(); });
}

Project Versicap::getProject() const { return impl->project; }

bool Versicap::saveProject (const File& file)
{
    auto project = getProject();
    if (! project.getValueTree().isValid())
        return false;

    auto setup = impl->devices->getAudioDeviceSetup();
    project.setAudioDeviceSetup (setup);
    project.setProperty (Tags::midiOutput, impl->engine->getDefaultMidiOutputName());
    // TODO: set midi input(s) here

    if (auto* processor = impl->engine->getAudioProcessor())
        project.updatePluginState (*processor);

    return project.writeToFile (file);
}

bool Versicap::loadProject (const File& file)
{
    auto newProject = Project();
    if (! newProject.loadFile (file))
        return false;

    const auto oldFile = getProjectFile();
    setProjectFile (file); // workaround so file is updated before 
                           // calling listeners
    if (setProject (newProject))
    {
        return true;
    }
    else
    {
        setProjectFile (oldFile);
    }

    return false;
}

bool Versicap::setProject (const Project& newProject)
{
    auto& engine = getAudioEngine();
    auto& devices = getDeviceManager();

    impl->project = newProject;
    auto& project = impl->project;

    AudioDeviceManager::AudioDeviceSetup setup;
    project.getAudioDeviceSetup (setup);
    devices.setAudioDeviceSetup (setup, true);

    PluginDescription desc;
    if (project.getPluginDescription (getPluginManager(), desc))
    {
        loadPlugin (desc, false);
        if (auto* const proc = engine.getAudioProcessor())
            project.applyPluginState (*proc);
    }

    auto midiOut = project.getProperty (Tags::midiOutput).toString();
    engine.setDefaultMidiOutput (midiOut);

    listeners.call ([](Listener& listener) { listener.projectChanged(); });

    return true;
}

File Versicap::getProjectFile() const { return impl->projectFile; }
void Versicap::setProjectFile (const File& file) { impl->projectFile = file; }

bool Versicap::hasProjectChanged() const
{
    if (auto* const pc = findController<ProjectsController>())
        return pc->hasProjectChanged();
    return false;
}

void Versicap::launched()
{
    for (auto* const controller : impl->controllers)
        controller->launched();
}

}
