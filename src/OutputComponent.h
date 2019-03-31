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
        addAndMakeVisible (nameLabel);
        nameLabel.setText ("Name", dontSendNotification);
        addAndMakeVisible (name);
        name.setTextToShowWhenEmpty ("Instrument/Patch name",
            findColour (TextEditor::textColourId).darker());

        addAndMakeVisible (directoryLabel);
        directoryLabel.setText ("Output Path", dontSendNotification);
        addAndMakeVisible (directory);
    }

    ~OutputComponent()
    {

    }

    void fillSettings (RenderContext& ctx) override
    {
        ctx.instrumentName  = name.getText().trim();
        ctx.outputPath      = directory.getCurrentFile().getFullPathName();
    }
    
    void updateSettings (const RenderContext& ctx) override
    {
        if (File::isAbsolutePath (ctx.outputPath))
        {
            File file (ctx.outputPath);
            if (file.exists() && ! file.isDirectory())
                file = file.getParentDirectory();
            directory.setCurrentFile (file, true, dontSendNotification);
        }

        name.setText (ctx.instrumentName.trim(), false);
    }
    
    void stabilizeSettings() override
    {

    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8, 10);
        layout (r, nameLabel, name);
        layout (r, directoryLabel, directory);
    }

private:
    Label nameLabel;
    Label directoryLabel;

    TextEditor name;
    FilenameComponent directory;
};

}
