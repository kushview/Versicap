
#include <juce/juce.h>
#include "kv/plugin.h"

namespace kv {

class PluginInstance
{
public:
    PluginInstance (const KV_Descriptor* _desc, KV_Handle _handle)
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
    const KV_Descriptor desc;
    KV_Handle handle;
};

}
