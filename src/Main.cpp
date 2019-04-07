
#include "JuceHeader.h"
#include "MainComponent.h"
#include "PluginManager.h"
#include "Versicap.h"
#include "UnlockStatus.h"

namespace vcp {

class Application : public JUCEApplication
{
public:
     Application() { }

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    void initialise (const String& commandLine) override
    {
        versicap.reset (new Versicap());
        if (maybeLaunchSlave (commandLine))
            return;

        if (sendCommandLineToPreexistingInstance())
        {
            quit();
            return;
        }

        setupGlobals();



        look.setColour (Slider::backgroundColourId, kv::LookAndFeel_KV1::widgetBackgroundColor.darker());
        LookAndFeel::setDefaultLookAndFeel (&look);
        mainWindow.reset (new MainWindow (getApplicationName(), *versicap));
    }

    void shutdown() override
    {
        shutdownGui();
        versicap->saveSettings();
        versicap->shutdown();
        versicap.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const String& commandLine) override
    {
        ignoreUnused (commandLine);
        if (mainWindow)
            mainWindow->toFront (true);
    }

    class MainWindow : public DocumentWindow
    {
    public:
        MainWindow (String name, Versicap& vc) 
            : DocumentWindow (name, Desktop::getInstance().getDefaultLookAndFeel()
                                        .findColour (ResizableWindow::backgroundColourId),
                                         DocumentWindow::allButtons),
              versicap (vc)
        {
            setUsingNativeTitleBar (true);
            setBackgroundColour (kv::LookAndFeel_KV1::widgetBackgroundColor.darker());
            setContentOwned (new MainComponent (vc), true);

            setResizable (false, false);
            constrain.setMinimumSize (440, 340);
            constrain.setMaximumSize (440, 340);
            setConstrainer (&constrain);
            centreWithSize (440, 340);

            if (auto* props = versicap.getSettings().getUserSettings())
            {
                const auto state = props->getValue ("windowPosition", String());
                if (state.isNotEmpty())
                    restoreWindowStateFromString (state);
            }
            
            setVisible (true);
        }

        ~MainWindow()
        {
            setConstrainer (nullptr);
        }

        void savePersistentData()
        {
            if (auto* props = versicap.getSettings().getUserSettings())
            {
                props->setValue ("windowPosition", getWindowStateAsString());
            }

            if (auto* const comp = dynamic_cast<MainComponent*> (getContentComponent()))
            {
                comp->saveSettings();
                comp->saveContextFile();
            }
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        void maximiseButtonPressed() override
        {
            return;
        }

    private:
        Versicap& versicap;
        ComponentBoundsConstrainer constrain;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
    kv::LookAndFeel_KV1 look;
    std::unique_ptr<Versicap> versicap;
    OwnedArray<kv::ChildProcessSlave>   slaves;

    void setupGlobals()
    {
        auto& formats = versicap->getAudioFormats();
        formats.registerBasicFormats();
        versicap->initializeExporters();
        versicap->initializePlugins();
        versicap->initializeAudioDevice();
        versicap->getUnlockStatus().load();

        auto& plugins = versicap->getPluginManager();
        plugins.scanAudioPlugins ({ "AudioUnit", "VST", "VST3" });
    }

    void shutdownGui()
    {
        versicap->closePluginWindow();
        
        if (mainWindow != nullptr)
        {
            mainWindow->savePersistentData();
            mainWindow = nullptr;
        }

        LookAndFeel::setDefaultLookAndFeel (nullptr);
    }

    bool maybeLaunchSlave (const String& commandLine)
    {
        slaves.clearQuick (true);
        slaves.add (versicap->getPluginManager().createAudioPluginScannerSlave());
        StringArray processIds = { VCP_PLUGIN_SCANNER_PROCESS_ID };
        for (auto* slave : slaves)
        {
            for (const auto& pid : processIds)
            {
                if (slave->initialiseFromCommandLine (commandLine, pid))
                {
				   #if JUCE_MAC
                    Process::setDockIconVisible (false);
				   #endif
                    juce::shutdownJuce_GUI();
                    return true;
                }
            }
        }
        
        return false;
    }
};

}

START_JUCE_APPLICATION (vcp::Application)
