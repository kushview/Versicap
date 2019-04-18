
#include "gui/ObjectPropertiesContentView.h"
#include "Versicap.h"

namespace vcp {

ObjectPropertiesContentView::ObjectPropertiesContentView (Versicap& vc)
    : PanelContentView (vc)
{
    setName ("Properties");
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

        setName ("Layer Properties");
    };

    watcher.onActiveSampleChanged = [this]()
    {
        panel.clear();
        auto sample = watcher.getProject().getActiveSample();
        Array<PropertyComponent*> props;
        sample.getProperties (props);
        panel.addProperties (props);

        setName ("Sample Properties");
    };
}

ObjectPropertiesContentView::~ObjectPropertiesContentView()
{

}

void ObjectPropertiesContentView::resizeContent (const Rectangle<int>& area)
{
    panel.setBounds (area);
}

}
