#pragma once

#include "gui/ContentView.h"
#include "ProjectWatcher.h"

namespace vcp {

class LayerPropertiesContentView : public ContentView,
                                   private ComponentListener
{
public:
    LayerPropertiesContentView (Versicap&);
    virtual ~LayerPropertiesContentView();
    void resized() override;

private:
    ProjectWatcher watcher;
    PropertyPanel panel;

    void refreshCompletePanel();
    void componentNameChanged (Component& component) override
    { 
        repaint (0, 0, getWidth(), 24);
    }
};

}
