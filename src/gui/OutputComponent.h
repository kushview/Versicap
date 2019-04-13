#pragma once

#include "gui/SettingGroup.h"
#include "Exporter.h"

namespace vcp {

class OutputComponent : public SettingGroup,
                        public FilenameComponentListener
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
        directoryLabel.setText ("Data Path", dontSendNotification);
        addAndMakeVisible (directory);
        directory.setCurrentFile (Versicap::getSamplesPath(),
                                  false, dontSendNotification);
        directory.addListener (this);

        addAndMakeVisible (formatLabel);
        formatLabel.setText ("Format", dontSendNotification);
        addAndMakeVisible (formatCombo);
        for (int i = 0; i < FormatType::NumTypes; ++i)
            formatCombo.addItem (FormatType::getName(i), i + 1);
        formatCombo.onChange = [this]()
        {
            versicap.getProject().setProperty (Tags::format,
                FormatType::getSlug (formatCombo.getSelectedId() - 1)); 
            updateFormatParams();
        };
        formatCombo.setSelectedItemIndex (0, dontSendNotification);
        
        addAndMakeVisible (channelsLabel);
        channelsLabel.setText ("Channels", dontSendNotification);
        addAndMakeVisible (channelsCombo);
        channelsCombo.addItem ("Mono", 1);
        channelsCombo.addItem ("Stereo", 2);
        channelsCombo.setSelectedId (2, dontSendNotification);

        addAndMakeVisible (bitDepthLabel);
        bitDepthLabel.setText ("Bit Depth", dontSendNotification);
        addAndMakeVisible (bitDepthCombo);

        addAndMakeVisible (sampleRateLabel);
        sampleRateLabel.setText ("Sample Rate", dontSendNotification);
        addAndMakeVisible (sampleRateCombo);

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

        updateFormatParams();
    }

    ~OutputComponent()
    {

    }
    
    void filenameComponentChanged (FilenameComponent*) override
    {
        auto project = versicap.getProject();
        project.setProperty (Tags::dataPath, directory.getCurrentFile().getFullPathName());
    }

    void updateSettings() override
    {
        auto project = versicap.getProject();
        name.getTextValue().referTo (project.getPropertyAsValue (Tags::name));
        const auto dataPath = project.getProperty (Tags::dataPath).toString();
        if (File::isAbsolutePath (dataPath))
        {
            File file (dataPath);
            if (file.exists() && ! file.isDirectory())
                file = file.getParentDirectory();
            directory.setCurrentFile (file, true, dontSendNotification);
        }
        else
        {
            directory.setCurrentFile (File(), false, dontSendNotification);
        }

        formatCombo.setSelectedId (1 + project.getFormatType(), dontSendNotification);
        channelsCombo.getSelectedIdAsValue().referTo (
            project.getPropertyAsValue (Tags::channels));
        bitDepthCombo.getSelectedIdAsValue().referTo (
            project.getPropertyAsValue (Tags::bitDepth));
    }
    
    void updateFormatParams();

    void stabilizeSettings() override
    {
        updateFormatParams();
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8, 10);
        layout (r, nameLabel, name, 0, 22, 4);
        layout (r, directoryLabel, directory, 0, 22, 16);
        
        layout (r, formatLabel, formatCombo, 0, 22, 4);
        layout (r, channelsLabel, channelsCombo, 0, 22, 4);
        layout (r, bitDepthLabel, bitDepthCombo, 0, 22, 4);
        // layout (r, sampleRateLabel, sampleRateCombo);

        // for (int i = 0; i < exporterLabels.size(); ++i)
        // {
        //     layout (r,  *exporterToggles [i], *exporterLabels [i],
        //             10, 22, 4);
        // }
    }

private:
    Label nameLabel;
    Label directoryLabel;
    Label formatLabel;
    Label bitDepthLabel;
    Label sampleRateLabel;
    Label channelsLabel;

    TextEditor name;
    FilenameComponent directory;
    ComboBox formatCombo;
    ComboBox bitDepthCombo;
    ComboBox sampleRateCombo;
    ComboBox channelsCombo;

    OwnedArray<Label> exporterLabels;
    OwnedArray<TextButton> exporterToggles;
};

}
