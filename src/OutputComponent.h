#pragma once

#include "SettingGroup.h"

namespace vcp {

class OutputComponent : public SettingGroup
{
public:
// const String& name,
//                        const File& currentFile,
//                        bool canEditFilename,
//                        bool isDirectory,
//                        bool isForSaving,
//                        const String& fileBrowserWildcard,
//                        const String& enforcedSuffix,
//                        const String& textWhenNothingSelected);
    OutputComponent()
        : directory ("OutputPath", File(), false, true, true,
                     String(), String(), "Path for rendered files")
    {
        addAndMakeVisible (directoryLabel);
        directoryLabel.setText ("Output Path", dontSendNotification);
        addAndMakeVisible (directory);
    }

    ~OutputComponent()
    {

    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8, 10);
        layout (r, directoryLabel, directory);
    }

private:
    Label directoryLabel;
    FilenameComponent directory;
};

}
