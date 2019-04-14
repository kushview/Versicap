
#ifndef VCP_STLIB

#include "gui/MainComponent.h"
#include "gui/MainMenu.h"

#include "PluginManager.h"
#include "Project.h"
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
        Project dummy;
        dummy.writeToFile (File("/Users/mfisher/Desktop/test.versicap"));
        dummy.loadFile (File("/Users/mfisher/Desktop/test.versicap"));

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
        versicap->saveRenderContext();
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
            menu.reset (new MainMenu (*this));
            menu->setupMenu();

            setUsingNativeTitleBar (true);
            setBackgroundColour (kv::LookAndFeel_KV1::widgetBackgroundColor.darker());
            setContentOwned (new MainComponent (vc), true);

            setResizable (true, false);
            constrain.setMinimumSize (540, 340);
            // constrain.setMaximumSize (1280, 340);
            setConstrainer (&constrain);
            centreWithSize (640, 360 + 240);

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
           #if ! JUCE_MAC
            setMenuBar (nullptr);
           #endif
            menu.reset();
        }

        void savePersistentData()
        {
            if (auto* props = versicap.getSettings().getUserSettings())
                props->setValue ("windowPosition", getWindowStateAsString());
            if (auto* const comp = dynamic_cast<MainComponent*> (getContentComponent()))
                comp->saveSettings();
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
        std::unique_ptr<MainMenu> menu;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
    kv::LookAndFeel_KV1 look;
    std::unique_ptr<Versicap> versicap;
    OwnedArray<kv::ChildProcessSlave>   slaves;

    void setupGlobals()
    {
        versicap->initialize();
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
#endif
