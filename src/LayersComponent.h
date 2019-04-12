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
    OwnedArray<TextButton> toggles;
    OwnedArray<Slider> sliders;

    bool hasLayers();
};

}
