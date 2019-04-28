
#include "exporters/Exporter.h"

namespace vcp {

//=============================================================================
class DirectoryPropertyComponent : public PropertyComponent,
                                   private FilenameComponentListener,
                                   private Value::Listener
{
public:
    DirectoryPropertyComponent (const Value& valueToControl,
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

//=============================================================================
void Exporter::getProperties (Array<PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (getPropertyAsValue (Tags::name),
        "Exporter Name", 100, false, true));
    props.add (new DirectoryPropertyComponent (getPropertyAsValue (Tags::path),
        "Export Path"));

    if (ExporterTypePtr type = getTypeObject())
    {
        Array<LoopType> loops;
        type->getLoopTypes (loops);
        if (loops.isEmpty())
            loops.add (LoopType::None);
            
        StringArray choices; Array<var> values;
        for (const auto& loop : loops)
        {
            choices.add (loop.getName());
            values.add (loop.getSlug());
        }

        props.add (new ChoicePropertyComponent (getPropertyAsValue (Tags::sampleRate),
            "Sample Rate", { "22050", "44100", "48000" }, { 22050, 44100, 48000 }));
        props.add (new ChoicePropertyComponent (getPropertyAsValue (Tags::bitDepth),
            "Bit Depth", { "16 bit", "24 bit" }, { 16, 24 }));
        props.add (new ChoicePropertyComponent (getPropertyAsValue (Tags::channels),
            "Bit Depth", { "Mono", "Stereo" }, { 1, 2 }));

        props.add (new ChoicePropertyComponent (getPropertyAsValue (Tags::loop),
            "Loop Mode", choices, values));
    }

    props.add (new ChoicePropertyComponent (getPropertyAsValue (Tags::layers),
        "Layers", { "All Layers" }, { "all" }));
}

}
