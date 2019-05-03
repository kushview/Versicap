#pragma once

#include "Project.h"
#include "Utils.h"

namespace vcp {

class NoteParams : public Component
{
public:
    NoteParams()
    {
        addAndMakeVisible (notesButton);
        notesButton.setButtonText ("Notes");
        notesButton.setTriggeredOnMouseDown (true);
        notesButton.onClick = [this]()
        {
            PopupMenu menu;
            Array<int> keys = { 37, 49, 61, 76, 88 };
            for (const auto& nkeys : keys)
                menu.addItem (nkeys, String(nkeys) + String (" Keys"));
            menu.addSeparator();
            menu.addItem (100, "All");
            menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&notesButton),
                ModalCallbackFunction::forComponent (&NoteParams::menuResult, this));
        };

        addAndMakeVisible (noteStart);
        noteStart.setSliderStyle (Slider::IncDecButtons);
        noteStart.setRange (0, 127, 1);
        noteStart.setTextBoxStyle (noteStart.getTextBoxPosition(), false, 30, noteStart.getTextBoxHeight());
        noteStart.textFromValueFunction = Util::noteValue;
        noteStart.updateText();
        noteStart.onValueChange = [this]() { 
            adjustNoteEnd(); 
            project.rebuildSampleList();
            DBG(project.getSamples().getValueTree().toXmlString());
        };

        addAndMakeVisible (noteEnd);
        noteEnd.setSliderStyle (Slider::IncDecButtons);
        noteEnd.setRange (0, 127, 1);
        noteEnd.setTextBoxStyle (noteEnd.getTextBoxPosition(), false, 30, noteEnd.getTextBoxHeight());
        noteEnd.textFromValueFunction = Util::noteValue;
        noteEnd.updateText();
        noteEnd.onValueChange = [this]() {
            adjustNoteStart();
            project.rebuildSampleList();
        };

        addAndMakeVisible (stepLabel);
        stepLabel.setFont (Font (12.f));
        stepLabel.setText ("Step:", dontSendNotification);
        addAndMakeVisible (noteStep);
        noteStep.setSliderStyle (Slider::IncDecButtons);
        noteStep.setRange (1, 12, 1);
        noteStep.setTextBoxStyle (noteStep.getTextBoxPosition(), false, 24, noteStep.getTextBoxHeight());
    }

    ~NoteParams() = default;

    int getRequiredWidth() { return (64 * 3) + 40 + 34 + 4; }

    void resized() override
    {
        auto r2 = getLocalBounds().withWidth (jmax (getRequiredWidth(), getWidth()));
        noteStep.setBounds (r2.removeFromRight (64));
        stepLabel.setBounds (r2.removeFromRight (34));
        r2.removeFromRight (4);
        noteEnd.setBounds (r2.removeFromRight (64));
        noteStart.setBounds (r2.removeFromRight (64));
        notesButton.setBounds (r2.removeFromRight (40));
    }

    void setProject (const Project& newProject)
    {
        project = newProject;
        noteStart.getValueObject().referTo (project.getPropertyAsValue (Tags::noteStart));
        noteEnd.getValueObject().referTo (project.getPropertyAsValue (Tags::noteEnd));
        noteStep.getValueObject().referTo (project.getPropertyAsValue (Tags::noteStep));
    }

    static void menuResult (int result, NoteParams* params) {
        if (result > 0 && params != nullptr)
            params->menuResult (result);
    }

private:
    Project project;
    Label stepLabel;
    Slider noteStart, noteEnd, noteStep;
    TextButton notesButton;

    void menuResult (int result)
    {
        bool handled = true;
        switch (result)
        {
            case 37:
                project.setNotes (36, 72);
                break;
            case 49:
                project.setNotes (36, 84);
                break;
            case 61:
                project.setNotes (36, 96);
                break;
            case 76:
                project.setNotes (36, 108);
                break;
            case 88:
                project.setNotes (21, 108);
                break;
            case 100:
                project.setNotes (0, 127);
                break;
            default: handled = false;
                break;
        };

        if (handled)
        {
            project.rebuildSampleList();
            DBG(project.getSamples().getValueTree().toXmlString());    
        }
    }

    bool adjustNoteEnd()
    {
        if (noteStart.getValue() > noteEnd.getValue())
        {
            noteEnd.setValue (noteStart.getValue());
            return true;
        }

        return false;
    }

    bool adjustNoteStart()
    {
        if (noteEnd.getValue() < noteStart.getValue())
        {
            noteStart.setValue (noteEnd.getValue());
            return true;
        }

        return false;
    }

    void clampNotes()
    {
        if (! adjustNoteEnd())
            adjustNoteStart();    
    }
};

}