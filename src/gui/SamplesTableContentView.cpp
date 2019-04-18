
#include "gui/SamplesTableContentView.h"
#include "Project.h"
#include "ProjectWatcher.h"

namespace vcp {

class SampleTable : public TableListBox,
                    public TableListBoxModel
{
public:
    SampleTable()
    {
        setModel (this);
        setHeaderHeight (24);
        getHeader().addColumn ("Note", 1, 100);
        getHeader().addColumn ("Name", 2, 100);

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

        for (auto* sample : filtered)
        {
            DBG("sample: " << sample->getNote());   
        }
        
        updateContent();
    }

    //=========================================================================
    int getNumRows() override { return filtered.size(); }

    void paintRowBackground (Graphics& g, int rowNumber,
                             int width, int height, bool rowIsSelected) override
    {
        if (rowIsSelected)
        {
            g.setColour (Colours::orange);
            g.fillAll();
        }
    }

    void paintCell (Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool rowIsSelected) override
    {
        g.setColour (Colours::white);
        if (auto* const sample = filtered [rowNumber])
        {
            String text = (columnId == 1) ? String (sample->getNote())
                : MidiMessage::getMidiNoteName (sample->getNote(), true, true, 4);
            g.drawText (text, 0, 0, width, height, Justification::centredLeft);
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
    content->setBounds (area);
}

void SamplesTableContentView::projectChanged()
{
    auto project = versicap.getProject();
    auto& table = content->table;
    table.setProject (project);
}

}
