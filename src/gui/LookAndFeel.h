
#include "JuceHeader.h"

namespace vcp {

class LookAndFeel : public kv::LookAndFeel_KV1
{
public:
    LookAndFeel() : LookAndFeel_KV1()
    {
        // Progress Bar
        setColour (ProgressBar::foregroundColourId, Colours::orange);

        // Labels
        setColour (Label::textWhenEditingColourId,      findColour(TextEditor::textColourId).darker (0.003));

        // Property Component
        setColour (PropertyComponent::labelTextColourId, LookAndFeel::textColor);
        setColour (PropertyComponent::backgroundColourId, LookAndFeel::widgetBackgroundColor.brighter (0.002));
        
       
        setColour (TextPropertyComponent::outlineColourId,      findColour (TextEditor::outlineColourId));
        setColour (TextPropertyComponent::backgroundColourId,   findColour (TextEditor::backgroundColourId));
        setColour (TextPropertyComponent::textColourId,         findColour (TextEditor::textColourId));

        setColour (ToggleButton::textColourId, textColor);
        
        // Boolean property comp
        setColour (BooleanPropertyComponent::backgroundColourId,    findColour (TextEditor::backgroundColourId));
        setColour (BooleanPropertyComponent::outlineColourId,       Colours::black);

        // List Box
        setColour (ListBox::textColourId, textColor);

        setColour (TableHeaderComponent::backgroundColourId, widgetBackgroundColor.darker());
        setColour (TableHeaderComponent::textColourId, Colours::black);
        setColour (TableHeaderComponent::highlightColourId, Colours::orange.withAlpha (0.6f));

        setColour (TextEditor::highlightColourId, Colours::whitesmoke);
        setColour (TextEditor::focusedOutlineColourId, Colours::orange.withAlpha (0.6f));
    }

    ~LookAndFeel() { }

    // combobox
    Font getComboBoxFont (ComboBox& box) override
    { 
        return Font (jmin (12.0f, box.getHeight() * 0.75f));
    }

    // text button
    Font getTextButtonFont (TextButton&, int buttonHeight) override
    {
        return Font (jmin (13.0f, buttonHeight * 0.5f));
    }


    // property component
    void drawPropertyComponentLabel (Graphics& g, int width, int height, 
                                     PropertyComponent& component) override
    {
        g.setColour (component.findColour (PropertyComponent::labelTextColourId)
                    .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.6f));

        // g.setFont (jmin (height, 24) * 0.55f);
        g.setFont (12.f);

        auto r = getPropertyComponentContentPosition (component);

        g.drawFittedText (component.getName(),
                          3, r.getY(), r.getX() - 5, r.getHeight(),
                          Justification::centredLeft, 2);
    }

#if 0
    virtual void drawPropertyPanelSectionHeader (Graphics& g, const String& name, bool isOpen, int width, int height) override;
    virtual void drawPropertyComponentBackground (Graphics&, int width, int height, PropertyComponent&) = 0;
    virtual Rectangle<int> getPropertyComponentContentPosition (PropertyComponent&) = 0;
    virtual int getPropertyPanelSectionHeaderHeight (const String& sectionTitle) = 0;
#endif

    // slider
    Label* createSliderTextBox (Slider& slider) override
    {
        auto l = LookAndFeel_V2::createSliderTextBox (slider);
        l->setFont (Font (13.f));
        return l;
    }
};

}
