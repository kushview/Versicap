#pragma once

#pragma once

#include "JuceHeader.h"

class SamplingComponent : public Component
{
public:
    SamplingComponent()
    {
        addAndMakeVisible (noteLengthLabel);
        noteLengthLabel.setText ("Note Length", dontSendNotification);
        addAndMakeVisible (noteLength);
        noteLength.setRange (0.5, 10.0, 0.01);

        addAndMakeVisible (tailLengthLabel);
        tailLengthLabel.setText ("Tail Length", dontSendNotification);
        addAndMakeVisible (tailLength);
        tailLength.setRange (0.5, 10.0, 0.01);

        addAndMakeVisible (baseNameLabel);
        baseNameLabel.setText ("Base Name", dontSendNotification);
        addAndMakeVisible (baseName);
        baseName.setText ("Sample");
    }

    ~SamplingComponent()
    {

    }

    void paint (Graphics&) override
    {

    }
    
    void resized() override
    {
        auto r = getLocalBounds();
        auto r2 = r.removeFromTop (22);
        baseNameLabel.setBounds (r2.removeFromLeft (100));
        baseNameLabel.setText ("Base Name", dontSendNotification);
        baseName.setBounds (r2);
        r.removeFromTop (4);
        r2 = r.removeFromTop (22);
        noteLengthLabel.setBounds (r2.removeFromLeft (100));
        noteLengthLabel.setText ("Note Length", dontSendNotification);
        noteLength.setBounds (r2);
        r.removeFromTop (4);
        r2 = r.removeFromTop (22);
        tailLengthLabel.setBounds (r2.removeFromLeft (100));
        tailLengthLabel.setText ("Tail Length", dontSendNotification);
        tailLength.setBounds (r2);
    }

private:
    Label noteLengthLabel;
    Label tailLengthLabel;
    Label baseNameLabel;
    Slider noteLength;
    Slider tailLength;
    TextEditor baseName;
};
