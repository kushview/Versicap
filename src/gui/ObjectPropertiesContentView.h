#pragma once

#include "gui/ContentView.h"
#include "ProjectWatcher.h"

namespace vcp {

class ObjectPropertiesContentView : public ContentView
{
public:
    ObjectPropertiesContentView (Versicap&);
    virtual ~ObjectPropertiesContentView();

    void resized() override;

private:
    ProjectWatcher watcher;
    PropertyPanel panel;
};

}
