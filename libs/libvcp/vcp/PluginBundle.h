
#pragma once

#include "JuceHeader.h"
#include "vcp/plugin.h"
#include "vcp/PluginInstance.h"

namespace vcp {

class PluginBundle
{
public:
    PluginBundle (const String& _bundlePath)
        : bundlePath (_bundlePath)
    { }

    ~PluginBundle()
    {
        close();
    }

    static const String binaryExtension()
    {
       #if JUCE_MAC
        return ".dylib";
       #elif JUCE_WINDOWS
        return ".dll";
       #elif JUCE_LINUX
        return ".so";
       #endif
    }

    bool isOpen() const { return libraryOpen; }

    bool open()
    {
        close();
        if (! libraryOpen)
        {
            File libraryFile (bundlePath);
            String fileName = libraryFile.getFileNameWithoutExtension(); 
            fileName << binaryExtension();
            libraryFile = libraryFile.getChildFile (fileName);
            libraryOpen = library.open (libraryFile.getFullPathName().toRawUTF8());
            if (libraryOpen)
            {
                uint32 i = 0;
                for (;;)
                {
                    if (const auto* desc = getDescriptor (i))
                        descriptors.add (desc);
                    else
                        break;
                    ++i;
                }
            }
        }

        return libraryOpen;
    }

    void close()
    {
        if (! libraryOpen)
            return;
        libraryOpen = false;
        descriptors.clearQuick();
        library.close();
    }

    PluginInstance* createInstance (const String& identifier)
    {
        if (! isOpen())
            return nullptr;

        for (const auto* desc : descriptors)
            if (strcmp (identifier.toRawUTF8(), desc->ID) == 0)
                return createInstanceForDescriptor (desc);

        return nullptr;
    }

protected:
    const VCPDescriptor* getDescriptor (const uint32 index)
    {
        if (descriptorFunction == nullptr)
            descriptorFunction = (DescriptorFunction) library.getFunction ("vcp_descriptor");
        return descriptorFunction != nullptr ? descriptorFunction (index) : nullptr;
    }

private:
    const String bundlePath;
    DynamicLibrary library;
    bool libraryOpen;
    Array<const VCPDescriptor*> descriptors;
    DescriptorFunction descriptorFunction = nullptr;

    PluginInstance* createInstanceForDescriptor (const VCPDescriptor* const desc)
    {
        jassert (desc != nullptr && desc->instantiate != nullptr && desc->extension != nullptr);
        if (desc == nullptr || desc->instantiate == nullptr || desc->extension == nullptr)
            return nullptr;
        if (VCPHandle handle = desc->instantiate (bundlePath.toRawUTF8()))
            return new PluginInstance (desc, handle);

        return nullptr;
    }
};

}
