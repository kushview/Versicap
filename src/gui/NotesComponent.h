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
        keyStart.setTextBoxIsEditable (true);

        addAndMakeVisible (keyEndLabel);
        keyEndLabel.setText ("Note End", dontSendNotification);
        addAndMakeVisible (keyEnd);
        keyEnd.setRange (0, 127, 1);
        keyEnd.textFromValueFunction = SettingGroup::noteValue;
        keyEnd.onValueChange = [this]() { stabilizeKeyEnd(); };
        setupSlider (keyEnd);
        keyEnd.setTextBoxIsEditable (true);

        addAndMakeVisible (keyStrideLabel);
        keyStrideLabel.setText ("Note Step", dontSendNotification);
        addAndMakeVisible (keyStride);
        keyStride.setRange (1, 12, 1);
        setupSlider (keyStride);

        addAndMakeVisible (noteLengthLabel);
        noteLengthLabel.setText ("Note On Len.", dontSendNotification);
        addAndMakeVisible (noteLength);
        noteLength.setRange (1, 10000, 1);
        noteLength.textFromValueFunction = SettingGroup::milliSecondValueInt;
        setupSlider (noteLength);

        addAndMakeVisible (tailLengthLabel);
        tailLengthLabel.setText ("Note Off Len.", dontSendNotification);
        addAndMakeVisible (tailLength);
        tailLength.setRange (1, 10000, 1);
        tailLength.textFromValueFunction = SettingGroup::milliSecondValueInt;
        setupSlider (tailLength);
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
        noteLength.getValueObject().referTo (project.getPropertyAsValue (Tags::noteLength));
        tailLength.getValueObject().referTo (project.getPropertyAsValue (Tags::tailLength));
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
        bool shouldConstrain = keyEnd.getValue() < keyStart.getValue();
        if (shouldConstrain)
            keyEnd.setValue (keyStart.getValue(), dontSendNotification);
        return shouldConstrain;
    }

    bool stabilizeKeyEnd()
    {
        bool shouldConstrain = keyStart.getValue() > keyEnd.getValue();
        if (shouldConstrain)
            keyStart.setValue (keyEnd.getValue(), dontSendNotification);
        return shouldConstrain;
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8, 10);
        layout (r, keyStartLabel, keyStart, 0, 22, 4);
        layout (r, keyEndLabel, keyEnd, 0, 22, 4);
        layout (r, keyStrideLabel, keyStride);

        layout (r, noteLengthLabel, noteLength, 0, 22, 4);
        layout (r, tailLengthLabel, tailLength);
    }

private:
    Label keyStartLabel;
    Label keyEndLabel;
    Label keyStrideLabel;
    Label noteLengthLabel;
    Label tailLengthLabel;

    Slider keyStart;
    Slider keyEnd;
    Slider keyStride;
    Slider noteLength;
    Slider tailLength;
};

}
