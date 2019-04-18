
#include "gui/ContentView.h"

namespace vcp {

void PanelContentView::paint (Graphics& g)
{
    Rectangle<float> lb (0.f, 0.f, (float) getWidth(), 24.f);
    g.setColour (kv::LookAndFeel_KV1::widgetBackgroundColor.brighter());
    g.fillRect (lb.reduced (2.f));
    g.setColour (kv::LookAndFeel_KV1::textColor);
    g.drawText (getName(), lb, Justification::centred);
}

void PanelContentView::resized()
{
    auto r = getLocalBounds();
    r.removeFromTop (24);
    resizeContent (r);
}

}
