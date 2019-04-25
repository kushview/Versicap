
#include "gui/LayerPropertiesContentView.h"
#include "Versicap.h"

namespace vcp {

LayerPropertiesContentView::LayerPropertiesContentView (Versicap& vc)
    : ContentView (vc)
{
    setName ("Properties");
    addAndMakeVisible (panel);
    watcher.setProject (vc.getProject());
    addComponentListener (this);
    watcher.onChanged = watcher.onActiveLayerChanged = watcher.onActiveSampleChanged = 
        std::bind (&LayerPropertiesContentView::refreshCompletePanel, this);
    refreshCompletePanel();
}

LayerPropertiesContentView::~LayerPropertiesContentView() { }

void LayerPropertiesContentView::refreshCompletePanel()
{
    auto project = watcher.getProject();
    Array<PropertyComponent*> props;
    std::unique_ptr<XmlElement> xml;
    if (! panel.isEmpty())
        xml.reset (panel.getOpennessState());

    panel.clear();
    project.getProperties (versicap, props);
    project.getRecordingProperties (versicap, props);
    project.getDevicesProperties (versicap, props);
    panel.addSection ("Project", props);
    
    props.clearQuick();
    auto layer = project.getActiveLayer();
    layer.getProperties (props);
    panel.addSection ("Layer", props);

    props.clearQuick();
    auto sample = project.getActiveSample();
    sample.getProperties (props);
    panel.addSection ("Sample", props);

    if (xml)
        panel.restoreOpennessState (*xml);
}

void LayerPropertiesContentView::resized()
{
    panel.setBounds (getLocalBounds());
}

}
