#pragma once

#include "gui/ContentView.h"

namespace vcp {

class SourceContentView : public PanelContentView
{
public:
    SourceContentView (Versicap& vc);
    ~SourceContentView();

    int getSourceType() const { return sourceCombo.getSelectedId() - 1; }

protected:
    void resizeContent (const Rectangle<int>&) override;

private:
    Label sourceLabel;
    ComboBox sourceCombo;

    void sourceChanged();
};

}
