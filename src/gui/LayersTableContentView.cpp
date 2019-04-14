
#include "gui/LayersTableContentView.h"
#include "Project.h"

namespace vcp {

class LayerTable : public TableListBox,
                   public TableListBoxModel
{
public:
    LayerTable()
    {
        setModel (this);
    }

    ~LayerTable()
    {
        setModel (nullptr);
    }

    //=========================================================================
    void setProject (const Project& newProject)
    {
        project = newProject;
        updateContent();
    }

    //=========================================================================
    int getNumRows() override { return project.getNumLayers(); }

    void paintRowBackground (Graphics&, int rowNumber,
                             int width, int height, bool rowIsSelected) override
    {
        
    }

    void paintCell (Graphics&, int rowNumber, int columnId,
                    int width, int height, bool rowIsSelected) override
    {
        
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
    virtual void selectedRowsChanged (int lastRowSelected);
    virtual void deleteKeyPressed (int lastRowSelected);
    virtual void returnKeyPressed (int lastRowSelected);
    virtual void listWasScrolled();
    virtual var getDragSourceDescription (const SparseSet<int>& currentlySelectedRows);
   #endif
private:
    Project project;
};

class LayersTableContentView::Content : public Component
{
public:
    Content() { }
    ~Content() { }
};

LayersTableContentView::LayersTableContentView()
{
    content.reset (new Content());
    addAndMakeVisible (content.get());
}

LayersTableContentView::~LayersTableContentView()
{

}

void LayersTableContentView::resized()
{
    content->setBounds (getLocalBounds());
}

}
