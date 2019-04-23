
#include "gui/SampleEditContentView.h"
#include "gui/WaveDisplayComponent.h"

#include "ProjectWatcher.h"
#include "Versicap.h"

namespace vcp {

class WaveZoomBar : public Component
{
public:
    WaveZoomBar() { }
    ~WaveZoomBar() { }

    void resized() override { }
    void paint (Graphics&) override { }
    void mouseMove (const MouseEvent&) override { }
};

class WaveCursor : public Component,
                   public Value::Listener
{
public:
    WaveCursor()
    {
        color = Colours::red;
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
        double newPos = dragPos + (secondsPerPixel * static_cast<double> (ev.getDistanceFromDragStartX()));
        position.setValue (jlimit (minPos, maxPos, newPos));
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

    void setMinPosition (double newMin)
    {
        minPos = newMin;
        if (getPosition() < minPos)
            position.setValue (minPos);
    }

    void setMaxPosition (double newMax)
    {
        maxPos = newMax;
        if (getPosition() > maxPos)
            position.setValue (maxPos);
    }

private:
    bool dragging = false;
    double dragPos  = 0.0;
    double minPos = 0.0;
    double maxPos = 0.0;
    Value position;
    float opacity = 0.85;
    Colour color;
    double pixelsPerSecond = 0.0;
    double secondsPerPixel = 0.0;
};

class SampleDisplayPanel : public Component,
                           public Value::Listener
{
public:
    SampleDisplayPanel (SampleEditContentView& view)
        : owner (view)
    {
        addAndMakeVisible (wave);
        addAndMakeVisible (inPoint);
        addAndMakeVisible (outPoint);

        timeIn.addListener (this);
        timeOut.addListener (this);
    }

    void valueChanged (Value& value) override
    {
        const double spread = 0.01;
        if (value.refersToSameSourceAs (timeIn))
        {
            double ti = timeIn.getValue();
            double to = timeOut.getValue();
            if (ti > to - spread)
                timeOut.setValue (ti + spread);
        }
        else if (value.refersToSameSourceAs (timeOut))
        {
            double ti = timeIn.getValue();
            double to = timeOut.getValue();
            if (to < ti + spread)
                timeIn.setValue (to - spread);
        }
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

        timeIn.removeListener (this);
        timeIn = sample.getPropertyAsValue (Tags::timeIn);
        inPoint.getPositionValue().referTo (timeIn);
        inPoint.setMaxPosition (sample.getTotalTime() - 0.01);
        timeIn.addListener (this);

        timeOut.removeListener (this);
        timeOut = sample.getPropertyAsValue (Tags::timeOut);
        outPoint.getPositionValue().referTo (timeOut);
        outPoint.setMinPosition (0.01);
        outPoint.setMaxPosition (sample.getTotalTime());
        timeOut.addListener (this);
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
    Value timeIn, timeOut;
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

        watcher.onActiveSampleChanged = [this]() {
            refreshWithActiveSample();
        };
    }

    ~Content()
    {
        view.setViewedComponent (nullptr, false);
        panel.reset();
    }

    void refreshWithActiveSample()
    {
        auto sample = watcher.getProject().getActiveSample();
        panel->setSample (sample);
        resized();
    }

    Project getProject() const { return watcher.getProject(); }
    
    void setProject (const Project& project)
    {
        watcher.setProject (project);
        refreshWithActiveSample();
    }

    void resized() override
    {
        auto r1 = getLocalBounds();
        auto r2 = getLocalBounds().removeFromTop (22);
        auto r3 = getLocalBounds().removeFromBottom (22);
        zoomOut.setBounds (r3.removeFromRight (24));
        zoomIn.setBounds (r3.removeFromRight (24));
        panel->setBounds (r1.reduced (1, 1));
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
    versicap.addListener (this);
    projectChanged();
}

SampleEditContentView::~SampleEditContentView()
{
    versicap.removeListener (this);
    content.reset();
}

void SampleEditContentView::resized()
{
    content->setBounds (getLocalBounds());
}

void SampleEditContentView::projectChanged()
{
    if (content)
        content->setProject (versicap.getProject());
}

}
