#pragma once

#include "JuceHeader.h"

namespace vcp {

class Settings :  public ApplicationProperties
{
public:
    Settings()
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

    ~Settings() { }

    void setLastProject (const String& path);
    File getLastProject();
};

}
