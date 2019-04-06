#pragma once

#include "RenderContext.h"
#include "SettingGroup.h"
#include "Exporter.h"

namespace vcp {

class OutputComponent : public SettingGroup
{
public:
    OutputComponent (Versicap& vc)
        : SettingGroup (vc),
          directory ("OutputPath", File(), false, true, true,
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
        directory.setCurrentFile ({}, false, dontSendNotification);

        for (auto* const exporter : versicap.getExporters())
        {
            auto* button = exporterToggles.add (new TextButton());
            button->setButtonText (exporter->getName());
            button->setClickingTogglesState (true);
            button->setColour (TextButton::buttonOnColourId, kv::Colors::toggleGreen);
            addAndMakeVisible (button);

            auto* label = exporterLabels.add (new Label());
            label->setText (exporter->getDescription(), dontSendNotification);
            addAndMakeVisible (label);
        }

        addAndMakeVisible (renderButton);
        renderButton.setButtonText ("Render");
        renderButton.onClick = [this]() {
            startRendering();
        };
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
        layout (r, nameLabel, name, 0, 22, 4);
        layout (r, directoryLabel, directory, 0, 22, 16);
        for (int i = 0; i < exporterLabels.size(); ++i)
        {
            layout (r,  *exporterToggles [i], *exporterLabels [i],
                    10, 22, 4);
        }

        r.removeFromTop (14);
        renderButton.setBounds (r.removeFromTop (22).reduced (60, 0));
    }

private:
    Label nameLabel;
    Label directoryLabel;

    TextEditor name;
    FilenameComponent directory;

    TextButton renderButton;

    OwnedArray<Label> exporterLabels;
    OwnedArray<TextButton> exporterToggles;

    void startRendering();
};

}
