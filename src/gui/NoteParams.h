#pragma once

#include "Project.h"
#include "Utils.h"

namespace vcp {

class NoteParams : public Component
{
public:
    NoteParams()
    {
        addAndMakeVisible (noteStart);
        noteStart.setSliderStyle (Slider::IncDecButtons);
        noteStart.setRange (0, 127, 1);
        noteStart.setTextBoxStyle (noteStart.getTextBoxPosition(), false, 30, noteStart.getTextBoxHeight());
        noteStart.textFromValueFunction = Util::noteValue;
        noteStart.updateText();
        noteStart.onValueChange = [this]() { adjustNoteEnd(); };

        addAndMakeVisible (noteEnd);
        noteEnd.setSliderStyle (Slider::IncDecButtons);
        noteEnd.setRange (0, 127, 1);
        noteEnd.setTextBoxStyle (noteEnd.getTextBoxPosition(), false, 30, noteEnd.getTextBoxHeight());
        noteEnd.textFromValueFunction = Util::noteValue;
        noteEnd.updateText();
        noteEnd.onValueChange = [this]() { adjustNoteStart(); };

        addAndMakeVisible (stepLabel);
        stepLabel.setFont (Font (12.f));
        stepLabel.setText ("Step:", dontSendNotification);
        addAndMakeVisible (noteStep);
        noteStep.setSliderStyle (Slider::IncDecButtons);
        noteStep.setRange (1, 12, 1);
        noteStep.setTextBoxStyle (noteStep.getTextBoxPosition(), false, 24, noteStep.getTextBoxHeight());
    }

    ~NoteParams() = default;

    int getRequiredWidth() { return (64 * 3) + 34 + 4; }

    void resized() override
    {
        auto r2 = getLocalBounds().withWidth (jmax (getRequiredWidth(), getWidth()));
        noteStep.setBounds (r2.removeFromRight (64));
        stepLabel.setBounds (r2.removeFromRight (34));
        r2.removeFromRight (4);
        noteEnd.setBounds (r2.removeFromRight (64));
        noteStart.setBounds (r2.removeFromRight (64));
    }

    void setProject (const Project& newProject)
    {
        project = newProject;
        noteStart.getValueObject().referTo (project.getPropertyAsValue (Tags::noteStart));
        noteEnd.getValueObject().referTo (project.getPropertyAsValue (Tags::noteEnd));
        noteStep.getValueObject().referTo (project.getPropertyAsValue (Tags::noteStep));
    }

private:
    Project project;
    Label stepLabel;
    Slider noteStart, noteEnd, noteStep;

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