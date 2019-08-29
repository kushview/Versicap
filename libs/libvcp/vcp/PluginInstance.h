
#pragma once

#include "vcp/plugin.h"

namespace vcp {

class PluginInstance final
{
public:
    PluginInstance (const VCPDescriptor* _desc, VCPHandle _handle)
        : desc (*_desc),
          handle (_handle)
    { 
        jassert (handle != nullptr);
    }

    virtual ~PluginInstance()
    {
        desc.destroy (handle);
        handle = nullptr;
    }

private:
    const VCPDescriptor desc;
    VCPHandle handle;
};

}
