
#include "Project.h"

namespace vcp {

//=========================================================================
String Sample::getUuidString() const {   return getProperty (Tags::uuid).toString(); }

Uuid Sample::getUuid() const
{
    const Uuid uuid (getUuidString());
    return uuid;
}

String Sample::getFileName () const
{
    const Project project (objectData.getParent().getParent());
    const Layer layer (project.findLayer (getProperty(Tags::layer).toString()));

    String name     = getProperty (Tags::name);
    String noteName = getNoteName();
    String noteNo   = String(getNote()).paddedLeft ('0', 3);
    String layerNo  = String(project.indexOf (layer)).paddedLeft ('0', 3);

    StringArray tokens;
    tokens.addArray ({ layerNo, noteNo, name, noteName });
    tokens.removeEmptyStrings (true);
    return tokens.joinIntoString("_");
}

bool Sample::isForLayer (const Layer& layer) const
{
    const Uuid sid (getProperty (Tags::layer).toString());
    const Uuid lid (layer.getUuid());
    return !sid.isNull() && !lid.isNull() && sid == lid;
}

File Sample::getFile() const
{
    auto filename = getProperty(Tags::file).toString();
    if (filename.isEmpty())
        return File();
    Project project (objectData.getParent().getParent());
    auto path = project.getProperty (Tags::dataPath).toString();
    
    if (File::isAbsolutePath (path))
    {
        File file (path);
        return file.getChildFile ("samples")
                   .getChildFile (filename);
    }

    return File();
}

void Sample::getProperties (Array<PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (getPropertyAsValue (Tags::name), 
        "Name", 100, false, true));
}

double Sample::getSampleRate() const    { return getProperty (Tags::sampleRate); }
double Sample::getTotalTime() const     { return getProperty (Tags::length); }
double Sample::getStartTime() const     { return getProperty (Tags::timeIn); }
double Sample::getEndTime() const       { return getProperty (Tags::timeOut); }
double Sample::getLength() const        { return getEndTime() - getStartTime(); }

}
