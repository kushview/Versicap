
#include "gui/ObjectPropertiesContentView.h"
#include "Versicap.h"

namespace vcp {

ObjectPropertiesContentView::ObjectPropertiesContentView (Versicap& vc)
    : ContentView (vc)
{
    addAndMakeVisible (panel);
    watcher.setProject (vc.getProject());

    watcher.onActiveLayerChanged = [this]()
    {
        panel.clear();
        auto layer = watcher.getProject().getActiveLayer();
        Array<PropertyComponent*> props;
        layer.getProperties (props);
        panel.addProperties (props);
    };

    watcher.onActiveSampleChanged = [this]()
    {
        panel.clear();
        auto sample = watcher.getProject().getActiveSample();
        Array<PropertyComponent*> props;
        sample.getProperties (props);
        panel.addProperties (props);
    };
}

ObjectPropertiesContentView::~ObjectPropertiesContentView()
{

}

void ObjectPropertiesContentView::resized()
{
    panel.setBounds (getLocalBounds());
}

}
