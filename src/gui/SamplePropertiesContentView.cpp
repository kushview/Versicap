
#include "gui/SamplePropertiesContentView.h"
#include "Versicap.h"

namespace vcp {

SamplePropertiesContentView::SamplePropertiesContentView (Versicap& vc)
    : PanelContentView (vc)
{
    setName ("Sample");
    addAndMakeVisible (panel);
    
    addComponentListener (this);

    watcher.onActiveSampleChanged = [this]()
    {
        panel.clear();
        auto sample = watcher.getProject().getActiveSample();
        Array<PropertyComponent*> props;
        sample.getProperties (props);
        panel.addProperties (props);
    };

    watcher.setProject (vc.getProject());
}

SamplePropertiesContentView::~SamplePropertiesContentView()
{

}

void SamplePropertiesContentView::resizeContent (const Rectangle<int>& area)
{
    panel.setBounds (area);
}

}
