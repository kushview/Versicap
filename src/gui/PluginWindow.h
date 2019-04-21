
#pragma once

#include "JuceHeader.h"

namespace vcp {

//=============================================================================
class PluginWindow : public DocumentWindow
{
public:
    PluginWindow (std::unique_ptr<PluginWindow>& o, AudioProcessorEditor* ed)
        : DocumentWindow (ed->getAudioProcessor()->getName(),
                          Colours::black, DocumentWindow::closeButton),
          owner (o)
    {
        owner.reset (this);
        editor.reset (ed);
        setUsingNativeTitleBar (true);
        setContentNonOwned (editor.get(), true);
        setResizable (editor->isResizable(), false);
        centreWithSize (getWidth(), getHeight());
        setVisible (true);
    }

    ~PluginWindow()
    {
        if (editor)
        {
            if (auto* proc = editor->getAudioProcessor())
                proc->editorBeingDeleted (editor.get());
            editor.reset();
        }
    }

    void closeButtonPressed() override
    {
        owner.reset();
    }

private:
    std::unique_ptr<PluginWindow>& owner;
    std::unique_ptr<AudioProcessorEditor> editor;
};

}
