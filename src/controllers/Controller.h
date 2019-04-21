
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

    virtual String getName() const =0;

    virtual void initialize() { }
    virtual void shutdown() { }
    virtual void getCommandInfo (CommandID commandID, ApplicationCommandInfo&) { }
    virtual bool perform (const ApplicationCommandTarget::InvocationInfo&) { return false; }

protected:
    Versicap& versicap;
};

}
