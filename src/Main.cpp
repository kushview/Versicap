
#include "JuceHeader.h"
#include "MainComponent.h"
#include "Versicap.h"
#include "UnlockStatus.h"

namespace vcp {

class VersicapApplication  : public JUCEApplication
{
public:
    VersicapApplication() { }

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return false; }

    void initialise (const String& commandLine) override
    {
        versicap.reset (new Versicap());
        
        setupGlobals();

        auto& unlock = versicap->getUnlockStatus();
        unlock.load();

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

    void setupGlobals()
    {
        auto& formats = versicap->getAudioFormats();
        formats.registerBasicFormats();
        versicap->initializeExporters();
        versicap->initializePlugins();
        versicap->initializeAudioDevice();
    }

    void shutdownGui()
    {
        if (mainWindow != nullptr)
        {
            mainWindow->savePersistentData();
            mainWindow = nullptr;
        }

        LookAndFeel::setDefaultLookAndFeel (nullptr);
    }
};

}

START_JUCE_APPLICATION (vcp::VersicapApplication)
