#include "vcp/plugin.h"
#include <iostream>

VCP_PLUGIN_CLASS (TestPlugin)
{
public:
    TestPlugin() {}
    
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
        void (*testvcp)(VCPHandle handle);
    };

    static void _testVcp (VCPHandle handle) {
        std::clog << "testVcp()\n";
    }
};

VCP_REGISTER_PLUGIN (TestPlugin, "com.versicap.TestPlugin", {});
