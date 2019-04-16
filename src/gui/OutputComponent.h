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
        name.setFont (Font (13.f));
        name.setTextToShowWhenEmpty ("Instrument/Patch name",
            findColour (TextEditor::textColourId).darker());

        addAndMakeVisible (directoryLabel);
        directoryLabel.setText ("Data Path", dontSendNotification);
        addAndMakeVisible (directory);
        directory.setCurrentFile (Versicap::getSamplesPath(),
                                  false, dontSendNotification);
        directory.addListener (this);

        addAndMakeVisible (sourceLabel);
        sourceLabel.setText ("Source", dontSendNotification);
        addAndMakeVisible (sourceCombo);
        sourceCombo.addItem ("MIDI Device", 1 + SourceType::MidiDevice);
        sourceCombo.addItem ("Plugin", 1 + SourceType::AudioPlugin);
        sourceCombo.onChange = [this]() { sourceChanged(); };
        sourceCombo.setSelectedId (1, dontSendNotification);

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

        addAndMakeVisible (latencyLabel);
        latencyLabel.setText ("Latency Comp.", dontSendNotification);
        latencyLabel.setTooltip ("Latency compensation of sample recordings");
        addAndMakeVisible (latency);
        latency.setRange (0.0, 9999.0, 1.0);
        setupSlider (latency);
        latency.setTextBoxIsEditable (true);
        latency.textFromValueFunction = [](double value) -> String
        {
            String text = String (roundToInt (value));
            text << " (samples)";
            return text;
        };
        latency.updateText();

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

        sourceCombo.setSelectedId (1 + project.getSourceType(), dontSendNotification);

        formatCombo.setSelectedId (1 + project.getFormatType(), dontSendNotification);
        channelsCombo.getSelectedIdAsValue().referTo (
            project.getPropertyAsValue (Tags::channels));
        bitDepthCombo.getSelectedIdAsValue().referTo (
            project.getPropertyAsValue (Tags::bitDepth));

        latency.getValueObject().referTo (
            project.getPropertyAsValue (Tags::latencyComp));
    }
    
    int getSourceType() const { return sourceCombo.getSelectedId() - 1; }

    void updateFormatParams();

    void stabilizeSettings() override
    {
        updateFormatParams();
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8, 10);
        layout (r, nameLabel, name, 0, 22, 4);
        layout (r, directoryLabel, directory, 0, 22);
        
        layout (r, sourceLabel, sourceCombo);

        layout (r, formatLabel, formatCombo, 0, 22, 4);
        layout (r, channelsLabel, channelsCombo, 0, 22, 4);
        layout (r, bitDepthLabel, bitDepthCombo, 0, 22);
        layout (r, latencyLabel, latency);
    }

private:
    Label nameLabel;
    Label directoryLabel;
    Label sourceLabel;
    Label formatLabel;
    Label bitDepthLabel;
    Label sampleRateLabel;
    Label channelsLabel;
    Label latencyLabel;

    TextEditor name;
    FilenameComponent directory;
    ComboBox sourceCombo;
    ComboBox formatCombo;
    ComboBox bitDepthCombo;
    ComboBox sampleRateCombo;
    ComboBox channelsCombo;
    Slider latency;

    OwnedArray<Label> exporterLabels;
    OwnedArray<TextButton> exporterToggles;

    void sourceChanged();
};

}
