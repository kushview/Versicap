
#include "RenderContext.h"
#include "LayersComponent.h"

namespace vcp {

LayersComponent::LayersComponent()
{
    for (int i = 0; i < 4; ++i)
    {
        auto* const slider = sliders.add (new Slider());
        addAndMakeVisible (slider);
        slider->setRange (0.0, 127.0, 1);
        setupSlider (*slider);
        
        auto* const toggle = toggles.add (new TextButton());
        addAndMakeVisible (toggle);
        if (i == 0)
            toggle->setToggleState (true, dontSendNotification);
        toggle->setClickingTogglesState (true);
        toggle->setColour (TextButton::buttonOnColourId, Colours::greenyellow);
        toggle->setButtonText (String ("Layer ") + String (i + 1));
        toggle->addListener (this);
    }
}

LayersComponent::~LayersComponent()
{
    sliders.clear();
    toggles.clear();
}

void LayersComponent::buttonClicked (Button* toggle)
{
    if (! hasLayers())
        toggle->setToggleState (true, dontSendNotification);
    stabilizeSettings();
}

bool LayersComponent::hasLayers()
{
    bool anythingToggled = false;
    for (int i = 0; i < 4; ++i)
    {
        if (toggles[i]->getToggleState())
        {
            anythingToggled = true;
            break;
        }
    }
    return anythingToggled;
}

void LayersComponent::fillSettings (RenderContext& ctx)
{
    jassert (hasLayers());

    for (int i = 0; i < 4; ++i)
    {
        ctx.layerEnabled[i]     = toggles[i]->getToggleState();
        ctx.layerVelocities[i]  = roundToInt (sliders[i]->getValue());
    }
}

void LayersComponent::updateSettings (const RenderContext& ctx)
{
    for (int i = 0; i < 4; ++i)
    {
        toggles[i]->setToggleState (ctx.layerEnabled[i], dontSendNotification);
        sliders[i]->setValue ((double) ctx.layerVelocities[i], dontSendNotification);
    }
}

void LayersComponent::stabilizeSettings()
{
    for (int i = 0; i < 4; ++i)
    {
        sliders[i]->setEnabled (toggles[i]->getToggleState());
    }
}

void LayersComponent::resized()
{
    auto r = getLocalBounds().reduced (8, 10);
    for (int i = 0; i < 4; ++i)
        layout (r, *toggles[i], *sliders[i], 10);
}

}
