
#include "gui/SampleEditContentView.h"
#include "gui/WaveDisplayComponent.h"

#include "ProjectWatcher.h"
#include "Versicap.h"

namespace vcp {

class WaveCursor : public Component,
                   public Value::Listener
{
public:
    WaveCursor()
    {
        color = Colours::black;
        position.addListener (this);
    }

    ~WaveCursor() = default;

    void valueChanged (Value& value) override
    {
        if (value.refersToSameSourceAs (position))
            update();
    }

    double getPosition() const { return position.getValue(); }
    Value& getPositionValue() { return position; }

    void paint (Graphics& g) override
    {
        g.setOpacity (opacity);
        g.fillAll (color);
    }

    void mouseDown (const MouseEvent& ev) override
    {
        if (! dragging)
        {
            dragging = true;
            dragPos = getPosition();
        }
    }

    void mouseDrag (const MouseEvent& ev) override
    {
        position.setValue (dragPos + (secondsPerPixel * static_cast<double> (ev.getDistanceFromDragStartX())));
        update();
    }

    void update()
    {
        setBounds (getBoundsInParent().withX (roundToInt (getPosition() * pixelsPerSecond)));
    }

    void mouseUp (const MouseEvent& ev) override
    {
        if (dragging)
        {
            dragging = false;
            dragPos = getPosition();
        }
    }

    MouseCursor getMouseCursor() override
    {
        return MouseCursor::LeftRightResizeCursor;
    }

    void setSecondsPerPixel (const double newVal)
    {
        secondsPerPixel = std::fabs (newVal);
        pixelsPerSecond = secondsPerPixel != 0.0 
            ? 1.0 / secondsPerPixel
            : 0.0;
    }

private:
    bool dragging = false;
    double dragPos  = 0.0;
    Value position;
    float opacity = 1.f;
    Colour color;
    double pixelsPerSecond = 0.0;
    double secondsPerPixel = 0.0;
};

class SampleDisplayPanel : public Component
{
public:
    SampleDisplayPanel (SampleEditContentView& view)
        : owner (view)
    {
        addAndMakeVisible (wave);
        addAndMakeVisible (inPoint);
        addAndMakeVisible (outPoint);
    }

    void setSample (const Sample& newSample)
    {
        sample = newSample;
    
        if (sample.getFile().existsAsFile())
        {
            wave.setAudioThumbnail (owner.getVersicap().createAudioThumbnail (sample.getFile()));
            inPoint.setSecondsPerPixel (wave.getSecondsPerPixel());
            outPoint.setSecondsPerPixel (wave.getSecondsPerPixel());
            setSize (owner.getWidth(), owner.getHeight());
        }

        inPoint.getPositionValue().referTo (sample.getPropertyAsValue ("startTime"));
        outPoint.getPositionValue().referTo (sample.getPropertyAsValue ("endTime"));
    }

    void resized() override
    {
        wave.setBounds (getLocalBounds());
        inPoint.setBounds (roundToInt (wave.getPixelsPerSecond() * inPoint.getPosition()), 0, 1, getHeight());
        outPoint.setBounds (roundToInt (wave.getPixelsPerSecond() * outPoint.getPosition()), 0, 1, getHeight());
    }

    void zoomIn() 
    {
        auto step = static_cast<double> (wave.getWidth() / 4) * wave.getSecondsPerPixel();
        wave.setEndTime (wave.getEndTime() - step);
        updateMarkers();
    }

    void zoomOut()
    {
        auto step = static_cast<double> (wave.getWidth() / 4) * wave.getSecondsPerPixel();
        wave.setEndTime (wave.getEndTime() + step);
        updateMarkers();
    }

    void updateMarkers()
    {
        inPoint.setSecondsPerPixel (wave.getSecondsPerPixel());
        outPoint.setSecondsPerPixel (wave.getSecondsPerPixel());
        inPoint.update();
        outPoint.update();
    }

private:
    SampleEditContentView& owner;
    Sample sample;
    WaveDisplayComponent wave;
    WaveCursor inPoint, outPoint;
};

class SampleEditContentView::Content : public Component
{
public:
    Content (SampleEditContentView& o)
        : owner (o)
    {
        // addAndMakeVisible (view);
        panel.reset (new SampleDisplayPanel (o));
        addAndMakeVisible (panel.get());
        // view.setViewedComponent (panel.get(), false);
        // view.setScrollBarsShown (false, true, false, false);

        addAndMakeVisible (zoomIn);
        zoomIn.setButtonText ("+");
        zoomIn.onClick = [this]() {
            panel->zoomIn();
        };

        addAndMakeVisible (zoomOut);
        zoomOut.setButtonText ("-");
        zoomOut.onClick = [this]() {
            panel->zoomOut();
        };

        watcher.onActiveSampleChanged = [this]()
        {
            auto sample = watcher.getProject().getActiveSample();
            panel->setSample (sample);
            resized();
        };
    }

    ~Content()
    {
        view.setViewedComponent (nullptr, false);
        panel.reset();
    }

    Project getProject() const { return watcher.getProject(); }
    
    void setProject (const Project& project)
    {
        watcher.setProject (project);
    }

    void resized() override
    {
        // view.setBounds (getLocalBounds());
        // auto pr = panel->getBoundsInParent();

        // auto& scroll = view.getHorizontalScrollBar();
        // int vh = view.getHeight();
        // if (scroll.isShowing())
        //     vh -= view.getScrollBarThickness();

        // if (pr.getHeight() != vh)
        //     pr.setHeight (vh);
        // if (pr.getWidth() < view.getWidth())
        //     pr.setWidth (view.getWidth());
        // if (pr != panel->getBoundsInParent())
        //     panel->setBounds (pr);

        panel->setBounds (getLocalBounds());
        
        auto r = getLocalBounds();
        r = r.removeFromTop (22);

        zoomOut.setBounds (r.removeFromRight (24));
        zoomIn.setBounds (r.removeFromRight (24));
    }

    Viewport& getViewPort() { return view; }

private:
    SampleEditContentView& owner;
    ProjectWatcher watcher;
    Viewport view;
    std::unique_ptr<SampleDisplayPanel> panel;
    TextButton zoomIn, zoomOut;
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
