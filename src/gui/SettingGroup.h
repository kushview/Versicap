#pragma once

#include "Versicap.h"
#include "RenderContext.h"

namespace vcp {

class SettingGroup : public Component
{
public:
    static String milliSecondValueInt (double value)
    {
        String str (roundToInt (value)); str << " (ms)";
        return str;
    }

    static String noteValue (double value)
    {
        return MidiMessage::getMidiNoteName (
            roundToInt (value),
            true, true, 4);
    }

    SettingGroup (Versicap& vc) 
        : Component(),
          versicap (vc)
    {}
    
    virtual ~SettingGroup() = default;

    virtual void refresh() {}
    virtual void updateSettings() { }
    virtual void stabilizeSettings() { }

    Versicap& getVersicap();
    void paint (Graphics&) override {}

protected:
    Versicap& versicap;
    int labelWidth = 120;
    int settingWidth = 300;
    void layout (Rectangle<int>& r, Component& label, Component& body,
                 int labelPad = 0, 
                 int settingSize = 22, 
                 int spacingBelow = 12)
    {
        r = r.withWidth (jmax (labelWidth + settingWidth, r.getWidth()));
        
        if (settingWidth <= 0)
        {
            auto r2 = r.removeFromTop (settingSize);
            label.setBounds (r2.removeFromLeft (labelWidth));
            body.setBounds (r2);
            r.removeFromTop (spacingBelow);
        }
        else
        {
            auto r2 = r.removeFromTop (settingSize);
            body.setBounds (r2.removeFromRight (settingWidth));
            label.setBounds (r2.withWidth (labelWidth - labelPad));
            r.removeFromTop (spacingBelow);
        }
    }

    void setupSlider (Slider& slider)
    {
        slider.setSliderStyle (Slider::LinearBar);
        slider.setTextBoxIsEditable (false);
    }
};

}
