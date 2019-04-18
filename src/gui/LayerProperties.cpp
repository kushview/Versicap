
#include "Project.h"

namespace vcp {

void Layer::getProperties (Array<PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (getPropertyAsValue (Tags::name), 
        "Name", 100, false, true));
    props.add (new SliderPropertyComponent (getPropertyAsValue (Tags::velocity, true),
        "Velocity", 0, 127, 1, 1, false));
    props.add (new SliderPropertyComponent (getPropertyAsValue (Tags::noteLength, true),
        "Note Length", 0, 20000, 1, 1, false));
    props.add (new SliderPropertyComponent (getPropertyAsValue (Tags::tailLength, true),
        "Tail Length", 0, 20000, 1, 1, false));
}

}
