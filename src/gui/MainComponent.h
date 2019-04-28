
#pragma once

#include "gui/ContentComponent.h"
#include "Versicap.h"

namespace vcp {

class Versicap;
class UnlockForm;

class MainComponent   : public ContentComponent,
                        public Versicap::Listener,
                        public ChangeListener
{
public:
    MainComponent (Versicap&);
    ~MainComponent();

    Versicap& getVersicap() { return versicap; }

    void saveSettings();

    void startRendering();
    void stopRendering();

    void displayObject (const ValueTree&) override;
    void getState (String&) override;
    void applyState (const String& state) override;

    void paint (Graphics&) override;
    void resized() override;
    
    void projectChanged() override;
    void renderWillStart() override;
    void renderStarted() override;
    void renderWillStop() override;

    void exportStarted() override;
    void exportFinished() override;
    void exportProgress (double, const String&) override;

    void changeListenerCallback (ChangeBroadcaster*) override;

private:
    Versicap& versicap;
    class Content;
    std::unique_ptr<Content> content;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

}
