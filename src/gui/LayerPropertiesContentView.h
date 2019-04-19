#pragma once

#include "gui/ContentView.h"
#include "ProjectWatcher.h"

namespace vcp {

class LayerPropertiesContentView : public PanelContentView,
                                   private ComponentListener
{
public:
    LayerPropertiesContentView (Versicap&);
    virtual ~LayerPropertiesContentView();

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
