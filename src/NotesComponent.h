#pragma once

#include "JuceHeader.h"

class NotesComponent : public Component
{
public:
    NotesComponent()
    {
        addAndMakeVisible (keyStartLabel);
        keyStartLabel.setText ("Key Start", dontSendNotification);
        addAndMakeVisible (keyStart);
        keyStart.setRange (0, 127, 1);

        addAndMakeVisible (keyEndLabel);
        keyEndLabel.setText ("Key End", dontSendNotification);
        addAndMakeVisible (keyEnd);
        keyEnd.setRange (0, 127, 1);

        addAndMakeVisible (keyStrideLabel);
        keyStrideLabel.setText ("Key Stride", dontSendNotification);
        addAndMakeVisible (keyStride);
        keyStride.setRange (1, 12, 1);
    }

    ~NotesComponent()
    {

    }

    void paint (Graphics&) override
    {

    }
    
    void resized() override
    {
        auto r  = getLocalBounds();
        auto r2 = r.removeFromTop (22);
        keyStartLabel.setBounds (r2.removeFromLeft (100));
        keyStart.setBounds (r2);
        r.removeFromTop (4);
        r2 = r.removeFromTop (22);
        keyEndLabel.setBounds (r2.removeFromLeft (100));
        keyEnd.setBounds (r2);
        r.removeFromTop (4);
        r2 = r.removeFromTop (22);
        keyStrideLabel.setBounds (r2.removeFromLeft (100));
        keyStride.setBounds (r2);
    }

private:
    Label keyStartLabel;
    Label keyEndLabel;
    Label keyStrideLabel;
    Slider keyStart;
    Slider keyEnd;
    Slider keyStride;
};
