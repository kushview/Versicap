
#pragma once

#include "JuceHeader.h"

namespace vcp {

class MainComponent   : public Component
{
public:
    MainComponent();
    ~MainComponent();

    void paint (Graphics&) override;
    void resized() override;

    void startRendering();
    void stopRendering();
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
    class Content;
    std::unique_ptr<Content> content;
};

}
