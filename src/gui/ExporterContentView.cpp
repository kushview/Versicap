
#include "gui/ExporterContentView.h"
#include "ProjectWatcher.h"
#include "Versicap.h"

namespace vcp {

class ExporterContentView::Content : public Component,
                                     private Versicap::Listener
{
public:
    Content (Versicap& vc)
        : versicap (vc)
    {
        watcher.setProject (versicap.getProject());
    }

    ~Content() {}

private:
    Versicap& versicap;
    ProjectWatcher watcher;

    void projectChanged() override
    {
        watcher.setProject (versicap.getProject());
    }
};

ExporterContentView::ExporterContentView (Versicap& vc)
    : ContentView (vc)
{
    content.reset (new Content (vc));
}

ExporterContentView::~ExporterContentView()
{
    content.reset();
}

void ExporterContentView::resized()
{
    content->setBounds (getLocalBounds());
}

}