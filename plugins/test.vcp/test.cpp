#include "kv/plugin.h"
#include <iostream>

#define KV_DEFINE_PLUGIN_CLASS(t) class t : public kv::Plugin<t>
KV_DEFINE_PLUGIN_CLASS (TestPlugin)
{
public:
    TestPlugin() { }
    
    ~TestPlugin()
    {
        std::clog << "hi there" << std::endl;
    }

    static TestPlugin* instantiate (const char* bundlePath)
    {
        return new TestPlugin();
    }

    static const void* extension (const char* ext)
    {
        if (strcmp (ext, "com.versicap.Plugin") == 0)
        {
            static VersicapDescriptor vcpdesc;
            vcpdesc.testvcp     = &TestPlugin::_testVcp;
            return &vcpdesc;
        }
        
        return nullptr;
    }

private:
    struct VersicapDescriptor
    {
        void (*testvcp)(KV_Handle handle);
    };

    static void _testVcp (KV_Handle handle) {
        std::clog << "testVcp()\n";
    }
};

KV_REGISTER_PLUGIN (TestPlugin, "com.versicap.TestPlugin", {
    "com.versicap.Plugin"
});
