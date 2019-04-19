
#include "gui/LayerPropertiesContentView.h"
#include "gui/LayersTableContentView.h"
#include "gui/NoteParams.h"
#include "gui/SamplePropertiesContentView.h"
#include "gui/SamplesTableContentView.h"
#include "gui/SampleEditContentView.h"
#include "gui/SourceContentView.h"

#include "gui/MainComponent.h"
#include "gui/MainTabs.h"
#include "gui/UnlockForm.h"

#include "Versicap.h"
#include "RenderContext.h"
#include "UnlockStatus.h"
#include "Utils.h"

#define VCP_DO_LOGO  1
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
        : owner (o),
          versicap (vc)
    {
        project = versicap.getProject();

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
            }
        };

        addAndMakeVisible (recordButton);
        recordButton.setButtonText ("Record");
        recordButton.onClick = [this]() { owner.startRendering(); };

        addAndMakeVisible (notes);
        
        source.reset (new SourceContentView (versicap));
        addAndMakeVisible (source.get());

        layers.reset (new LayersTableContentView (versicap));
        addAndMakeVisible (layers.get());

        samples.reset (new SamplesTableContentView (versicap));
        addAndMakeVisible (samples.get());

        view.reset (new SampleEditContentView (versicap));
        addAndMakeVisible (view.get());

        layer.reset (new LayerPropertiesContentView (versicap));
        addAndMakeVisible (layer.get());

        sample.reset (new SamplePropertiesContentView (versicap));
        addAndMakeVisible (sample.get());

        engine.reset (new EngineTabs (versicap));
        addAndMakeVisible (engine.get());

        keyboard.reset (new MidiKeyboardComponent (versicap.getMidiKeyboardState(), 
            MidiKeyboardComponent::horizontalKeyboard));
        keyboard->setKeyWidth (24);
        addAndMakeVisible (keyboard.get());

        logo = ImageCache::getFromMemory (BinaryData::versicap_v1_png,
                                          BinaryData::versicap_v1_pngSize);

        progress.onCancel = std::bind (&Versicap::stopRendering, &vc);
        setSize (440, 340);

        setProject (versicap.getProject());
    }

    ~Content()
    {
        if (auto* _unlock = unlock.getComponent())
            delete _unlock;
    }

    void paint (Graphics& g) override
    {
        g.fillAll (kv::LookAndFeel_KV1::widgetBackgroundColor.darker());
       #if VCP_DO_LOGO
        g.drawImageWithin (logo, 12, 6, 32, 32, RectanglePlacement::centred, false);
        g.setColour (kv::LookAndFeel_KV1::textColor);
        g.drawText (project.getProperty (Tags::name), 50, 6, 150, 32, Justification::centredLeft);
       #endif
    }

    void resized() override
    {
        auto r = getLocalBounds();
       #if VCP_DO_LOGO
        auto r2 = r.removeFromTop (40).reduced (0, 11);
       #else
        auto r2 = r.removeFromTop (18);
       #endif
        Component* buttons [3] = { &importButton, &exportButton, &recordButton };

        r2.removeFromRight (14);
        for (int i = 0; i < 3; ++i)
        {
            buttons[i]->setBounds (r2.removeFromRight (60));
            r2.removeFromRight (1);
        }

        notes.setBounds ((getWidth() / 2) - (notes.getRequiredWidth() / 2), 
                         r2.getY(), notes.getRequiredWidth(), r2.getHeight());

       #if 0
        keyboard->setBounds (r.removeFromBottom (60));
       #endif

        r.removeFromTop (1);
        r.removeFromLeft (6);
        r.removeFromRight (6);
        r.removeFromBottom (8);

        r2 = r.removeFromLeft (220);
        
        // source->setBounds (r2.removeFromTop (80));
        r2.removeFromTop (4);
        layers->setBounds (r2.removeFromTop (140));
        samples->setBounds (r2);

        auto r3 = r.removeFromRight (240);
        r3.removeFromTop (4);
        engine->setBounds (r3.removeFromBottom (260));
        sample->setBounds (r3.removeFromBottom (50));
        layer->setBounds (r3);

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

    Rectangle<int> getAlertBounds() const
    {
        return getLocalBounds().withSizeKeepingCentre (380, 180);
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

    void setProject (const Project& newProject)
    {
        project = newProject;
        notes.setProject (project);
    }

private:
    friend class MainComponent;
    MainComponent& owner;
    Versicap& versicap;
    Project project;
        
    std::unique_ptr<ContentView> view;
    std::unique_ptr<SourceContentView> source;
    std::unique_ptr<LayersTableContentView> layers;
    std::unique_ptr<SamplesTableContentView> samples;
    std::unique_ptr<LayerPropertiesContentView> layer;
    std::unique_ptr<SamplePropertiesContentView> sample;
    std::unique_ptr<EngineTabs> engine;

    TextButton importButton;
    TextButton exportButton;
    TextButton recordButton;
    RenderProgress progress;

    NoteParams notes;

    ComboBox sourceCombo;

    std::unique_ptr<MidiKeyboardComponent> keyboard;

    Component::SafePointer<UnlockForm> unlock;

    Image logo;

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

    auto& tabs = *content->engine;
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

//=============================================================================
void MainComponent::projectChanged()
{
    content->setProject (versicap.getProject());
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
        props->setValue ("currentTab", content->engine->getCurrentTabIndex());
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
