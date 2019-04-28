
#include "gui/ExporterContentView.h"
#include "exporters/Exporter.h"
#include "ProjectWatcher.h"
#include "Types.h"
#include "Versicap.h"

namespace vcp {



class ExporterContentView::Content : public Component,
                                     private Versicap::Listener,
                                     private Value::Listener
{
public:
    Content (Versicap& vc)
        : versicap (vc)
    {
        addAndMakeVisible (panel);
        watcher.setProject (versicap.getProject());
        name.addListener (this);
    }

    ~Content() {
        name.removeListener (this);
    }

    void setExporter (const Exporter& e)
    {
        exporter = e;
        name.referTo (exporter.getPropertyAsValue (Tags::name));
        panel.clear();
        Array<PropertyComponent*> props;
        exporter.getProperties (props);
        panel.addProperties (props);
    }

    void paint (Graphics& g) override
    {
        g.setColour (Colours::black);
        g.fillRect (getLocalBounds().removeFromTop (42));
        g.setColour (kv::LookAndFeel_KV1::textColor);
        g.setFont (Font (18.f, Font::plain));
        g.drawText (title, getLocalBounds().removeFromTop (42).reduced (4, 0), 
            Justification::centredLeft, true);
    }

    void resized() override
    {
        auto r1 = getLocalBounds();
        auto r2 = r1.removeFromTop (42);
        panel.setBounds (r1);
    }

    void updateTitle()
    {
        auto type = exporter.getTypeObject();
        title = (type) ? type->getName() : "";
        auto name = exporter.getProperty (Tags::name).toString();

        if (this->title.isNotEmpty())
        {
            if (name != this->title)
                this->title << " - " << name;
        }
        else
        {
            this->title = name;
        }
        
        this->repaint (0, 0, getWidth(), 42);
    }

    void valueChanged (Value& value) override
    {
        if (value.refersToSameSourceAs (name))
            updateTitle();
    }

private:
    friend class ExporterContentView;
    Versicap& versicap;
    Exporter exporter;
    Value name;
    ProjectWatcher watcher;
    String title;
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
    
    Exporter exporter (tree);
    content->setExporter (exporter);
}

void ExporterContentView::resized()
{
    content->setBounds (getLocalBounds());
}

}
