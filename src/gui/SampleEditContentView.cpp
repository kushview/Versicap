
#include "gui/SampleEditContentView.h"
#include "gui/WaveDisplayComponent.h"

#include "ProjectWatcher.h"
#include "Versicap.h"

namespace vcp {

class SampleEditContentView::Content : public Component
{
public:
    Content (SampleEditContentView& view)
        : owner (view)
    {
        watcher.onActiveSampleChanged = [this]()
        {
            const auto sample = watcher.getProject().getActiveSample();
            DBG(sample.getFile().getFullPathName());
        };
    }

    Project getProject() const { return watcher.getProject(); }
    void setProject (const Project& project)
    {
        watcher.setProject (project);
    }

private:
    SampleEditContentView& owner;
    ProjectWatcher watcher;
};

SampleEditContentView::SampleEditContentView (Versicap& vc)
    : ContentView (vc)
{
    content.reset (new Content (*this));
    addAndMakeVisible (content.get());
    content->setProject (versicap.getProject());
}

void SampleEditContentView::resized()
{
    content->setBounds (getLocalBounds());
}

}
