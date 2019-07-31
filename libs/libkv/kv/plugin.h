/*
    This file is part of KV Modules for JUCE

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef KV_PLUGIN_H
#define KV_PLUGIN_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KV_PREFIX "net.kushview."

typedef void* KV_Handle;

typedef struct _KV_Extension {
    /** Unique identifier for this feature */
    const char* ID;

    /** Opaque extension data */
    void* data;
} KV_Extension;

typedef struct _KV_Descriptor {
    /** Globally unique identifier for this plugin */
    const char* ID;

    /** Allocate plugin instance */
    KV_Handle (*instantiate)(const char* bundle_path);

    /** Destroy/clean-up resources */
    void (*destroy)(KV_Handle handle);

    /** Returns extension data */
    const void * (*extension)(const char * identifier);
} KV_Descriptor;

#ifdef __cplusplus
 #define KV_SYMBOL_EXTERN extern "C"
#else
 #define KV_SYMBOL_EXTERN
#endif

#ifdef _WIN32
 #define KV_SYMBOL_EXPORT KV_SYMBOL_EXTERN __declspec(dllexport)
#else
 #define KV_SYMBOL_EXPORT KV_SYMBOL_EXTERN __attribute__((visibility("default")))
#endif

KV_SYMBOL_EXPORT
const KV_Descriptor* kv_descriptor (const uint32_t index);

typedef const KV_Descriptor* (*KV_Descriptor_Function)(const uint32_t);

#ifdef __cplusplus
} /* extern "C" */

#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace kv {

typedef KV_Descriptor PluginDescriptor;
typedef std::initializer_list<std::string> ExtensionList;
typedef const void* (*ExtensionDataFunction)(const char*);
typedef KV_Descriptor_Function DescriptorFunction;

struct PluginDescriptorList : public std::vector<PluginDescriptor> {
    inline ~PluginDescriptorList() {
        for (const auto& desc : *this)
            std::free ((void*) desc.ID);
    }
};

/** @internal */
inline static PluginDescriptorList& plugin_descriptors()
{
    static PluginDescriptorList _descriptors;
    return _descriptors;
}

/** C++ bindings to the C interface */
template<class Instance>
class Plugin
{
public:
    Plugin() = default;
    virtual ~Plugin() = default;

    inline static uint32_t registerDescriptor (const std::string& _identifier,
                                               const ExtensionList& _extensions = {})
    {
        KV_Descriptor desc;
        memset (&desc, 0, sizeof (KV_Descriptor));
        desc.ID             = strdup (_identifier.c_str());
        desc.instantiate    = &PluginType::_instantiate;
        desc.destroy        = &PluginType::_destroy;
        desc.extension      = &PluginType::_extension;

        for (const auto& ext : _extensions)
            extensions()[ext] = Instance::extension (ext.c_str());

        kv::plugin_descriptors().push_back (desc);
        return kv::plugin_descriptors().size() - 1;
    }

private:
    typedef Plugin<Instance> PluginType;
    typedef std::map<std::string, const void*> ExtensionMap;

    inline static ExtensionMap& extensions() {
        static ExtensionMap _extensions;
        return _extensions;
    }

    // Base Plugin
    inline static KV_Handle _instantiate (const char* bundlePath) {
        std::unique_ptr<Instance> instance (Instance::instantiate (bundlePath));
        return static_cast<KV_Handle> (instance.release());
    }

    inline static void _destroy (KV_Handle handle) {
        delete static_cast<Instance*> (handle);
    }

    inline static const void* _extension (const char* identifier) {
        auto iter = extensions().find (identifier);
        return iter != extensions().end() ? (*iter).second : nullptr;
    }
};

}

#define KV_REGISTER_PLUGIN(t, i, e) static uint32_t __t = kv::Plugin<t>::registerDescriptor(i, e); \
    KV_SYMBOL_EXPORT const KV_Descriptor* kv_descriptor (const uint32_t index) { \
        return index < kv::plugin_descriptors().size() \
            ? &kv::plugin_descriptors()[index] : NULL; }

#endif // __cplusplus
#endif // header
