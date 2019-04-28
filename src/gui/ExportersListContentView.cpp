
#include "gui/ExportersListContentView.h"
#include "exporters/Exporter.h"
#include "ProjectWatcher.h"
#include "Versicap.h"

namespace vcp {

class ExportersList : public ListBox,
                      public ListBoxModel
{
public:
    ExportersList (Versicap& vc)
        : versicap (vc)
    {
        setModel (this);
    }

    ~ExportersList()
    {
        setModel (nullptr);
    }

    void setProject (const Project& p)
    {
        project = p;
        updateContent();
        selectRow (0);
    }

    int getNumRows() override { return project.getNumExporters(); }

    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height,
                           bool rowIsSelected) override
    {
        const auto exporter = project.getExporterData (rowNumber);

        if (rowIsSelected)
        {
            g.setOpacity (0.85);
            g.setColour (Colours::orange);
            g.fillAll();
        }

        g.setColour (rowIsSelected ? Colours::white : kv::LookAndFeel_KV1::textColor);
        auto text = exporter.getProperty (Tags::name, "Untitled Exporter").toString();
        g.drawText (text, 10, 0, width - 10, height, Justification::centredLeft);
    }

    void listBoxItemClicked (int row, const MouseEvent& ev) override
    {
        project.setActiveExporter (row);
        versicap.post (new DisplayObjectMessage (project.getExporterData (row)));

        if (ev.mods.isPopupMenu())
        {
            auto exporters = project.getExportersTree();
            PopupMenu menu;
            menu.addItem (1, "Remove Exporter");
            const auto result = menu.show();
            
            if (result == 1)
            {
                exporters.removeChild (row, nullptr);
                updateContent();
            }
        }
    }

   #if 0
    virtual Component* refreshComponentForRow (int rowNumber, bool isRowSelected,
                                               Component* existingComponentToUpdate);
    virtual void listBoxItemDoubleClicked (int row, const MouseEvent&);
    virtual void backgroundClicked (const MouseEvent&);
    virtual void selectedRowsChanged (int lastRowSelected);
    virtual void deleteKeyPressed (int lastRowSelected);
    virtual void returnKeyPressed (int lastRowSelected);
    virtual void listWasScrolled();
    virtual var getDragSourceDescription (const SparseSet<int>& rowsToDescribe);
    virtual String getTooltipForRow (int row);
    virtual MouseCursor getMouseCursorForRow (int row);
   #endif
private:
    Versicap& versicap;
    Project project;
};

class ExportersListContentView::Content : public Component,
                                          private Versicap::Listener
{
public:
    Content (ExportersListContentView& o, Versicap& vc)
        : versicap (vc), owner (o)
    {
        exporters.reset (new ExportersList (vc));
        addAndMakeVisible (exporters.get());
        addAndMakeVisible (addButton);
        addButton.setButtonText ("+");
        addButton.setTriggeredOnMouseDown (true);
        addButton.onClick = [this]()
        {
            PopupMenu menu;
            int i = 0;
            for (auto* const type : versicap.getExporterTypes())
                menu.addItem (++i, type->getName());
            menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&addButton),
                                ModalCallbackFunction::forComponent (&Content::handleMenuResult, this));
        };

        watcher.onExportersChanged = [this]() { exporters->updateContent(); };
        watcher.onChanged = [this]() { exporters->setProject (watcher.getProject()); };
        watcher.setProject (versicap.getProject());
        versicap.addListener (this);
    }

    ~Content()
    {
        versicap.removeListener (this);
        exporters.reset();
    }

    void resized() override
    {
        auto r = getLocalBounds();
        auto r2 = r.removeFromTop (18);
        r2 = r2.withHeight (18);
        addButton.setBounds (r2.removeFromLeft (24));
        exporters->setBounds (r);
    }

    static void handleMenuResult (int result, Content* content)
    {
        if (result > 0 && content != nullptr)
            content->handleMenuResult (result);
    }

private:
    friend class ExportersListContentView;
    Versicap& versicap;
    ProjectWatcher watcher;
    ExportersListContentView& owner;
    std::unique_ptr<ExportersList> exporters;
    TextButton addButton;

    void handleMenuResult (int result)
    {
        auto project = versicap.getProject();
        if (auto type = versicap.getExporterTypes()[result - 1])
        {
            project.addExporter (*type);
            exporters->selectRow (project.getNumExporters() - 1);
        }
    }

    void projectChanged() override
    {
        watcher.setProject (versicap.getProject());
    }
};

ExportersListContentView::ExportersListContentView (Versicap& vc)
    : ContentView (vc)
{
    setName ("Targets");
    content.reset (new Content (*this, vc));
    addAndMakeVisible (content.get());
}

ExportersListContentView::~ExportersListContentView()
{

}

void ExportersListContentView::resized()
{
    content->setBounds (getLocalBounds());
}

}
