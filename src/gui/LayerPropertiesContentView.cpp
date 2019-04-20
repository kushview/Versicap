
#include "gui/LayerPropertiesContentView.h"
#include "Versicap.h"

namespace vcp {

LayerPropertiesContentView::LayerPropertiesContentView (Versicap& vc)
    : ContentView (vc)
{
    setName ("Layer");
    addAndMakeVisible (panel);
    watcher.setProject (vc.getProject());

    addComponentListener (this);

    watcher.onActiveLayerChanged = [this]()
    {
        panel.clear();
        auto layer = watcher.getProject().getActiveLayer();
        Array<PropertyComponent*> props;
        layer.getProperties (props);
        panel.addProperties (props);
    };
}

LayerPropertiesContentView::~LayerPropertiesContentView() { }

void LayerPropertiesContentView::resized()
{
    panel.setBounds (getLocalBounds());
}

}
