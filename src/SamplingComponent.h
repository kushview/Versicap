#pragma once

#include "SettingGroup.h"

namespace vcp {

class SamplingComponent : public SettingGroup
{
public:
    SamplingComponent()
    {
        settingWidth = 300;
        
        addAndMakeVisible (baseNameLabel);
        baseNameLabel.setText ("Base Name", dontSendNotification);
        addAndMakeVisible (baseName);
        baseName.setText ("Sample");

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

    void fillSettings (RenderContext& ctx) override 
    { 
        ctx.baseName    = baseName.getText();
        ctx.noteLength  = roundToInt (noteLength.getValue());
        ctx.tailLength  = roundToInt (tailLength.getValue());
    }

    void updateSettings (const RenderContext& ctx) override 
    {
        baseName.setText (ctx.baseName, false);
        noteLength.setValue ((double) ctx.noteLength, dontSendNotification);
        tailLength.setValue ((double) ctx.tailLength, dontSendNotification);
    }
    
    void stabilizeSettings() override {}
    
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
