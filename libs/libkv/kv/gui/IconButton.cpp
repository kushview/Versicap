
#include "kv/gui/IconButton.h"

namespace kv {

IconButton::IconButton (const String& buttonName) 
    : Button (buttonName) { }
IconButton::~IconButton() {}

void IconButton::setIcon (Icon newIcon, float reduceSize)
{
    icon = newIcon;
    repaint();
}

void IconButton::paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    getLookAndFeel().drawButtonBackground (g, *this,
        findColour (getToggleState() ? TextButton::buttonOnColourId : TextButton::buttonColourId),
        isMouseOverButton, isButtonDown);
    Rectangle<float> bounds (0.f, 0.f, (float) jmin (getWidth(), getHeight()), 
                                       (float) jmin (getWidth(), getHeight()));
    icon.colour = isEnabled() ? LookAndFeel_KV1::textColor
                              : LookAndFeel_KV1::textColor.darker();
    icon.draw (g, bounds.reduced (iconReduceSize), false);
}

}
