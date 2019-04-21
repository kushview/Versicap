
#pragma once

#include "Versicap.h"

namespace vcp {

class Controller : private Versicap::Listener
{
public:
    Controller (Versicap& vc)
        : versicap (vc)
    {
        versicap.addListener (this);
    }

    virtual ~Controller()
    {
        versicap.removeListener (this);
    }

    virtual void initialize() { }
    virtual void shutdown() { }

    virtual void getCommandInfo (CommandID commandID, ApplicationCommandInfo&) { }
    virtual bool perform (const ApplicationCommandTarget::InvocationInfo&) { return false; }

private:
    Versicap& versicap;
};

}
