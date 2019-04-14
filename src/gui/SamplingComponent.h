#pragma once

#include "gui/SettingGroup.h"

namespace vcp {

class SamplingComponent : public SettingGroup
{
public:
    SamplingComponent (Versicap& vc)
        : SettingGroup (vc)
    {   
        addAndMakeVisible (baseNameLabel);
        baseNameLabel.setText ("Base Name", dontSendNotification);
        addAndMakeVisible (baseName);
        // baseName.setText ("Sample");

        addAndMakeVisible (noteLengthLabel);
        noteLengthLabel.setText ("Note Length", dontSendNotification);
        addAndMakeVisible (noteLength);
        noteLength.setRange (1, 10000, 1);
        noteLength.textFromValueFunction = SettingGroup::milliSecondValueInt;
        setupSlider (noteLength);

        addAndMakeVisible (tailLengthLabel);
        tailLengthLabel.setText ("Tail Length", dontSendNotification);
        addAndMakeVisible (tailLength);
        tailLength.setRange (1, 10000, 1);
        tailLength.textFromValueFunction = SettingGroup::milliSecondValueInt;
        setupSlider (tailLength);
    }

    ~SamplingComponent() = default;

    void updateSettings() override
    {
        auto project = versicap.getProject();
        baseName.getTextValue().referTo (project.getPropertyAsValue (Tags::baseName));
        noteLength.getValueObject().referTo (project.getPropertyAsValue (Tags::noteLength));
        tailLength.getValueObject().referTo (project.getPropertyAsValue (Tags::tailLength));
    }
    
    void stabilizeSettings() override { }
    
    void resized() override
    {
        auto r = getLocalBounds().reduced (8, 10);
        layout (r, baseNameLabel, baseName);
        layout (r, noteLengthLabel, noteLength);
        layout (r, tailLengthLabel, tailLength);
    }

private:
    Label noteLengthLabel;
    Label tailLengthLabel;
    Label baseNameLabel;
    Slider noteLength;
    Slider tailLength;
    TextEditor baseName;
};

}
