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
        
        addAndMakeVisible (noteEnd);
        noteEnd.setSliderStyle (Slider::IncDecButtons);
        noteEnd.setRange (0, 127, 1);
        noteEnd.setTextBoxStyle (noteEnd.getTextBoxPosition(), false, 30, noteEnd.getTextBoxHeight());
        noteEnd.textFromValueFunction = Util::noteValue;
        noteEnd.updateText();
        
        addAndMakeVisible (stepLabel);
        stepLabel.setFont (Font (12.f));
        stepLabel.setText ("Step:", dontSendNotification);
        addAndMakeVisible (noteStep);
        noteStep.setSliderStyle (Slider::IncDecButtons);
        noteStep.setRange (1, 12, 1);
        noteStep.setTextBoxStyle (noteStep.getTextBoxPosition(), false, 24, noteStep.getTextBoxHeight());

        addAndMakeVisible (applyButton);
        applyButton.setButtonText ("Apply");
        applyButton.setConnectedEdges (Button::ConnectedOnRight);
        applyButton.setEnabled (false);
        applyButton.onClick = [this]() { applyChanges(); };

        addAndMakeVisible (resetButton);
        resetButton.setButtonText ("Reset");
        resetButton.setConnectedEdges (Button::ConnectedOnLeft);
        resetButton.setEnabled (false);
        resetButton.onClick = [this]() {
            stabilizeComponents();
            setChanged (false);
        };

        setChanged (false);

        bind();
    }

    ~NoteParams() = default;

    int getRequiredWidth() { return (64 * 3) + 34 + 34 + 4 + 34 + 34 + 2; }

    void resized() override
    {
        auto r2 = getLocalBounds().withWidth (jmax (getRequiredWidth(), getWidth()));
        resetButton.setBounds (r2.removeFromRight (34));
        applyButton.setBounds (r2.removeFromRight (34));
        r2.removeFromRight (2);
        noteStep.setBounds (r2.removeFromRight (64));
        stepLabel.setBounds (r2.removeFromRight (34));
        r2.removeFromRight (4);
        noteEnd.setBounds (r2.removeFromRight (64));
        noteStart.setBounds (r2.removeFromRight (64));
        notesButton.setBounds (r2.removeFromRight (34));
    }

    void setProject (const Project& newProject)
    {
        project = newProject;
        stabilizeComponents();
        setChanged (false);
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
    TextButton applyButton;
    TextButton resetButton;
    bool changedFlag = false;
    
    void bind()
    {
        noteStart.onValueChange = [this]() {
            adjustNoteEnd();
            setChanged (true);
        };

        noteEnd.onValueChange = [this]() {
            adjustNoteStart();
            setChanged (true);
        };

        noteStep.onValueChange = [this]() { setChanged (true); };
    }

    void unbind()
    {
        noteStart.onValueChange = nullptr;
        noteEnd.onValueChange = nullptr;
        noteStep.onValueChange = nullptr;
    }

    void stabilizeComponents()
    {
        unbind();
        noteStart.setValue (project.getProperty (Tags::noteStart), dontSendNotification);
        noteEnd.setValue (project.getProperty (Tags::noteEnd), dontSendNotification);
        noteStep.setValue (project.getProperty (Tags::noteStep), dontSendNotification);
        bind();
    }

    void applyChanges()
    {
        project.setProperty (Tags::noteStart, roundToInt (noteStart.getValue()));
        project.setProperty (Tags::noteEnd, roundToInt (noteEnd.getValue()));
        project.setProperty (Tags::noteStep, roundToInt (noteStep.getValue()));
        project.rebuildSampleList();
        setChanged (false);
    }

    void setChanged (bool changed)
    {
        changedFlag = changed;
        resetButton.setEnabled (changedFlag);
        applyButton.setEnabled (changedFlag);
        resized();
    }

    void menuResult (int result)
    {
        bool handled = true;
        
        switch (result)
        {
            case 37:
                noteStart.setValue (36);
                noteEnd.setValue (72);
                break;
            case 49:
                noteStart.setValue (36);
                noteEnd.setValue (84);
                break;
            case 61:
                noteStart.setValue (36);
                noteEnd.setValue (96);
                break;
            case 76:
                noteStart.setValue (36);
                noteEnd.setValue (108);
                break;
            case 88:
                noteStart.setValue (21);
                noteEnd.setValue (108);
                break;
            case 100:
                noteStart.setValue (0);
                noteEnd.setValue (127);
                break;
            default: handled = false;
                break;
        };

        if (handled)
            setChanged (handled);
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