
#include "gui/LookAndFeel.h"
#include "gui/SampleEditContentView.h"
#include "gui/WaveDisplayComponent.h"

#include "ProjectWatcher.h"
#include "Versicap.h"

namespace vcp {

class WaveZoomBar : public Component
{
public:
    WaveZoomBar()
    { 
        visibleRange.setStart (0.25);
        visibleRange.setLength (0.5);
    }
    
    ~WaveZoomBar() { }

    double getTotalLength() const { return totalLength; }

    void setTotalLength (double length)
    {
        if (length == totalLength)
            return;
        jassert (length >= 0.0);
        totalLength = length;
        if (visibleRange.getLength() > totalLength)
            visibleRange.setLength (totalLength);
        updateBarRectangle();
        repaint();
    }

    void setRange (double start, double end, bool notify = false)
    {
        visibleRange.setStart (start);
        visibleRange.setEnd (end);
        if (end > totalLength)
            totalLength = end;
        updateBarRectangle();
        repaint();

        if (onMoved && notify)
            onMoved();
    }

    void setRange (double start, double end, double total)
    {
        totalLength = total;
        visibleRange.setStart (start);
        visibleRange.setEnd (end);
        if (visibleRange.getLength() > totalLength)
            visibleRange.setLength (totalLength);
        updateBarRectangle();
        repaint();
    }

    void setRange (Range<double> range, double total)
    {
        totalLength = total;
        visibleRange = range;
        if (visibleRange.getLength() > totalLength)
            visibleRange.setLength (totalLength);
        updateBarRectangle();
        repaint();
    }

    Range<double> getVisibleRange() const { return visibleRange; }

    void resized() override
    { 
        updateBarRectangle();
    }
    
    void paint (Graphics& g) override
    {
        g.fillAll (LookAndFeel::widgetBackgroundColor.darker());
        g.setColour (LookAndFeel::widgetBackgroundColor);
        g.fillRect (rect);
    }

    void mouseDown (const MouseEvent& ev) override
    {
        dragging  = rect.contains (ev.position);
        trimStart = ev.position.x >= rect.getX() && ev.position.x < rect.getX() + 4.0;
        trimEnd   = ev.position.x >= rect.getRight() - 4.0 && ev.position.x < rect.getRight();
        if (dragging)
        {
            dragStartPos    = ev.position;
            dragStart       = visibleRange.getStart();
            dragEnd         = visibleRange.getEnd();
        }
    }

    void mouseUp (const MouseEvent& ev) override
    {
        dragging = false;
    }

    void mouseMove (const MouseEvent& ev) override
    {
        if (ev.position.x >= rect.getX() && ev.position.x < rect.getX() + 4.0)
            setMouseCursor (MouseCursor::LeftRightResizeCursor);
        else if (ev.position.x >= rect.getRight() - 4.0 && ev.position.x < rect.getRight())
            setMouseCursor (MouseCursor::LeftRightResizeCursor);
        else
            setMouseCursor (MouseCursor::NormalCursor);
    }

    void mouseDrag (const MouseEvent& ev) override
    {
        if (! dragging)
            return;
        
        const auto oldRange = visibleRange;
        double delta = ev.position.x - dragStartPos.x;
        delta = totalLength * (delta / (double) getWidth());

        if (trimStart)
        {
            const double start = dragStart + delta;
            const double maxStartEnd = totalLength - minLength;
            visibleRange.setStart (jlimit (0.0, maxStartEnd, start));
        }
        else if (trimEnd)
        {
            const double end = dragEnd + delta;
            visibleRange.setEnd (jlimit (0.0, totalLength, end));
        }
        else
        {
            const double start = dragStart + delta;
            const double maxStartEnd = totalLength - visibleRange.getLength();
            visibleRange = visibleRange.movedToStartAt (jlimit (0.0, maxStartEnd, start));
        }
        
        if (visibleRange != oldRange)
        {
            updateBarRectangle();
            repaint();
            if (onMoved)
                onMoved();
        }
    }

    std::function<void()> onMoved;

private:
    bool dragging = false;
    bool trimStart = false;
    bool trimEnd = false;

    Point<float> dragStartPos;
    double dragStart = 0.0;
    double dragEnd = 0.0;
    
    double minLength;
    double totalLength = 1.0;
    Range<double> visibleRange;
    
    Rectangle<float> rect;

    void updateBarRectangle()
    {
        rect = {
            static_cast<float> ((visibleRange.getStart() / totalLength) * (double) getWidth()),
            0.f,
            static_cast<float> ((visibleRange.getLength() / totalLength) * (double) getWidth()),
            (float) getHeight()
        };
    }
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

    void setPositionOffset (double newOffset) { offset = newOffset; }
    void update()
    {
        setBounds (getBoundsInParent().withX (roundToInt (pixelsPerSecond * (getPosition() - offset))));
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
    double offset = 0.0;
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
        addAndMakeVisible (zoomBar);

        zoomBar.onMoved = [this]()
        {
            wave.setRange (zoomBar.getVisibleRange());
            inPoint.setSecondsPerPixel (wave.getSecondsPerPixel());
            inPoint.setPositionOffset (zoomBar.getVisibleRange().getStart());
            outPoint.setSecondsPerPixel (wave.getSecondsPerPixel());
            outPoint.setPositionOffset (zoomBar.getVisibleRange().getStart());
            updateMarkerBounds();
        };

        addAndMakeVisible (zoomInButton);
        zoomInButton.setButtonText ("+");
        zoomInButton.onClick = [this]() { zoomIn(); };

        addAndMakeVisible (zoomOutButton);
        zoomOutButton.setButtonText ("-");
        zoomOutButton.onClick = [this]() { zoomOut(); };

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
    
        if (sample.isValid() && sample.getFile().existsAsFile())
        {
            wave.setAudioThumbnail (owner.getVersicap().createAudioThumbnail (sample.getFile()));
            inPoint.setSecondsPerPixel (wave.getSecondsPerPixel());
            outPoint.setSecondsPerPixel (wave.getSecondsPerPixel());
        }
        else
        {
            wave.setAudioThumbnail (nullptr);
        }

        timeIn.removeListener (this);
        timeOut.removeListener (this);
        
        if (sample.isValid())
        {
            zoomBar.setRange (wave.getStartTime(), wave.getEndTime(), sample.getTotalTime());
            timeIn = sample.getPropertyAsValue (Tags::timeIn);
            inPoint.getPositionValue().referTo (timeIn);
            inPoint.setMaxPosition (sample.getTotalTime() - 0.01);
            
            timeOut = sample.getPropertyAsValue (Tags::timeOut);
            outPoint.getPositionValue().referTo (timeOut);
            outPoint.setMinPosition (0.01);
            outPoint.setMaxPosition (sample.getTotalTime());
        }
        else
        {
            timeIn = Value();
            inPoint.getPositionValue().referTo (timeIn);
            timeOut = Value();
            outPoint.getPositionValue().referTo (timeOut);
        }

        timeIn.addListener (this);
        timeOut.addListener (this);

        setSize (owner.getWidth(), owner.getHeight());
    }

    void mouseWheelMove (const MouseEvent& event,
                         const MouseWheelDetails& wheel) override
    {
        if (! displayBounds.contains (event.position.toInt()))
            return;

        if (wheel.deltaY > 0.f)
        {
            // zoomIn();
        }
        else if (wheel.deltaY < 0.f)
        {
            // zoomOut();
        }

        const float scrollScale = -1.4f;
        
        if (wheel.deltaX < 0.f)
        {
            auto delta = wheel.deltaX * scrollScale;
            auto range = zoomBar.getVisibleRange();
            auto end = range.getEnd() + delta;
            range = range.movedToEndAt (jmin (zoomBar.getTotalLength(), end));
            zoomBar.setRange (range.getStart(), range.getEnd(), true);
        }
        else if (wheel.deltaX > 0.f)
        {
            auto delta = wheel.deltaX * scrollScale;
            auto range = zoomBar.getVisibleRange();
            auto start = range.getStart() + delta;
            range = range.movedToStartAt (jmax (0.0, start));
            zoomBar.setRange (range.getStart(), range.getEnd(), true);
        }
    }

    void resized() override
    {
        auto r1 = getLocalBounds();
        auto r2 = r1.removeFromBottom (22);
        wave.setBounds (r1);
        displayBounds = r1;

        zoomBar.setRange (wave.getStartTime(), wave.getEndTime());
        zoomInButton.setBounds (r2.removeFromRight (24));
        zoomOutButton.setBounds (r2.removeFromRight (24));
        r2.removeFromRight (2);

        zoomBar.setBounds (r2);
        updateMarkerBounds();
    }

    void zoomIn()
    {
        auto step = static_cast<double> (wave.getWidth() / 4) * wave.getSecondsPerPixel();
        wave.setEndTime (wave.getEndTime() - step);
        zoomBar.setRange (wave.getStartTime(), wave.getEndTime());
        updateMarkers();
    }

    void zoomOut()
    {
        auto step = static_cast<double> (wave.getWidth() / 4) * wave.getSecondsPerPixel();
        wave.setEndTime (wave.getEndTime() + step);
        zoomBar.setRange (wave.getStartTime(), wave.getEndTime());
        updateMarkers();
    }

private:
    SampleEditContentView& owner;
    Sample sample;
    Value timeIn, timeOut;
    WaveDisplayComponent wave;
    WaveZoomBar zoomBar;
    TextButton zoomInButton, zoomOutButton;
    WaveCursor inPoint, outPoint;

    Rectangle<int> displayBounds;

    void updateMarkerBounds()
    {
        auto visible = zoomBar.getVisibleRange();
        inPoint.setBounds (roundToInt (wave.getPixelsPerSecond() * (inPoint.getPosition() - visible.getStart())),
                           displayBounds.getY(), 
                           1, 
                           displayBounds.getHeight());
        outPoint.setBounds (roundToInt (wave.getPixelsPerSecond() * (outPoint.getPosition() - visible.getStart())), 
                            displayBounds.getY(), 
                            1, 
                            displayBounds.getHeight());
    }

    void updateMarkers()
    {
        inPoint.setSecondsPerPixel (wave.getSecondsPerPixel());
        outPoint.setSecondsPerPixel (wave.getSecondsPerPixel());
        inPoint.update();
        outPoint.update();
    }
};

class SampleEditContentView::Content : public Component
{
public:
    Content (SampleEditContentView& o)
        : owner (o)
    {
        panel.reset (new SampleDisplayPanel (o));
        addAndMakeVisible (panel.get());

        watcher.onActiveSampleChanged = [this]() {
            refreshWithActiveSample();
        };
    }

    ~Content()
    {
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
        panel->setBounds (r1.reduced (1, 1));
    }

private:
    SampleEditContentView& owner;
    ProjectWatcher watcher;
    std::unique_ptr<SampleDisplayPanel> panel;
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
