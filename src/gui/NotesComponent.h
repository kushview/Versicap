#pragma once

#include "gui/SettingGroup.h"

namespace vcp {

class NotesComponent : public SettingGroup
{
public:
    NotesComponent (Versicap& vc)
        : SettingGroup (vc)
    {
        addAndMakeVisible (keyStartLabel);
        keyStartLabel.setText ("Note Start", dontSendNotification);
        addAndMakeVisible (keyStart);
        keyStart.setRange (0, 127, 1);
        keyStart.textFromValueFunction = SettingGroup::noteValue;
        keyStart.onValueChange = [this]() { stabilizeKeyStart(); };
        setupSlider (keyStart);

        addAndMakeVisible (keyEndLabel);
        keyEndLabel.setText ("Note End", dontSendNotification);
        addAndMakeVisible (keyEnd);
        keyEnd.setRange (0, 127, 1);
        keyEnd.textFromValueFunction = SettingGroup::noteValue;
        keyEnd.onValueChange = [this]() { stabilizeKeyEnd(); };
        setupSlider (keyEnd);

        addAndMakeVisible (keyStrideLabel);
        keyStrideLabel.setText ("Key Stride", dontSendNotification);
        addAndMakeVisible (keyStride);
        keyStride.setRange (1, 12, 1);
        setupSlider (keyStride);
    }

    ~NotesComponent()
    {

    }

    void updateSettings() override
    {
        auto project = versicap.getProject();
        keyStart.getValueObject().referTo (project.getPropertyAsValue (Tags::noteStart));
        keyEnd.getValueObject().referTo (project.getPropertyAsValue (Tags::noteEnd));
        keyStride.getValueObject().referTo (project.getPropertyAsValue (Tags::noteStep));
    }

    void stabilizeSettings() override
    {
        if (! stabilizeKeyStart())
            stabilizeKeyEnd();
        keyStart.updateText();
        keyEnd.updateText();        
    }

    bool stabilizeKeyStart()
    {
        bool shouldConstrain = keyEnd.getValue() <= keyStart.getValue();
        if (shouldConstrain)
            keyEnd.setValue (keyStart.getValue() + 1.0, dontSendNotification);
        return shouldConstrain;
    }

    bool stabilizeKeyEnd()
    {
        bool shouldConstrain = keyStart.getValue() >= keyEnd.getValue();
        if (shouldConstrain)
            keyStart.setValue (keyEnd.getValue() - 1.0, dontSendNotification);
        return shouldConstrain;
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8, 10);
        layout (r, keyStartLabel, keyStart);
        layout (r, keyEndLabel, keyEnd);
        layout (r, keyStrideLabel, keyStride);
    }

private:
    Label keyStartLabel;
    Label keyEndLabel;
    Label keyStrideLabel;
    Slider keyStart;
    Slider keyEnd;
    Slider keyStride;
};

}
