/*
    This file is part of Versicap

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

#ifndef VCP_PLUGIN_H
#define VCP_PLUGIN_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VCP_PREFIX "net.kushview.versicap"

typedef void* VCPHandle;

typedef struct _VCPExtension {
    /** Unique identifier for this feature */
    const char* ID;

    /** Opaque extension data */
    void* data;
} VCPExtension;

typedef struct _VCPDescriptor {
    /** Globally unique identifier for this plugin */
    const char* ID;

    /** Allocate plugin instance */
    VCPHandle (*instantiate)(const char* bundle_path);

    /** Destroy/clean-up resources */
    void (*destroy)(VCPHandle handle);

    /** Returns extension data */
    const void * (*extension)(const char * identifier);
} VCPDescriptor;

#ifdef __cplusplus
 #define VCP_SYMBOL_EXTERN extern "C"
#else
 #define VCP_SYMBOL_EXTERN
#endif

#ifdef _WIN32
 #define VCP_SYMBOL_EXPORT VCP_SYMBOL_EXTERN __declspec(dllexport)
#else
 #define VCP_SYMBOL_EXPORT VCP_SYMBOL_EXTERN __attribute__((visibility("default")))
#endif

VCP_SYMBOL_EXPORT
const VCPDescriptor* vcp_descriptor (const uint32_t index);

typedef const VCPDescriptor* (*VCPDescriptorFunction)(const uint32_t);
typedef const void* (*VCPExtensionFunction)(const char*);

#ifdef __cplusplus
} /* extern "C" */

#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace vcp {

typedef VCPDescriptor PluginDescriptor;
typedef VCPDescriptorFunction DescriptorFunction;
typedef std::initializer_list<std::string> ExtensionList;
typedef VCPExtensionFunction ExtensionFunction;

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
template<class I>
class Plugin
{
public:
    Plugin() = default;
    virtual ~Plugin() = default;

    inline static uint32_t registerDescriptor (const std::string& _identifier,
                                               const ExtensionList& _extensions = {})
    {
        VCPDescriptor desc;
        memset (&desc, 0, sizeof (VCPDescriptor));
        desc.ID             = strdup (_identifier.c_str());
        desc.instantiate    = &P::_instantiate;
        desc.destroy        = &P::_destroy;
        desc.extension      = &P::_extension;

        for (const auto& ext : _extensions)
            extensions()[ext] = I::extension (ext.c_str());

        vcp::plugin_descriptors().push_back (desc);
        return vcp::plugin_descriptors().size() - 1;
    }

private:
    typedef Plugin<I> P;
    typedef std::map<std::string, const void*> ExtensionMap;

    inline static ExtensionMap& extensions() {
        static ExtensionMap _extensions;
        return _extensions;
    }

    // Base Plugin
    inline static VCPHandle _instantiate (const char* bundlePath) {
        std::unique_ptr<I> instance (I::instantiate (bundlePath));
        return static_cast<VCPHandle> (instance.release());
    }

    inline static void _destroy (VCPHandle handle) {
        delete static_cast<I*> (handle);
    }

    inline static const void* _extension (const char* identifier) {
        auto iter = extensions().find (identifier);
        return iter != extensions().end() ? (*iter).second : nullptr;
    }
};

}

#define VCP_PLUGIN_CLASS(t) class t : public vcp::Plugin<t> 
#define VCP_REGISTER_PLUGIN(t, i, e) static uint32_t __t = vcp::Plugin<t>::registerDescriptor(i, e); \
    VCP_SYMBOL_EXPORT const VCPDescriptor* vcp_descriptor (const uint32_t index) { \
        return index < vcp::plugin_descriptors().size() \
            ? &vcp::plugin_descriptors()[index] : NULL; }

#endif // __cplusplus
#endif // header
