#include "VelocityComponent.h"

VelocityComponent::VelocityComponent()
{
    for (int i = 0; i < 4; ++i)
    {
        auto* const slider = sliders.add (new Slider());
        addAndMakeVisible (slider);
        slider->setRange (0.0, 127.0, 1);

        auto* const toggle = toggles.add (new ToggleButton());
        addAndMakeVisible (toggle);
        toggle->setButtonText (String ("Velocity ") + String (i + 1));
    }
}

VelocityComponent::~VelocityComponent()
{
    sliders.clear();
    toggles.clear();
}

void VelocityComponent::paint (Graphics& g)
{
    g.fillAll (Colours::black);
}

void VelocityComponent::resized()
{
    auto r = getLocalBounds();
    for (int i = 0; i < 4; ++i)
    {
        auto r2 = r.removeFromTop (24);
        toggles[i]->setBounds (r2.removeFromLeft (getWidth() / 2));
        sliders[i]->setBounds (r2);
        r.removeFromTop (10);
    }
}
