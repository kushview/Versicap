
#include "gui/SamplesTableContentView.h"
#include "Project.h"
#include "ProjectWatcher.h"

namespace vcp {

class SampleTable : public TableListBox,
                    public TableListBoxModel
{
public:
    enum Columns {
        NoteColumn = 1,
        MidiColumn = 2,
        NameColumn = 3
    };
    
    SampleTable()
    {
        setModel (this);
        setHeaderHeight (24);
        getHeader().addColumn ("Note", NoteColumn, 60);
        getHeader().addColumn ("MIDI", MidiColumn, 60);
        getHeader().addColumn ("Name", NameColumn, 100);

        watcher.onChanged = [this]() { refreshSamples(); };
        watcher.onSamplesAdded = [this]() { refreshSamples(); };
        watcher.onActiveLayerChanged = [this]()
        {
            layer = watcher.getProject().getActiveLayer();
            refreshSamples();
        };
    }

    ~SampleTable()
    {
        setModel (nullptr);
    }

    void setProject (const Project& newProject)
    {
        watcher.setProject (newProject);
    }

    Project getProject() const { return watcher.getProject(); }

    void refreshSamples()
    {
        const auto project = watcher.getProject();
        filtered.clearQuick (true);
        auto layerIdx = project.indexOf (layer);
        if (isPositiveAndBelow (layerIdx, project.getNumLayers()))
            project.getSamples (layerIdx, filtered);
        updateContent();
        repaint();
    }

    //=========================================================================
    int getNumRows() override { return filtered.size(); }

    void paintRowBackground (Graphics& g, int rowNumber,
                             int width, int height, bool rowIsSelected) override
    {
        if (rowIsSelected)
        {
            g.setOpacity (0.84);
            g.setColour (Colours::orange);
            g.fillAll();
        }
    }

    void paintCell (Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool rowIsSelected) override
    {
        g.setColour (rowIsSelected ? Colours::white
                                   : kv::LookAndFeel_KV1::textColor);

        if (auto* const sample = filtered [rowNumber])
        {
            String text;
            switch (columnId)
            {
                case MidiColumn: text = String (sample->getNote()); break;
                case NoteColumn: text = MidiMessage::getMidiNoteName (sample->getNote(), true, true, 4); break;
                case NameColumn: text = sample->getProperty (Tags::name);
            } 

            g.drawText (text, 10, 0, width - 10, height, Justification::centredLeft);
        }
    }

    void selectedRowsChanged (int lastRowSelected) override
    {
        auto project = getProject();
        if (auto* sample = filtered [lastRowSelected])
            project.setActiveSample (*sample);
    }
   #if 0
    virtual Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected,
                                                Component* existingComponentToUpdate);
    virtual void cellClicked (int rowNumber, int columnId, const MouseEvent&);
    virtual void cellDoubleClicked (int rowNumber, int columnId, const MouseEvent&);
    virtual void backgroundClicked (const MouseEvent&);
    virtual void sortOrderChanged (int newSortColumnId, bool isForwards);
    virtual int getColumnAutoSizeWidth (int columnId);
    virtual String getCellTooltip (int rowNumber, int columnId);
    
    virtual void deleteKeyPressed (int lastRowSelected);
    virtual void returnKeyPressed (int lastRowSelected);
    virtual void listWasScrolled();
    virtual var getDragSourceDescription (const SparseSet<int>& currentlySelectedRows);
   #endif

private:
    ProjectWatcher watcher;
    Layer layer;
    OwnedArray<Sample> filtered;
};

class SamplesTableContentView::Content : public Component
{
public:
    Content()
    { 
        addAndMakeVisible (table);
    }

    ~Content() { }

    void resized() override
    {
        table.setBounds (getLocalBounds());
    }

private:
    friend class SamplesTableContentView;
    SampleTable table;
};

SamplesTableContentView::SamplesTableContentView (Versicap& vc)
    : PanelContentView (vc)
{
    setName ("Samples");
    content.reset (new Content());
    addAndMakeVisible (content.get());
    versicap.addListener (this);
    auto& table = content->table;
    table.setProject (versicap.getProject());
}

SamplesTableContentView::~SamplesTableContentView()
{

}

void SamplesTableContentView::resizeContent (const Rectangle<int>& area)
{
    content->setBounds (area.reduced (4, 2));
}

void SamplesTableContentView::projectChanged()
{
    auto project = versicap.getProject();
    auto& table = content->table;
    table.setProject (project);
}

}
