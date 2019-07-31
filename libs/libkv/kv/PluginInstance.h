
#include <juce/juce.h>
#include "kv/plugin.h"

namespace kv {

class PluginInstance
{
public:
    PluginInstance (KV_Handle _handle) : handle (_handle) { assert (handle != nullptr); }
    virtual ~PluginInstance() = default;

private:
    KV_Handle handle;
};

}
