#pragma once

#include "SettingGroup.h"

namespace vcp {

class LoopingComponent : public SettingGroup
{
public:
    LoopingComponent()
    {
        addAndMakeVisible (loopTypeLabel);
        loopTypeLabel.setText ("Loop Mode", dontSendNotification);
        addAndMakeVisible (loopType);
        loopType.addItem ("Off", 1);
        loopType.addItem ("Forwards", 2);
        loopType.addItem ("Alternating", 3);
        loopType.addItem ("Reverse", 4);
        loopType.onChange = [this]() { stabilizeSettings(); };
        loopType.setSelectedId (1);

        addAndMakeVisible (loopStartLabel);
        loopStartLabel.setText ("Loop Start", dontSendNotification);
        addAndMakeVisible (loopStart);
        loopStart.setRange (0, 10000, 1);
        loopStart.textFromValueFunction = SettingGroup::milliSecondValueInt;
        setupSlider (loopStart);

        addAndMakeVisible (loopEndLabel);
        loopEndLabel.setText ("Loop End", dontSendNotification);
        addAndMakeVisible (loopEnd);
        loopEnd.setRange (0, 10000, 1);
        loopEnd.textFromValueFunction = SettingGroup::milliSecondValueInt;
        setupSlider (loopEnd);

        addAndMakeVisible (crossfadeLengthLabel);
        crossfadeLengthLabel.setText ("Crossfade Length", dontSendNotification);
        addAndMakeVisible (crossfadeLength);
        crossfadeLength.setRange (0, 5000, 1);
        crossfadeLength.textFromValueFunction = SettingGroup::milliSecondValueInt;
        setupSlider (crossfadeLength);
    }

    ~LoopingComponent()
    {

    }

    void fillSettings (RenderContext& ctx) override
    {
        ctx.loopMode        = loopType.getSelectedId() - 1;
        ctx.loopStart       = roundToInt (loopStart.getValue());
        ctx.loopEnd         = roundToInt (loopEnd.getValue());
        ctx.crossfadeLength = roundToInt (crossfadeLength.getValue());
    }

    void updateSettings (const RenderContext& ctx) override
    {
        loopType.setSelectedId (1 + ctx.loopMode, dontSendNotification);
        loopStart.setValue (static_cast<double> (ctx.loopStart), dontSendNotification);
        loopEnd.setValue (static_cast<double> (ctx.loopEnd), dontSendNotification);
        crossfadeLength.setValue (static_cast<double> (ctx.crossfadeLength), dontSendNotification);     
    }

    void stabilizeSettings() override
    {
        loopStart.setEnabled (loopType.getSelectedId() != 1);
        loopEnd.setEnabled (loopType.getSelectedId() != 1);
        crossfadeLength.setEnabled (loopType.getSelectedId() == 2);
    }
    
    void resized() override
    {
        auto r = getLocalBounds().reduced (8, 10);
        layout (r, loopTypeLabel, loopType);
        layout (r, loopStartLabel, loopStart);
        layout (r, loopEndLabel, loopEnd);
        layout (r, crossfadeLengthLabel, crossfadeLength);
    }

private:
    Label loopTypeLabel;
    Label loopStartLabel;
    Label loopEndLabel;
    Label crossfadeLengthLabel;

    ComboBox loopType;
    Slider loopStart;
    Slider loopEnd;
    Slider crossfadeLength;
};

}
