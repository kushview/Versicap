
#include "Versicap.h"
#include "RenderContext.h"
#include "MainComponent.h"
#include "MainTabs.h"
#include "UnlockStatus.h"
#include "UnlockForm.h"

namespace vcp {

class MainComponent::Content : public Component
{
public:
    Content (MainComponent& o, Versicap& vc)
        : owner (o),
          tabs (vc)
    {
        setOpaque (true);
        
        addAndMakeVisible (clearButton);
        clearButton.setButtonText ("C");
        clearButton.onClick = [this]() { tabs.updateSettings (RenderContext()); };

        addAndMakeVisible (importButton);
        importButton.setButtonText ("I");
        importButton.onClick = [this]() {
            FileChooser chooser ("Open Versicap File", File(), "*.versicap", true, false, this);
            if (chooser.browseForFileToOpen())
            {
                auto ctx = tabs.getRenderContext();
                ctx.restoreFromFile (chooser.getResult());
                tabs.updateSettings (ctx);
            }
        };

        addAndMakeVisible (exportButton);
        exportButton.setButtonText ("S");
        exportButton.onClick = [this]() {
            FileChooser chooser ("Save Versicap File", File(), "*.versicap", true, false, this);
            if (chooser.browseForFileToSave(true))
            {
                auto ctx = tabs.getRenderContext();
                ctx.writeToFile (chooser.getResult());
            }
        };

        addAndMakeVisible (tabs);
        setSize (440, 340);
    }

    ~Content()
    {

    }

    void paint (Graphics& g) override
    {
        g.fillAll (kv::LookAndFeel_KV1::widgetBackgroundColor.darker());
    }

    void resized() override
    {
        auto r = getLocalBounds();
        r.removeFromTop (1);
        auto r2 = r.removeFromTop (18);
        Component* buttons [3] = {
            &clearButton, &importButton, &exportButton
        };

        r2.removeFromLeft (2);
        for (int i = 0; i < 3; ++i)
        {
            buttons[i]->setBounds (r2.removeFromLeft (20));
            r2.removeFromLeft (1);
        }

        r.removeFromTop (1);
        tabs.setBounds (r);
    }

    MainTabs& getTabs() { return tabs; }

private:
    MainComponent& owner;
    MainTabs tabs;
    TextButton clearButton;
    TextButton importButton;
    TextButton exportButton;
};

MainComponent::MainComponent (Versicap& vc)
    : versicap (vc)
{
    setOpaque (true);
    content.reset (new Content (*this, vc));
    addAndMakeVisible (content.get());
    setSize (600, 400);

    auto& tabs = content->getTabs();
    tabs.refresh();

    RenderContext ctx;
    File contextFile;
    if (auto* props = versicap.getSettings().getUserSettings())
    {
        tabs.setCurrentTabIndex (props->getIntValue ("currentTab", 0));
        contextFile = props->getFile().getParentDirectory().getChildFile("context.versicap");
        if (contextFile.existsAsFile())
            ctx.restoreFromFile (contextFile);
    }

    tabs.updateSettings (ctx);

    if (! (bool) versicap.getUnlockStatus().isUnlocked())
    {
        unlock = new UnlockForm (versicap.getUnlockStatus(), 
            "Sign in to your account to unlock Versicap", false, true);
        addAndMakeVisible (unlock.getComponent(), 9999);
    }
}

MainComponent::~MainComponent()
{
    content.reset();
    if (auto* overlay = unlock.getComponent())
        delete overlay;
}

void MainComponent::saveSettings()
{
    if (auto* props = versicap.getSettings().getUserSettings())
        props->setValue ("currentTab", content->getTabs().getCurrentTabIndex());
}

void MainComponent::saveContextFile()
{
    const auto ctx = content->getTabs().getRenderContext();
    File contextFile;

    if (auto* props = versicap.getSettings().getUserSettings())
        contextFile = props->getFile().getParentDirectory()
            .getChildFile ("context.versicap");

    if (! contextFile.getParentDirectory().exists())
        contextFile.getParentDirectory().createDirectory();

    ctx.writeToFile (contextFile);
}

void MainComponent::startRendering()
{
    auto& tabs = content->getTabs();
    auto ctx = tabs.getRenderContext();

    auto result = versicap.startRendering (ctx);
    if (result.failed())
    {
        AlertWindow::showNativeDialogBox ("Versicap", result.getErrorMessage(), false);
        return;
    }
}

void MainComponent::stopRendering()
{

}

void MainComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (DocumentWindow::backgroundColourId));
}

void MainComponent::resized()
{
    content->setBounds (getLocalBounds());
    if (auto* overlay = unlock.getComponent())
        overlay->setBounds (getLocalBounds());
}

}
