#pragma once

#include "JuceHeader.h"

namespace kv {

class IconButton : public Button
{
public:
    IconButton (const String& buttonName = String());
    virtual ~IconButton();
    void setIcon (Icon newIcon, float reduceSize = 4.f);

protected:
    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override;

private:
    Icon icon;
    float iconReduceSize = 4.f;
};

}
