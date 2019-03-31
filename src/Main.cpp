
#include "JuceHeader.h"
#include "MainComponent.h"
#include "Versicap.h"

namespace vcp {

class VersicapApplication  : public JUCEApplication
{
public:
    VersicapApplication() {}

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return false; }

    void initialise (const String& commandLine) override
    {
        versicap.reset (new Versicap());
        
        setupGlobals();

        look.setColour (Slider::backgroundColourId, kv::LookAndFeel_KV1::widgetBackgroundColor.darker());
        LookAndFeel::setDefaultLookAndFeel (&look);
        mainWindow.reset (new MainWindow (getApplicationName(), *versicap));
    }

    void setupGlobals()
    {
        auto& formats = versicap->getAudioFormats();
        formats.registerBasicFormats();
        versicap->initializePlugins();
        versicap->initializeAudioDevice();
    }

    void shutdown() override
    {
        mainWindow = nullptr;
        LookAndFeel::setDefaultLookAndFeel (nullptr);

        versicap->saveSettings();

        auto& devices = versicap->getDeviceManager();
        devices.removeAudioCallback (versicap.get());
        devices.removeMidiInputCallback (String(), versicap.get());
        
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
                                         DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setBackgroundColour (kv::LookAndFeel_KV1::widgetBackgroundColor.darker());
            setContentOwned (new MainComponent (vc), true);

            setResizable (false, false);
            constrain.setMinimumSize (440, 300);
            constrain.setMaximumSize (440, 300);
            setConstrainer (&constrain);
            centreWithSize (getWidth(), getHeight());

            setVisible (true);
        }

        ~MainWindow()
        {
            setConstrainer (nullptr);
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
        ComponentBoundsConstrainer constrain;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
    kv::LookAndFeel_KV1 look;
    std::unique_ptr<Versicap> versicap;
};

}

START_JUCE_APPLICATION (vcp::VersicapApplication)
