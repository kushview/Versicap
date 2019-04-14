
#pragma once

namespace vcp {

class Versicap;

class Controller
{
public:
    Controller (Versicap& vc)
        : versicap (vc) { }

private:
    Versicap& versicap;
};

}
