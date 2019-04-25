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

    void setProject (const Project& project) { watcher.setProject (project); }
    XmlElement* getOpennessState() const { return panel.getOpennessState(); }
    void restoreOpennessState (const XmlElement& xml) { panel.restoreOpennessState (xml); }

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
