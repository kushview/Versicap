
#pragma once

#include "JuceHeader.h"

namespace vcp {

class Versicap;
class UnlockForm;

class MainComponent   : public Component
{
public:
    MainComponent (Versicap&);
    ~MainComponent();

    Versicap& getVersicap() { return versicap; }

    void saveContextFile();
    void saveSettings();

    void startRendering();
    void stopRendering();

    void paint (Graphics&) override;
    void resized() override;
    
private:
    Versicap& versicap;
    class Content;
    std::unique_ptr<Content> content;
    Component::SafePointer<UnlockForm> unlock;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

}
