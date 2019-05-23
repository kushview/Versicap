
#pragma once

#include "gui/ContentComponent.h"
#include "Versicap.h"

namespace vcp {

class Versicap;
class UnlockForm;

class MainComponent   : public ContentComponent,
                        private Versicap::Listener,
                        private ChangeListener
{
public:
    MainComponent (Versicap&);
    ~MainComponent();

    Versicap& getVersicap() { return versicap; }

    void saveSettings();

    void displayObject (const ValueTree&) override;
    void getState (String&) override;
    void applyState (const String& state) override;
    void stabilizeProject() override;

    void paint (Graphics&) override;
    void resized() override;

private:
    Versicap& versicap;
    class Content;
    std::unique_ptr<Content> content;

    void projectChanged() override;
    void renderWillStart() override;
    void renderStarted() override;
    void renderWillStop() override;
    void renderProgress (double, const String&) override;
    void exportStarted() override;
    void exportFinished() override;
    void exportProgress (double, const String&) override;

    void changeListenerCallback (ChangeBroadcaster*) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

}
