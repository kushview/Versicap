#pragma once

#include "gui/ContentView.h"
#include "ProjectWatcher.h"

namespace vcp {

class SamplePropertiesContentView : public ContentView,
                                    private ComponentListener
{
public:
    SamplePropertiesContentView (Versicap&);
    virtual ~SamplePropertiesContentView();
    void resized() override;

private:
    ProjectWatcher watcher;
    PropertyPanel panel;

    void componentNameChanged (Component& component) override
    { 
        repaint (0, 0, getWidth(), 24);
    }
};

}
