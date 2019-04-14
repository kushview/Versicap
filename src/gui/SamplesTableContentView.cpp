
#include "gui/SamplesTableContentView.h"
#include "Project.h"

namespace vcp {

class SampleTable : public TableListBox,
                   public TableListBoxModel
{
public:
    SampleTable()
    {
        setModel (this);
    }

    ~SampleTable()
    {
        setModel (nullptr);
    }

    void refreshSamples()
    {
        filtered.clearQuick (true);
        // project.getSamples (filtered);
    }

    void setProject (const Project& newProject)
    {
        project = newProject;
    }

    //=========================================================================
    int getNumRows() override { 
        return 0;
    }

    void paintRowBackground (Graphics&, int rowNumber,
                             int width, int height, bool rowIsSelected) override
    {
        ignoreUnused (rowNumber, width, height, rowIsSelected);
    }

    void paintCell (Graphics&, int rowNumber, int columnId,
                    int width, int height, bool rowIsSelected) override
    {
        ignoreUnused (rowNumber, columnId, width, height, rowIsSelected);
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
    Layer layer;
    OwnedArray<Sample> filtered;
};

class SamplesTableContentView::Content : public Component
{
public:
    Content() { }
    ~Content() { }
};

SamplesTableContentView::SamplesTableContentView()
{
    content.reset (new Content());
    addAndMakeVisible (content.get());
}

SamplesTableContentView::~SamplesTableContentView()
{

}

void SamplesTableContentView::resized()
{
    content->setBounds (getLocalBounds());
}

}
