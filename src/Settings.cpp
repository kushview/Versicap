#include "Settings.h"

namespace vcp {

const char* Settings::lastProjectPathKey    = "lastProjectPath";

Settings::Settings()
{
    PropertiesFile::Options opts;
    opts.applicationName     = "Versicap";
    opts.filenameSuffix      = "conf";
    opts.osxLibrarySubFolder = "Application Support";
    opts.storageFormat       = PropertiesFile::storeAsCompressedBinary;

   #if JUCE_DEBUG
    opts.applicationName << "Debug";
    opts.storageFormat       = PropertiesFile::storeAsXML;
   #endif

   #if JUCE_LINUX
    opts.folderName          = ".config/versicap";
   #else
    opts.folderName          = "Versicap";
   #endif

    setStorageParameters (opts);
}

void Settings::setLastProject (const String& path)
{
    if (auto* props = getUserSettings())
        props->setValue (lastProjectPathKey, path);
}

File Settings::getLastProject()
{
    String path;
    if (auto* props = getUserSettings())
        path = props->getValue (lastProjectPathKey);
    return File::isAbsolutePath (path) ? File (path) : File();
}

}
