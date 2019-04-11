#include "Settings.h"

namespace vcp {

void Settings::setLastProject (const String& path)
{
    if (auto* props = getUserSettings())
        props->setValue ("lastProjectPath", path);
}

File Settings::getLastProject()
{
    String path;
    if (auto* props = getUserSettings())
        path = props->getValue ("lastProjectPath");
    return File::isAbsolutePath (path) ? File (path) : File();
}

}
