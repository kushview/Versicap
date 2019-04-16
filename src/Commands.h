#pragma once

namespace vcp {
namespace Commands {
enum ID {

    projectOpen         = 0x00001000,
    projectSave,
    projectSaveAs,
    projectNew,
    
    showAbout           = 0x00005000,

    checkForUpdates     = 0x00006000
};
}}
