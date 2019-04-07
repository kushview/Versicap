#pragma once

#include "JuceHeader.h"

namespace vcp {

class PluginPicker : public Component
{
public:
    PluginPicker()
    {
        addAndMakeVisible (pluginButton);
        pluginButton.setButtonText (emptyText);
        pluginButton.onClick = [this]() {
            if (onChoose)
                onChoose();
        };

        addAndMakeVisible (editorButton);
        editorButton.setButtonText ("E");
        editorButton.onClick = [this]() {
            if (onEditor)
                onEditor();
        };

        addAndMakeVisible (closeButton);
        closeButton.setButtonText ("X");
        closeButton.onClick = [this]() {
            if (onClose)
                onClose();
        };

        setSize (200, 22);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        closeButton.setBounds (r.removeFromRight (24));
        editorButton.setBounds (r.removeFromRight (24));
        pluginButton.setBounds (r);
    }

    ~PluginPicker() = default;

    void setPluginName (const String& name)
    {
        pluginButton.setButtonText (name.isNotEmpty() ? name : emptyText);
    }

    std::function<void()> onChoose;
    std::function<void()> onEditor;
    std::function<void()> onClose;
    
private:
    String emptyText { "Choose a plugin..." };
    TextButton pluginButton;
    TextButton editorButton;
    TextButton closeButton;
};

}
