#include "VelocityComponent.h"

VelocityComponent::VelocityComponent()
{
    for (int i = 0; i < 4; ++i)
    {
        auto* const slider = sliders.add (new Slider());
        addAndMakeVisible (slider);
        slider->setRange (0.0, 127.0, 1);

        auto* const toggle = toggles.add (new TextButton());
        addAndMakeVisible (toggle);
        toggle->setClickingTogglesState (true);
        toggle->setColour (TextButton::buttonOnColourId, Colours::greenyellow);
        toggle->setButtonText (String ("Layer ") + String (i + 1));
    }
}

VelocityComponent::~VelocityComponent()
{
    sliders.clear();
    toggles.clear();
}

void VelocityComponent::paint (Graphics& g)
{

}

void VelocityComponent::resized()
{
    auto r = getLocalBounds();
    for (int i = 0; i < 4; ++i)
    {
        auto r2 = r.removeFromTop (24);
        toggles[i]->setBounds (r2.removeFromLeft (100));
        sliders[i]->setBounds (r2);
        r.removeFromTop (4);
    }
}
