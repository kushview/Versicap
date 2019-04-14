

#include "gui/ContentView.h"
#include "gui/MainComponent.h"
#include "gui/MainTabs.h"
#include "gui/UnlockForm.h"

#include "Versicap.h"
#include "RenderContext.h"
#include "UnlockStatus.h"

namespace vcp {

class RenderProgress : public Component
{
public:
    RenderProgress()
        : progress (-1.0),
          bar (progress)
    {
        addAndMakeVisible (bar);

        addAndMakeVisible (text);
        text.setText ("Waiting...", dontSendNotification);
        text.setJustificationType (Justification::topLeft);
        text.setFont (Font (13.f));

        addAndMakeVisible (cancelButton);
        cancelButton.setButtonText ("Cancel");

        // shadow.setShadowProperties (DropShadow (Colours::black.withAlpha (0.7f), 6, Point<int> (0, 1)));
        // setComponentEffect (&shadow);

        setSize (300, 140);
    }

    void setProgressText (const String& newText)
    {
        jassert(newText.isNotEmpty());
        text.setText (newText, dontSendNotification);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (kv::LookAndFeel_KV1::widgetBackgroundColor);
        g.setColour (kv::LookAndFeel_KV1::widgetBackgroundColor.brighter());
        g.drawRect (getLocalBounds().toFloat(), 1.5);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        r = r.withWidth (jmax (240, r.getWidth()))
             .withHeight (jmax (140, r.getHeight()))
             .reduced (30, 0);

        r.removeFromTop (getHeight() / 3);
        bar.setBounds (r.removeFromTop (28));
        r.removeFromTop (8);
        text.setBounds (r.removeFromTop (30));
        
        int buttonSize = 26;
        r.removeFromBottom (30);
        cancelButton.changeWidthToFitText (buttonSize);
        cancelButton.setBounds (r.removeFromBottom (buttonSize)
                                 .withWidth (cancelButton.getWidth())
                                 .withX ((getWidth() / 2) - (cancelButton.getWidth() / 2)));
        cancelButton.onClick = [this]() { if (onCancel) onCancel(); };
    }

    std::function<void()> onCancel;

private:
    double progress;
    ProgressBar bar;
    Label text;
    TextButton cancelButton;
    DropShadowEffect shadow;
};

class MainComponent::Content : public Component
{
public:
    Content (MainComponent& o, Versicap& vc)
        : versicap (vc),
          owner (o),
          tabs (vc)
    {
        setOpaque (true);

        addAndMakeVisible (importButton);
        importButton.setButtonText ("Open");
        importButton.onClick = [this]()
        {
            FileChooser chooser ("Open Versicap File", Versicap::getPresetsPath(), 
                "*.versicap", true, false, this);
            if (chooser.browseForFileToOpen())
            {
                versicap.loadProject (chooser.getResult());
            }
        };

        addAndMakeVisible (exportButton);
        exportButton.setButtonText ("Save");
        exportButton.onClick = [this]()
        {
            FileChooser chooser ("Save Versicap File", Versicap::getPresetsPath(), 
                "*.versicap", true, false, this);
            if (chooser.browseForFileToSave (true))
            {
                versicap.saveProject (chooser.getResult());
                DBG(versicap.getProject().getValueTree().toXmlString());
            }
        };

        addAndMakeVisible (recordButton);
        recordButton.setButtonText ("Record");
        recordButton.onClick = [this]() { owner.startRendering(); };

        addAndMakeVisible (tabs);

        view.reset (new ContentView());
        addAndMakeVisible (view.get());

        progress.onCancel = std::bind (&Versicap::stopRendering, &vc);
        setSize (440, 340);
    }

    ~Content()
    {
        if (auto* _unlock = unlock.getComponent())
            delete _unlock;
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
        Component* buttons [2] = { &importButton, &exportButton };

        r2.removeFromLeft (2);
        for (int i = 0; i < 2; ++i)
        {
            buttons[i]->setBounds (r2.removeFromLeft (60));
            r2.removeFromLeft (1);
        }

        recordButton.setBounds (r2.removeFromRight (60));

        r.removeFromTop (1);
        tabs.setBounds (r.removeFromLeft (240));
        r.removeFromLeft (2);
        view->setBounds (r);

        if (overlay.isVisible())
        {
            overlay.setBounds (getLocalBounds());
        }

        if (progress.isVisible())
        {
            progress.setBounds (getAlertBounds());
        }

        if (unlock != nullptr && unlock->isVisible())
        {
            unlock->setBounds (getAlertBounds());
        }
    }

    MainTabs& getTabs() { return tabs; }

    Rectangle<int> getAlertBounds() const
    {
        return getLocalBounds().reduced (40, 60);
    }

    void showProgress (bool showIt)
    {
        showOverlay (showIt);

        if (showIt)
        {
            addAndMakeVisible (progress, 9999);
        }
        else
        {
            removeChildComponent (&progress);
        }

        resized();
    }

    void showOverlay (bool showIt)
    {
        if (showIt)
        {
            addAndMakeVisible (overlay, 9998);
        }
        else
        {
            removeChildComponent (&overlay);
        }

        resized();
    }

    void showUnlockForm()
    {
        showOverlay (true);
        unlock = new UnlockForm (versicap.getUnlockStatus(), 
            "Sign in to your account to unlock Versicap", false, true);
        addAndMakeVisible (unlock.getComponent(), 10000);
        resized();
    }

    RenderProgress& getRenderProgress() { return progress; }

private:
    Versicap& versicap;
    MainComponent& owner;
    MainTabs tabs;
    std::unique_ptr<ContentView> view;
    
    TextButton importButton;
    TextButton exportButton;
    TextButton recordButton;
    RenderProgress progress;

    Component::SafePointer<UnlockForm> unlock;

    struct Overlay : public Component
    {
        Overlay()
        {
            setInterceptsMouseClicks (true, false);
        }

        bool hitTest (int, int) override
        {
            return true;
        }

        void paint (Graphics& g) override
        {
            g.setColour (Colours::black);
            g.setOpacity (0.24f);
            g.fillAll();
        }

    } overlay;
};

MainComponent::MainComponent (Versicap& vc)
    : ContentComponent (vc),
      versicap (vc)
{
    setOpaque (true);
    content.reset (new Content (*this, vc));
    addAndMakeVisible (content.get());
    setSize (600, 400);

    auto& tabs = content->getTabs();
    tabs.refresh();

    if (auto* props = versicap.getSettings().getUserSettings())
        tabs.setCurrentTabIndex (props->getIntValue ("currentTab", 0));

    tabs.updateSettings();

    versicap.addListener (this);
    versicap.getUnlockStatus().addChangeListener (this);
    if (! (bool) versicap.getUnlockStatus().isUnlocked())
        content->showUnlockForm();
}

MainComponent::~MainComponent()
{
    const auto ctx = content->getTabs().getRenderContext();
    versicap.setRenderContext (ctx);
    versicap.removeListener (this);
    versicap.getUnlockStatus().removeChangeListener (this);
    content.reset();
}

void MainComponent::changeListenerCallback (ChangeBroadcaster* bcaster)
{
    if (bcaster == &versicap.getUnlockStatus())
    {
        if ((bool) versicap.getUnlockStatus().isUnlocked())
        {
            content->showOverlay (false);
        }
        else
        {
            content->showUnlockForm();
        }
    }
}

void MainComponent::renderWillStart()
{
    auto& progress = content->getRenderProgress();
    progress.setProgressText ("Waiting...");
    content->showProgress (true);
}

void MainComponent::renderStarted()
{
    auto& progress = content->getRenderProgress();
    progress.setProgressText ("Rendering...");
}

void MainComponent::renderWillStop()
{
    content->showProgress (false);
}

void MainComponent::saveSettings()
{
    if (auto* props = versicap.getSettings().getUserSettings())
        props->setValue ("currentTab", content->getTabs().getCurrentTabIndex());
}

void MainComponent::startRendering()
{
    auto result = versicap.startRendering();
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
}

}
