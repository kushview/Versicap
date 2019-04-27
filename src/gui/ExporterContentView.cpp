
#include "gui/ExporterContentView.h"
#include "ProjectWatcher.h"
#include "Types.h"
#include "Versicap.h"

namespace vcp {

//=============================================================================
class DirecotryPropertyComponent : public PropertyComponent,
                                   private FilenameComponentListener,
                                   private Value::Listener
{
public:
    DirecotryPropertyComponent (const Value& valueToControl,
                                const String& propertyName)
        : PropertyComponent (propertyName),
          directory ("Directory", File(), false, true, true,
                     String(), String(), "Path for rendered files")
    {
        addAndMakeVisible (directory);
        directory.addListener (this);
        value.referTo (valueToControl);
        value.addListener (this);
        refresh();
    }

    void refresh() override
    {
        const String path = value.getValue().toString();
        if (File::isAbsolutePath (path))
            directory.setCurrentFile (File (path), dontSendNotification);
    }

private:
    FilenameComponent directory;
    Value value;

    void valueChanged (Value&) override { refresh(); }
    void filenameComponentChanged (FilenameComponent*) override
    {
        value.removeListener (this);
        value.setValue (directory.getCurrentFile().getFullPathName());
        value.addListener (this);
    }
};

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
    ValueTree exporter = tree;

    Array<PropertyComponent*> props;

    props.add (new TextPropertyComponent (exporter.getPropertyAsValue (Tags::name, nullptr),
        "Exporter Name", 100, false, true));
    props.add (new DirecotryPropertyComponent (exporter.getPropertyAsValue ("path", nullptr),
        "Export Path"));

    if (ExporterTypePtr type = dynamic_cast<ExporterType*> (exporter.getProperty (Tags::object).getObject()))
    {
        Array<LoopType> loops;
        type->getLoopTypes (loops);
        StringArray choices; Array<var> values;
        for (const auto& loop : loops)
        {
            choices.add (loop.getName());
            values.add (loop.getSlug());
        }

        props.add (new ChoicePropertyComponent (exporter.getPropertyAsValue (Tags::loop, nullptr),
            "Loop Mode", choices, values));
    }

    props.add (new ChoicePropertyComponent (exporter.getPropertyAsValue (Tags::layers, nullptr),
        "Layers", { "All Layers" }, { "all" }));

    panel.addProperties (props);
}

void ExporterContentView::resized()
{
    content->setBounds (getLocalBounds());
}

}