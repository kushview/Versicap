
#include "gui/ExporterContentView.h"
#include "ProjectWatcher.h"
#include "Types.h"
#include "Versicap.h"

namespace vcp {



class ExporterContentView::Content : public Component,
                                     private Versicap::Listener
{
public:
    Content (Versicap& vc)
        : versicap (vc)
    {
        addAndMakeVisible (panel);
        watcher.setProject (versicap.getProject());
    }

    ~Content() { }

    void resized() override
    {
        panel.setBounds (getLocalBounds());
    }
private:
    friend class ExporterContentView;
    Versicap& versicap;
    ProjectWatcher watcher;
    PropertyPanel panel;
    void projectChanged() override
    {
        watcher.setProject (versicap.getProject());
    }
};

ExporterContentView::ExporterContentView (Versicap& vc)
    : ContentView (vc)
{
    content.reset (new Content (vc));
    addAndMakeVisible (content.get());
}

ExporterContentView::~ExporterContentView()
{
    content.reset();
}

void ExporterContentView::displayObject (const ValueTree& tree)
{
    jassert (tree.hasType (Tags::exporter));
    if (! tree.hasType (Tags::exporter))
        return;
    auto& panel = content->panel;
    
    panel.clear();
    Exporter exporter (tree);
    Array<PropertyComponent*> props;
    exporter.getProperties (props);
    panel.addProperties (props);
}

void ExporterContentView::resized()
{
    content->setBounds (getLocalBounds());
}

}
