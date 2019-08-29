
#pragma once

#include "JuceHeader.h"

namespace kv {

class Spinner  : public Component,
                 private Timer
{
public:
    Spinner()                       { startTimer (1000 / 50); }
    ~Spinner() noexcept = default;

    void paint (Graphics& g) override
    {
        getLookAndFeel().drawSpinningWaitAnimation (
            g, Colours::darkgrey, 0, 0, getWidth(), getHeight());
    }

private:
    void timerCallback() override   { repaint(); }
};

}
