
#include "gui/LayersTableContentView.h"
#include "ProjectWatcher.h"
#include "Versicap.h"

namespace vcp {

class LayerTable : public TableListBox,
                   public TableListBoxModel
{
public:
    LayerTable()
    {
        setModel (this);
        getHeader().addColumn ("Name", 1, 100);
        setHeaderHeight (0);

        watcher.onChanged = [this]() {
            updateContent();
        };

        watcher.onLayerAdded = [this]() {
            updateContent();
        };

        watcher.onLayerRemoved = [this]() {
            updateContent();
        };
    }   

    ~LayerTable()
    {
        setModel (nullptr);
    }

    //=========================================================================
    void setProject (const Project& newProject) { watcher.setProject (newProject); }
    Project getProject() const { return watcher.getProject(); }

    //=========================================================================
    int getNumRows() override { return watcher.getProject().getNumLayers(); }

    void paintRowBackground (Graphics& g, int rowNumber,
                             int width, int height, 
                             bool rowIsSelected) override
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
        const auto project = watcher.getProject();
        const auto layer = project.getLayer (rowNumber);
        g.setColour (Colours::white);
        auto text = layer.getProperty (Tags::name).toString();
        if (text.isEmpty())
            text = "Layer " + String (1 + project.indexOf (layer));

        g.drawText (text,
                    0, 0, width, height, 
                    Justification::centredLeft);
    }

    void selectedRowsChanged (int lastRowSelected) override
    {
        auto project = watcher.getProject();
        const auto layer = project.getLayer (lastRowSelected);
        project.setActiveLayer (layer);
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
};

class LayersTableContentView::Content : public Component
{
public:
    Content()
    { 
        addAndMakeVisible (addButton);
        addButton.setButtonText ("A");
        addButton.onClick = [this]()
        {
            auto project = table.getProject();
            auto layer = project.addLayer();
            table.selectRow (project.indexOf (layer));
        };

        addAndMakeVisible (table);
    }

    ~Content() { }

    void paint (Graphics& g) override
    {
        g.fillAll (kv::LookAndFeel_KV1::widgetBackgroundColor);
    }

    void resized() override
    {
        auto r1 = getLocalBounds();
        auto r2 = r1.removeFromTop (22);
        addButton.setBounds (r2.removeFromLeft (24));
        r1.removeFromTop (2);
        table.setBounds (r1);
    }

private:
    friend class LayersTableContentView;
    LayerTable table;
    TextButton addButton;
};

LayersTableContentView::LayersTableContentView (Versicap& vc)
    : ContentView (vc)
{
    content.reset (new Content());
    addAndMakeVisible (content.get());
    auto& table = content->table;
    table.setProject (vc.getProject());
}

LayersTableContentView::~LayersTableContentView()
{

}

void LayersTableContentView::projectChanged()
{
    auto project = versicap.getProject();
    auto& table = content->table;
    table.setProject (project);
}

void LayersTableContentView::resized()
{
    content->setBounds (getLocalBounds());
}

}
