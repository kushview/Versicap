
#ifndef VCP_STLIB

#include "gui/LookAndFeel.h"
#include "gui/MainWindow.h"

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

private:
    std::unique_ptr<MainWindow> mainWindow;
    vcp::LookAndFeel look;
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
