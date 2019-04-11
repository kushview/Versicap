
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

bool Project::writeToFile (const File& file) const
{
    TemporaryFile tempFile (file);
    
    {
        FileOutputStream fo (tempFile.getFile());
        GZIPCompressorOutputStream go (fo);
        objectData.writeToStream (go);
        go.flush();
    }

    return tempFile.overwriteTargetFileWithTemporary();
}

bool Project::loadFile (const File& file)
{
    FileInputStream fi (file);
    GZIPDecompressorInputStream gi (fi);
    auto newData = ValueTree::readFromStream (gi);
    if (newData.isValid() && newData.hasType (Tags::project))
        objectData = newData;
    return objectData == newData;
}

void Project::setMissingProperties()
{
    stabilizePropertyString ("name", "");
    stabilizePropertyString ("dataPath", "");
    
    RenderContext context;
    
    stabilizePropertyPOD ("noteStart",  context.keyStart);
    stabilizePropertyPOD ("noteEnd",    context.keyEnd);
    stabilizePropertyPOD ("noteStep",   context.keyStride);
    
    auto layers = objectData.getOrCreateChildWithName ("layers", nullptr);
    if (layers.getNumChildren() <= 0)
    {
        auto layer = layers.getOrCreateChildWithName ("layer", nullptr);
        layer.setProperty ("enabled", true, nullptr)
             .setProperty ("velocity", 127, nullptr);
    }

    auto plugin = objectData.getOrCreateChildWithName ("plugin", nullptr);
}

}
