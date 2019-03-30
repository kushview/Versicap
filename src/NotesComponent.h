#pragma once

#include "SettingGroup.h"

class NotesComponent : public SettingGroup
{
public:
    NotesComponent()
    {
        addAndMakeVisible (keyStartLabel);
        keyStartLabel.setText ("Start Note", dontSendNotification);
        addAndMakeVisible (keyStart);
        keyStart.setRange (0, 127, 1);
        keyStart.textFromValueFunction = SettingGroup::noteValue;
        setupSlider (keyStart);

        addAndMakeVisible (keyEndLabel);
        keyEndLabel.setText ("Note End", dontSendNotification);
        addAndMakeVisible (keyEnd);
        keyEnd.setRange (0, 127, 1);
        keyEnd.textFromValueFunction = SettingGroup::noteValue;
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
    
    void fillSettings (RenderContext& ctx) override 
    { 
        ctx.keyStart    = roundToInt (keyStart.getValue());
        ctx.keyEnd      = roundToInt (keyEnd.getValue());
        ctx.keyStride   = roundToInt (keyStride.getValue());
    }

    void updateSettings (const RenderContext& ctx) override
    {
        keyStart.setValue ((double) ctx.keyStart, dontSendNotification);
        keyEnd.setValue ((double) ctx.keyEnd, dontSendNotification);
        keyStride.setValue ((double) ctx.keyStride, dontSendNotification);
    }

    void stabilizeSettings() override
    {
        keyStart.updateText();
        keyEnd.updateText();
        if (keyEnd.getValue() <= keyStart.getValue())
            keyEnd.setValue (keyStart.getValue() + 1.0, dontSendNotification);
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
