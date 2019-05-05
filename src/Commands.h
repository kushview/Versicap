#pragma once

namespace vcp {
namespace Commands {
enum ID {
    projectOpen         = 0x00001000,
    projectSave,
    projectSaveAs,
    projectNew,
    projectRecord,
    projectShowDataPath,
    projectExport,

    layerRecord         = 0x00002000,
    
    showAbout           = 0x00005000,
    showLicenseManagement,

    checkForUpdates     = 0x00006000
};
}}
