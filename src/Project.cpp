
#include "PluginManager.h"
#include "Project.h"
#include "RenderContext.h"
#include "Tags.h"

namespace vcp {

Project::Project()
    : ObjectModel (Tags::project)
{
    setMissingProperties();
}

Project::~Project() {}

// AudioProcessor* Project::createAudioProcessor (PluginManager& plugins) const
// {
//     String errorMessage;
//     auto plugin = objectData.getChildWithName (Tags::plugin);
//     auto& list = plugins.getKnownPlugins();
//     if (const auto* const desc = list.getTypeForIdentifierString (plugin.getProperty (Tags::identifier))
//         return plugins.createAudioPlugin (*desc, errorMessage);
//     return nullptr;
// }

bool Project::getPluginDescription (PluginManager& plugins, PluginDescription& desc) const
{
    auto plugin = objectData.getChildWithName (Tags::plugin);
    auto& list = plugins.getKnownPlugins();
    if (const auto* const type = list.getTypeForIdentifierString (plugin.getProperty (Tags::identifier)))
    {
        desc = *type;
        return true;
    }

    return false;
}

void Project::setPluginDescription (const PluginDescription& desc)
{
    auto plugin = objectData.getChildWithName (Tags::plugin);
    plugin.setProperty (Tags::format, desc.pluginFormatName, nullptr)
          .setProperty (Tags::fileOrId, desc.fileOrIdentifier, nullptr)
          .setProperty (Tags::identifier, desc.createIdentifierString(), nullptr);
}

void Project::updatePluginState (AudioProcessor& processor)
{
    auto plugin = objectData.getChildWithName (Tags::plugin);
    MemoryBlock mb;
    processor.getStateInformation (mb);
    
    if (mb.getSize() > 0)
    {
        MemoryOutputStream mo;
        {
            GZIPCompressorOutputStream gz (mo);
            gz.write (mb.getData(), mb.getSize());
        }

        plugin.setProperty (Tags::state, mo.getMemoryBlock(), nullptr);
    }
}

void Project::applyPluginState (AudioProcessor& processor) const
{
    const auto plugin = objectData.getChildWithName (Tags::plugin);
    if (const auto* state = plugin.getProperty(Tags::state).getBinaryData())
    {
        MemoryInputStream mi (*state, false);
        GZIPDecompressorInputStream gz (mi);
        MemoryBlock mb;
        gz.readIntoMemoryBlock (mb);
        processor.setStateInformation (mb.getData(), static_cast<int> (mb.getSize()));
    }
}

bool Project::writeToFile (const File& file) const
{
    TemporaryFile tempFile (file);
    
    {
        FileOutputStream fo (tempFile.getFile());
        {
            GZIPCompressorOutputStream go (fo);
            objectData.writeToStream (go);
        }
    }

    return tempFile.overwriteTargetFileWithTemporary();
}

bool Project::loadFile (const File& file)
{
    if (! file.existsAsFile())
        return false;
    FileInputStream fi (file);
    GZIPDecompressorInputStream gi (fi);
    auto newData = ValueTree::readFromStream (gi);
    if (newData.isValid() && newData.hasType (Tags::project))
        objectData = newData;
    DBG(newData.toXmlString());
    return objectData == newData;
}

void Project::setMissingProperties()
{
    stabilizePropertyString (Tags::name, "");
    stabilizePropertyString (Tags::dataPath, "");
    
    RenderContext context;
    
    stabilizePropertyPOD (Tags::noteStart,  context.keyStart);
    stabilizePropertyPOD (Tags::noteEnd,    context.keyEnd);
    stabilizePropertyPOD (Tags::noteStep,   context.keyStride);
    
    auto layers = objectData.getOrCreateChildWithName (Tags::layers, nullptr);
    if (layers.getNumChildren() <= 0)
    {
        auto layer = layers.getOrCreateChildWithName (Tags::layer, nullptr);
        layer.setProperty (Tags::enabled, true, nullptr)
             .setProperty (Tags::velocity, 127, nullptr);
    }

    auto plugin = objectData.getOrCreateChildWithName (Tags::plugin, nullptr);
}

}
