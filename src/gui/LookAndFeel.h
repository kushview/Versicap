
#include "JuceHeader.h"

namespace vcp {

class LookAndFeel : public kv::LookAndFeel_KV1
{
public:
    LookAndFeel() : LookAndFeel_KV1()
    {
        setColour (ProgressBar::foregroundColourId, Colours::orange);
    }

    ~LookAndFeel()
    {

    }
};

}
