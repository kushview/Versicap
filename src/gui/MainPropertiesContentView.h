#pragma once

#include "gui/ContentView.h"
#include "ProjectWatcher.h"

namespace vcp {

class MainPropertiesContentView : public ContentView,
                                   private ComponentListener
{
public:
    MainPropertiesContentView (Versicap&);
    virtual ~MainPropertiesContentView();
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
