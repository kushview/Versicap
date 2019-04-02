#pragma once

#include "SettingGroup.h"

namespace vcp {

class LayersComponent : public SettingGroup,
                        public Button::Listener
{
public:
    LayersComponent (Versicap&);
    ~LayersComponent();

    void fillSettings (RenderContext&) override;
    void updateSettings (const RenderContext&) override;
    void stabilizeSettings() override;
    
    void resized() override;
    void buttonClicked (Button*) override;

private:
    OwnedArray<TextButton> toggles;
    OwnedArray<Slider> sliders;

    bool hasLayers();
};

}
