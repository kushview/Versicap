#pragma once

#include "gui/ContentView.h"
#include "ProjectWatcher.h"

namespace vcp {

class SamplePropertiesContentView : public PanelContentView,
                                    private ComponentListener
{
public:
    SamplePropertiesContentView (Versicap&);
    virtual ~SamplePropertiesContentView();

protected:
    void resizeContent (const Rectangle<int>&) override;

private:
    ProjectWatcher watcher;
    PropertyPanel panel;

    void componentNameChanged (Component& component) override
    { 
        repaint (0, 0, getWidth(), 24);
    }
};

}
