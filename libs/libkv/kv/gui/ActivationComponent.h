
#pragma once

namespace kv {

class ActivationComponent  : public Component,
                             public FileDragAndDropTarget,
                             private Timer,
                             public Button::Listener
{
public:
    //==============================================================================
    ActivationComponent (UnlockStatus& _status);
    ~ActivationComponent();

    void setForTrial (bool setupForTrial);
    void setForRegistration (bool setupRegistration);
    void setForManagement (bool setupManagement);
    
    void setQuitButtonTextForTrial (const String& text)
    {
        trialQuitButtonText = text;
        if (isForTrial)
            quitButton->setButtonText (trialQuitButtonText);
    }

    void visibilityChanged() override;
    void timerCallback() override;
    void setBackgroundColour (const Colour& color) { backgroundColour = color; repaint(); }
    void setOverlayOpacity (float opacity) { overlayOpacity = jlimit (0.f, 1.f, opacity); }
    void setOverlayShowText (bool showIt) { overlayShowText = showIt; }

    bool isInterestedInFileDrag (const StringArray& files) override;
    void filesDropped (const StringArray& files, int x, int y) override;

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;

private:
    String activateInstructions;
    Colour backgroundColour;
    UnlockStatus& status;
    std::unique_ptr<Component> unlock;

    double progress = 0.0;
    float overlayOpacity = 0.72f;
    bool overlayShowText = true;
    bool grabbedFirstFocus = false;
    
    bool isForTrial = false;
    bool isForRegistration = false;
    bool isForManagement = false;

    String textBeforeReg;
    Label emailLabel { "Email"};
    TextEditor email;
    Label userNameLabel { "Username" };
    TextEditor username;
    Label passwordLabel { "Password" };
    TextEditor password;
    ProgressBar progressBar;    
    std::unique_ptr<TextButton> syncButton;
    String trialQuitButtonText = "Continue";
    TextButton copyMachineButton;
    
    std::unique_ptr<Label> appNameLabel;
    std::unique_ptr<HyperlinkButton> onlineActivateLink;
    std::unique_ptr<Label> instructionLabel;
    std::unique_ptr<TextEditor> licenseKey;
    std::unique_ptr<TextButton> activateButton;
    std::unique_ptr<TextButton> quitButton;
    std::unique_ptr<HyperlinkButton> myLicenseLink;
    std::unique_ptr<HyperlinkButton> getLicenseLink;
    std::unique_ptr<HyperlinkButton> registerTrialLink;
    std::unique_ptr<Label> instructionLabel2;
    std::unique_ptr<ToggleButton> deactivateOthers;

    void handleRefreshResult (const UnlockStatus::UnlockResult result, UnlockOverlay::Action);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ActivationComponent)
};

class ActivationDialog : public DialogWindow
{
public:
    ActivationDialog (UnlockStatus& _status, std::unique_ptr<Component>& o)
        : DialogWindow ("Activation", Colours::black, true, false),
          owner (o), status (_status)
    {
        owner.reset (this);
        setEnabled (true);        
        setUsingNativeTitleBar (true);

        auto* const activation = new ActivationComponent (status);
        setContentOwned (activation, true);
        activation->resized(); // < make sure buttons are positioned appropriately
        
        setAlwaysOnTop (true);
        addToDesktop();
        setVisible (true);
    }

    void closeButtonPressed() override
    {
        owner.reset();
    }

private:
    std::unique_ptr<Component>& owner;
    UnlockStatus& status;
};

} // namespace kv
