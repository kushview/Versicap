
#include "RenderContext.h"
#include "LayersComponent.h"

namespace vcp {

LayersComponent::LayersComponent (Versicap& vc)
    : SettingGroup (vc)
{
    addAndMakeVisible (addLayerButton);
    addLayerButton.setButtonText ("Add Layer");
    addLayerButton.onClick = std::bind (&LayersComponent::addLayer, this);
}

LayersComponent::~LayersComponent()
{
    sliders.clear();
    toggles.clear();
}

void LayersComponent::buttonClicked (Button* toggle)
{
    auto project = versicap.getProject();
    auto* removebtn = dynamic_cast<TextButton*> (toggle);
    if (removebtn && toggles.contains (removebtn))
    {
        int index = toggles.indexOf (removebtn);
        if (isPositiveAndBelow (index, project.getNumLayers()))
        {
            project.removeLayer (index);
            updateSettings(); // slider / toggle will be deleted so bail
            return;
        }
    }
}

bool LayersComponent::hasLayers()
{
    return sliders.size() > 0;
}

void LayersComponent::updateSettings()
{
    for (auto* b : toggles)
        b->removeListener (this);
    toggles.clearQuick (true);
    sliders.clearQuick (true);

    auto project = versicap.getProject();
    for (int i = 0; i < project.getNumLayers(); ++i)
    {
        auto layer = project.getLayer (i);
        auto* const slider = sliders.add (new Slider());
        addAndMakeVisible (slider);
        slider->setRange (0, 127, 1);
        setupSlider (*slider);
        slider->getValueObject().referTo (
            layer.getPropertyAsValue (Tags::velocity));
        slider->onValueChange = [slider, this]() {
            lastVelocity = jlimit (0, 127, roundToInt (slider->getValue()));
        };
        auto* const toggle = toggles.add (new TextButton());
        addAndMakeVisible (toggle);        
        toggle->setButtonText (String ("Remove"));
        toggle->addListener (this);
    }
    jassert (sliders.size() == toggles.size());
    resized();
}

void LayersComponent::stabilizeSettings() { }

void LayersComponent::resized()
{
    auto r = getLocalBounds().reduced (8, 10);
    addLayerButton.setBounds (r.removeFromTop(22).removeFromLeft (74));
    r.removeFromTop (3);
    for (int i = 0; i < jmin (toggles.size(), sliders.size()); ++i)
        layout (r, *toggles[i], *sliders[i], 10, 22, 6);
}

void LayersComponent::addLayer()
{
    auto project = versicap.getProject();
    auto newLayer = project.addLayer();
    if (isPositiveAndBelow (lastVelocity, 128))
        newLayer.setProperty (Tags::velocity, lastVelocity);
    updateSettings();
}

}
