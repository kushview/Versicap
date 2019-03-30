#pragma once

#include "SettingGroup.h"

class VelocityComponent : public SettingGroup
{
public:
    VelocityComponent();
    ~VelocityComponent();

    void fillSettings (RenderContext&) override;
    void updateSettings (const RenderContext&) override;
    void stabilizeSettings() override;
    
    void resized() override;

private:
    OwnedArray<TextButton> toggles;
    OwnedArray<Slider> sliders;
};
