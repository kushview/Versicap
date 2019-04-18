
#include "gui/SourceContentView.h"
#include "Tags.h"
#include "Types.h"
#include "Versicap.h"

namespace vcp {

SourceContentView::SourceContentView (Versicap& vc)
    : PanelContentView (vc)
{
    setName ("Source");

    addAndMakeVisible (sourceLabel);
    sourceLabel.setText ("Source", dontSendNotification);
    sourceLabel.setFont (Font (12.f));
    addAndMakeVisible (sourceCombo);
    sourceCombo.addItem ("MIDI Device", 1 + SourceType::MidiDevice);
    sourceCombo.addItem ("Plugin", 1 + SourceType::AudioPlugin);
    sourceCombo.onChange = [this]() { sourceChanged(); };
    sourceCombo.setSelectedId (1, dontSendNotification);
}

SourceContentView::~SourceContentView() {}

void SourceContentView::resizeContent (const Rectangle<int>& area)
{
    auto r1 = area.reduced (4);
    auto r2 = r1.removeFromTop (22);
    sourceLabel.setBounds (r2.removeFromLeft (80));
    r2.removeFromLeft (4);
    sourceCombo.setBounds (r2);
}

void SourceContentView::sourceChanged()
{
    versicap.getProject().setProperty (Tags::source,
        SourceType::getSlug (getSourceType()));}
}
