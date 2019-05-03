
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
    
    std::function<void(const Sample&)> onSelected;

    SampleTable()
    {
        setModel (this);
        setHeaderHeight (24);
        getHeader().addColumn ("Note", NoteColumn, 52);
        getHeader().addColumn ("MIDI", MidiColumn, 52);
        getHeader().addColumn ("Name", NameColumn, 96);

        watcher.onChanged = [this]() { refreshSamples(); };
        watcher.onSamplesAdded = watcher.onSampleAdded = watcher.onSampleRemoved = [this]()
        { 
            refreshSamples(); 
        };
        
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
        layer = watcher.getProject().getActiveLayer();
        refreshSamples();
        selectActiveSample();
    }

    Project getProject() const { return watcher.getProject(); }

    void refreshSamples()
    {
        const auto project = watcher.getProject();
        filtered.clearQuick (true);
        auto layerIdx = project.indexOf (layer);
        
        if (isPositiveAndBelow (layerIdx, project.getNumLayers()))
            project.getSamples (layerIdx, filtered);
        
        struct NameSort
        {
            int compareElements (Sample* lhs, Sample* rhs) {
                return lhs->getNote() < rhs->getNote() ? -1 
                     : lhs->getNote() == rhs->getNote() ? 0
                     : lhs->getNote() > rhs->getNote() ? 1
                     : 0;
            }
        } sorter;

        filtered.sort (sorter);

        updateContent();
        repaint();
    }

    int indexOf (const Sample& sample)
    {
        for (int i = 0; i < filtered.size(); ++i)
            if (sample.getUuidString() == filtered[i]->getUuidString())
                return i;
        return -1;
    }

    void selectActiveSample()
    {
        const auto project = watcher.getProject();
        const auto sample = project.getActiveSample();
        auto sampleIdx = indexOf (sample);
        ProjectWatcher::ScopedBlock sb (watcher);
        if (isPositiveAndBelow (sampleIdx, filtered.size()))
            selectRow (sampleIdx);
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
        {
            project.setActiveSample (*sample);
            if (onSelected)
                onSelected (*sample);
        }
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
    Content (SamplesTableContentView& v)
        : owner (v)
    {
        addAndMakeVisible (table);
        table.onSelected = [this](const Sample& sample)
        {
            owner.getVersicap().post (
                new DisplayObjectMessage (sample.getValueTree()));
        };
    }

    ~Content() { }

    void resized() override
    {
        table.setBounds (getLocalBounds());
    }

private:
    friend class SamplesTableContentView;
    SamplesTableContentView& owner;
    SampleTable table;
};

SamplesTableContentView::SamplesTableContentView (Versicap& vc)
    : ContentView (vc)
{
    setName ("Samples");
    content.reset (new Content (*this));
    addAndMakeVisible (content.get());
    versicap.addListener (this);
    projectChanged();
}

SamplesTableContentView::~SamplesTableContentView()
{
    versicap.removeListener (this);
}

void SamplesTableContentView::resized ()
{
    content->setBounds (getLocalBounds());
}

void SamplesTableContentView::projectChanged()
{
    auto project = versicap.getProject();
    auto& table = content->table;
    table.setProject (project);
}

}
