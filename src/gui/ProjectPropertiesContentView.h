
#pragma once

#include "gui/ContentView.h"
#include "Project.h"
#include "Versicap.h"

namespace vcp {

class ProjectPropertiesContentView : public ContentView
{
public:
    ProjectPropertiesContentView (Versicap& vc)
        : ContentView (vc)
    {
        addAndMakeVisible (panel);
        project = vc.getProject();
        updateProperties();
    }

    ~ProjectPropertiesContentView() {}

    void resized() override
    {
        panel.setBounds (getLocalBounds());
    }

private:
    Project project;
    PropertyPanel panel;

    void updateProperties()
    {
        Array<PropertyComponent*> props;
        project.getProperties (props);
        panel.clear();
        panel.addProperties (props);
    }
};

}
