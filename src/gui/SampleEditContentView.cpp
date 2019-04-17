
#include "gui/SampleEditContentView.h"
#include "gui/WaveDisplayComponent.h"

#include "ProjectWatcher.h"
#include "Versicap.h"

namespace vcp {

class WaveCursor : public Component
{
public:
    WaveCursor() = default;
    ~WaveCursor() = default;

    double getPosition() const { return position; }
    
    void paint (Graphics& g) override
    {
        g.setOpacity (opacity);
        g.fillAll (color);
    }

    MouseCursor getMouseCursor() override
    {
        return MouseCursor::LeftRightResizeCursor;
    }

private:
    double position = 0.0;
    float opacity = 1.f;
    Colour color;
};

class SampleEditContentView::Content : public Component
{
public:
    Content (SampleEditContentView& view)
        : owner (view)
    {
        addAndMakeVisible (wave);
        watcher.onActiveSampleChanged = [this]()
        {
            const auto sample = watcher.getProject().getActiveSample();
            if (sample.getFile().existsAsFile())
            {
                wave.setAudioThumbnail (owner.versicap.createAudioThumbnail (sample.getFile()));
                resized();
            }
        };

        addAndMakeVisible (range);
        addAndMakeVisible (cursor);
    }

    Project getProject() const { return watcher.getProject(); }
    void setProject (const Project& project)
    {
        watcher.setProject (project);
    }

    void resized() override
    {
        wave.setBounds (getLocalBounds());
        range.setBounds (getLocalBounds().reduced (20, 0));
        int cursorX = (getWidth() / 2) - 1;
        cursor.setBounds (cursorX, 0, 1, getHeight());
    }

private:
    SampleEditContentView& owner;
    ProjectWatcher watcher;
    WaveDisplayComponent wave;
    WaveCursor cursor;
    SampleInOutRange range;
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
