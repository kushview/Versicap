
#include "Project.h"
#include "Utils.h"

namespace vcp {

class MillisecondIntSliderPropertyComponent : public SliderPropertyComponent
{
public:
    MillisecondIntSliderPropertyComponent (const Value& valueToControl,
                                           const String& propertyName,
                                           double rangeMin, 
                                           double rangeMax)
        : SliderPropertyComponent (valueToControl, propertyName,
                                   rangeMin, rangeMax, 1.0, 1.0, false)
    {
        slider.textFromValueFunction = Util::milliSecondValueInt;
        slider.valueFromTextFunction = [this](const String& text) -> double {
            return text.getDoubleValue() * 1000.0;
        };

        slider.updateText();
    }
};

class MidiProgramPropertyComponent : public SliderPropertyComponent
{
public:
    MidiProgramPropertyComponent (const Value& valueToControl,
                                  const String& propertyName)
        : SliderPropertyComponent (valueToControl, propertyName,
                                   -1, 127, 1.0, 1.0, false)
    {
        slider.textFromValueFunction = [this](double value) -> String {
            auto intVal = 1 + roundToInt (value);
            return intVal > 0 ? String(intVal) : "none";
        };

        slider.valueFromTextFunction = [this](const String& text) -> double {
            return static_cast<double> (jlimit (-1, 127, text.getIntValue() - 1));
        };

        slider.updateText();
    }
};

void SampleSet::getProperties (Array<PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (getPropertyAsValue (Tags::name), 
        "Name", 100, false, true));
    props.add (new SliderPropertyComponent (getPropertyAsValue (Tags::velocity, true),
        "Velocity", 0, 127, 1, 1, false));
    props.add (new MillisecondIntSliderPropertyComponent (getPropertyAsValue (Tags::noteLength, true),
        "Note Length", 0, 20000));
    props.add (new MillisecondIntSliderPropertyComponent (getPropertyAsValue (Tags::tailLength, true),
        "Tail Length", 0, 20000));
    props.add (new SliderPropertyComponent (getPropertyAsValue ("midiChannel", true),
        "Channel", 1, 16, 1, 1, false));
    props.add (new MidiProgramPropertyComponent (getPropertyAsValue ("midiProgram", true),
        "Program"));
}

}
