
#include "gui/ExportersListContentView.h"
#include "gui/LayersTableContentView.h"
#include "gui/ProjectConcertinaPanel.h"
#include "gui/SamplesTableContentView.h"

namespace vcp {

ProjectConcertinaPanel::ProjectConcertinaPanel()
{
    setSize (220, 640);
}

ProjectConcertinaPanel::~ProjectConcertinaPanel() {}

void ProjectConcertinaPanel::createPanels (Versicap& versicap)
{
    if (getNumPanels() > 0)
        return;
    addPanel (-1, new LayersTableContentView (versicap), true);
    addPanel (-1, new SamplesTableContentView (versicap), true);
    addPanel (-1, new ExportersListContentView (versicap), true);
    for (int i = 0; i < getNumPanels(); ++i)
        setPanelHeaderSize (getPanel (i), 23);
    setPanelSize (getPanel (0), 140, false);
    setPanelSize (getPanel (2), 200, false);
}

}
