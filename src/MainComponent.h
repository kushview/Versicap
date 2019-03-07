
#pragma once

#include "JuceHeader.h"

class MainComponent   : public Component
{
public:
    MainComponent();
    ~MainComponent();

    void paint (Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
    class Content;
    std::unique_ptr<Content> content;
};
