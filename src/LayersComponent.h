#pragma once

#include "SettingGroup.h"

namespace vcp {

class LayersComponent : public SettingGroup,
                        public Button::Listener
{
public:
    LayersComponent (Versicap&);
    ~LayersComponent();

    void updateSettings() override;
    void stabilizeSettings() override;
    
    void resized() override;
    void buttonClicked (Button*) override;

private:
    TextButton addLayerButton;
    OwnedArray<TextButton> toggles;
    OwnedArray<Slider> sliders;

    int lastVelocity = 127;
    bool hasLayers();
    void addLayer();
};

}
