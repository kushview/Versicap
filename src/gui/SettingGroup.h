#pragma once

#include "Versicap.h"
#include "RenderContext.h"
#include "gui/ContentView.h"

namespace vcp {

class SettingGroup : public ContentView
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
        : ContentView (vc)
    { }
    
    virtual ~SettingGroup() = default;

    virtual void refresh() {}
    virtual void updateSettings() { }
    virtual void stabilizeSettings() { }

    Versicap& getVersicap();
    void paint (Graphics&) override {}

protected:
    int labelWidth = 80;
    int settingWidth = 100;
    void layout (Rectangle<int>& r, Component& label, Component& body,
                 int labelPad = 0, 
                 int settingSize = 22, 
                 int spacingBelow = 12)
    {
        if (auto* const _label = dynamic_cast<Label*> (&label))
            _label->setFont (Font (12.f));
        
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
            label.setBounds (r2.removeFromLeft (labelWidth));
            r2.removeFromLeft (4);
            body.setBounds (r2);
            
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
