
#include <kv/kv.h>

#if ! defined (KV_ACTIVATION_INSTRUCTIONS)
 #define KV_ACTIVATION_INSTRUCTIONS      "APPNAME requires activation to run.\n" \
                                         "Please enter your license key for APPNAME."
#endif

namespace kv {

ActivationComponent::ActivationComponent (UnlockStatus& _status)
    : status (_status), progressBar (progress)
{
    appNameLabel.reset (new Label ("AppNameLabel", TRANS("ELEMENT")));
    addAndMakeVisible (appNameLabel.get());
    appNameLabel->setExplicitFocusOrder (7);
    appNameLabel->setFont (Font (44.00f, Font::plain).withTypefaceStyle ("Regular"));
    appNameLabel->setJustificationType (Justification::centred);
    appNameLabel->setEditable (false, false, false);
    appNameLabel->setColour (TextEditor::textColourId, Colours::black);
    appNameLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    onlineActivateLink.reset (new HyperlinkButton (TRANS("Online Activation"), 
                                                   URL ("EL_URL_HELP_ACTIVATION")));
    addAndMakeVisible (onlineActivateLink.get());
    onlineActivateLink->setTooltip (TRANS("EL_URL_HELP_ACTIVATION"));
    onlineActivateLink->setExplicitFocusOrder (8);
    onlineActivateLink->setButtonText (TRANS("Online Activation"));

    instructionLabel.reset (new Label ("InstructionLabel",
                                       TRANS("APPNAME requires activation to run.  \n"
                                       "Please enter your FULL or TRIAL license key for APPNAME.  ")));
    addAndMakeVisible (instructionLabel.get());
    instructionLabel->setExplicitFocusOrder (9);
    instructionLabel->setFont (Font (14.00f, Font::plain).withTypefaceStyle ("Regular"));
    instructionLabel->setJustificationType (Justification::centred);
    instructionLabel->setEditable (false, false, false);
    instructionLabel->setColour (TextEditor::textColourId, Colours::black);
    instructionLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    licenseKey.reset (new TextEditor ("LicenseKey"));
    addAndMakeVisible (licenseKey.get());
    licenseKey->setExplicitFocusOrder (1);
    licenseKey->setMultiLine (false);
    licenseKey->setReturnKeyStartsNewLine (false);
    licenseKey->setReadOnly (false);
    licenseKey->setScrollbarsShown (true);
    licenseKey->setCaretVisible (true);
    licenseKey->setPopupMenuEnabled (false);
    licenseKey->setText (String());

    activateButton.reset (new TextButton ("ActivateButton"));
    addAndMakeVisible (activateButton.get());
    activateButton->setExplicitFocusOrder (2);
    activateButton->setButtonText (TRANS("Activate"));
    activateButton->addListener (this);

    quitButton.reset (new TextButton ("QuitButton"));
    addAndMakeVisible (quitButton.get());
    quitButton->setExplicitFocusOrder (3);
    quitButton->setButtonText (TRANS("Start Trial"));
    quitButton->addListener (this);

    myLicenseLink.reset (new HyperlinkButton (TRANS("My Licenses"),
                                              URL ("http://www.juce.com")));
    addAndMakeVisible (myLicenseLink.get());
    myLicenseLink->setTooltip (TRANS("http://www.juce.com"));
    myLicenseLink->setExplicitFocusOrder (4);
    myLicenseLink->setButtonText (TRANS("My Licenses"));

    getLicenseLink.reset (new HyperlinkButton (TRANS("Get A License"),
                                               URL ("https://kushview.net/element/purchase")));
    addAndMakeVisible (getLicenseLink.get());
    getLicenseLink->setTooltip (TRANS("https://kushview.net/element/purchase"));
    getLicenseLink->setExplicitFocusOrder (5);
    getLicenseLink->setButtonText (TRANS("Get A License"));

    registerTrialLink.reset (new HyperlinkButton (TRANS("Register For Trial"),
                                                  URL ("https://kushview.net/element/trial")));
    addAndMakeVisible (registerTrialLink.get());
    registerTrialLink->setTooltip (TRANS("https://kushview.net/element/trial"));
    registerTrialLink->setExplicitFocusOrder (6);
    registerTrialLink->setButtonText (TRANS("Register For Trial"));

    instructionLabel2.reset (new Label ("InstructionLabel",
                                        TRANS("Your license key will be emailed to you upon sucessful registration. Fake credentials won\'t cut it.")));
    addAndMakeVisible (instructionLabel2.get());
    instructionLabel2->setExplicitFocusOrder (9);
    instructionLabel2->setFont (Font (14.00f, Font::italic));
    instructionLabel2->setJustificationType (Justification::centred);
    instructionLabel2->setEditable (false, false, false);
    instructionLabel2->setColour (Label::textColourId, Colour (0xfff2f2f2));
    instructionLabel2->setColour (TextEditor::textColourId, Colours::black);
    instructionLabel2->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    deactivateOthers.reset (new ToggleButton ("DeactivateOthers"));
    addAndMakeVisible (deactivateOthers.get());
    deactivateOthers->setButtonText (TRANS("Deactivate other machines?"));

    appNameLabel->setText ("The Application", dontSendNotification);
    instructionLabel->setText (
        instructionLabel->getText().replace ("APPNAME", "The Application"),
        dontSendNotification);
    activateInstructions = instructionLabel->getText();

    licenseKey->setWantsKeyboardFocus (true);
    const auto key = status.getLicenseKey();
    if (key.isNotEmpty())
        licenseKey->setText (key, dontSendNotification);

    instructionLabel2->setVisible (false);
    registerTrialLink->setVisible (false);
    onlineActivateLink->setURL (URL ("EL_URL_HELP_ACTIVATION"));
    myLicenseLink->setURL (URL ("EL_URL_MY_LICENSES"));
    getLicenseLink->setURL (URL ("EL_URL_ELEMENT_PURCHASE"));
    registerTrialLink->setURL (URL ("EL_URL_ELEMENT_GET_TRIAL"));

    password.setPasswordCharacter (kv::defaultPasswordChar());

    syncButton.reset(new TextButton ("Refresh"));
    // TODO: syncButton->setIcon (Icon (getIcons().farSyncAlt, LookAndFeel::textColor));
    syncButton->addListener (this);

    // TODO: copyMachineButton.setIcon (Icon (getIcons().falCopy,
    //     findColour (TextButton::textColourOffId)));
    copyMachineButton.addListener (this);
    copyMachineButton.setTooltip (TRANS ("Copy your machine ID to the clip board"));
    addAndMakeVisible (copyMachineButton);

    quitButton->setButtonText ("Quit");

    setSize (480, 346);

    setBackgroundColour (findColour (DocumentWindow::backgroundColourId));
    startTimer (250);
}

ActivationComponent::~ActivationComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    syncButton->removeListener (this);
    syncButton = nullptr;
    //[/Destructor_pre]

    appNameLabel = nullptr;
    onlineActivateLink = nullptr;
    instructionLabel = nullptr;
    licenseKey = nullptr;
    activateButton = nullptr;
    quitButton = nullptr;
    myLicenseLink = nullptr;
    getLicenseLink = nullptr;
    registerTrialLink = nullptr;
    instructionLabel2 = nullptr;
    deactivateOthers = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}


void ActivationComponent::setAppName (const String& newName)
{
    appName = newName;
    appNameLabel->setText (appName, dontSendNotification);
    instructionLabel->setText (
        instructionLabel->getText().replace ("APPNAME", appName),
        dontSendNotification);
    activateInstructions = instructionLabel->getText();
}

//==============================================================================
void ActivationComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xff323e44));

    //[UserPaint] Add your own custom painting code here..
    g.fillAll (backgroundColour);
    //[/UserPaint]
}

void ActivationComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    appNameLabel->setBounds ((getWidth() / 2) - (320 / 2), 37, 320, 48);
    onlineActivateLink->setBounds ((getWidth() / 2) - (108 / 2), 94, 108, 18);
    instructionLabel->setBounds ((getWidth() / 2) - (352 / 2), 124, 352, 39);
    licenseKey->setBounds ((getWidth() / 2) - (260 / 2), 169, 260, 22);
    activateButton->setBounds (((getWidth() / 2) - (260 / 2)) + 260 / 2 + -3 - 90, 204, 90, 24);
    quitButton->setBounds (((getWidth() / 2) - (260 / 2)) + 260 / 2 + 3, 204, 90, 24);
    myLicenseLink->setBounds ((getWidth() / 2) - (96 / 2), 256, 96, 18);
    getLicenseLink->setBounds ((getWidth() / 2) - (94 / 2), 279, 94, 18);
    registerTrialLink->setBounds ((getWidth() / 2) - (100 / 2), 300, 100, 18);
    instructionLabel2->setBounds ((getWidth() / 2) - (280 / 2), 240, 280, 64);
    deactivateOthers->setBounds ((getWidth() / 2) - (183 / 2), 234, 183, 18);
    //[UserResized] Add your own custom resize handling here..

   #if EL_RUNNING_AS_PLUGIN
    if (! quitButton->isVisible())
    {
        activateButton->setBounds (activateButton->getBoundsInParent()
                .withX (activateButton->getX() + (activateButton->getWidth() / 2)));
    }
   #endif

    if (nullptr != findParentComponentOfClass<ActivationDialog>())
    {
        copyMachineButton.setBounds (getWidth() - 34, getHeight() - 32,
                                     24, 22);
    }
    else
    {
        copyMachineButton.setBounds (getWidth() - 94, getHeight() - 32,
                                     24, 22);
    }

    if (syncButton && syncButton->isVisible())
    {
        int shiftLeft = activateButton->getHeight() / 2;
        activateButton->setBounds (activateButton->getBoundsInParent()
            .withX (activateButton->getX() - shiftLeft));
        quitButton->setBounds (quitButton->getBoundsInParent()
            .withX (quitButton->getX() - shiftLeft));
        syncButton->setBounds (quitButton->getRight() + 6, quitButton->getY(),
                               quitButton->getHeight(), quitButton->getHeight());
    }

    if (isForRegistration)
    {
        instructionLabel->setBounds (
            instructionLabel->getBoundsInParent().withY (
                instructionLabel->getBoundsInParent().getY() - 40));

        Rectangle<int> regBox = {
            licenseKey->getX(),
            instructionLabel->getBottom(),
            licenseKey->getWidth(),
            jmax(24 * 3, quitButton->getY() - instructionLabel->getBottom())
        };

        auto leftSlice = regBox.removeFromLeft (90);

        emailLabel.setBounds (leftSlice.removeFromTop (24));
        userNameLabel.setBounds (leftSlice.removeFromTop (24));
        passwordLabel.setBounds (leftSlice.removeFromTop (24));

        email.setBounds (regBox.removeFromTop (22));
        regBox.removeFromTop (3);
        username.setBounds (regBox.removeFromTop (22));
        regBox.removeFromTop (3);
        password.setBounds (regBox.removeFromTop (22));
    }

    if (progressBar.isVisible())
    {
        progressBar.setBounds (licenseKey->getBounds().expanded (30, 2));
    }

    if (unlock)
        unlock->setBounds (getLocalBounds());
    //[/UserResized]
}

void ActivationComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    auto* const dialog = findParentComponentOfClass<DialogWindow>();
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == activateButton.get())
    {
        //[UserButtonCode_activateButton] -- add your button handler code here..
        if (isForManagement)
        {
            auto* unlockRef = new UnlockOverlay (unlock,
                status, UnlockOverlay::Deactivate,
                status.getLicenseKey().trim());

            unlockRef->setOpacity (overlayOpacity);
            unlockRef->setShowText (overlayShowText);
            unlockRef->onFinished = [this](const UnlockStatus::UnlockResult result, UnlockOverlay::Action)
            {
                if (result.succeeded)
                {
                    auto& _status = status;
                    setForManagement (false);
                    _status.sendChangeMessage();
                }
            };
            addAndMakeVisible (unlock.get());
        }
        else if (isForTrial)
        {
            #if 0
            if (EL_IS_TRIAL_EXPIRED (status))
            {
                URL purchaseUrl (EL_URL_ELEMENT_PURCHASE);
                purchaseUrl.launchInDefaultBrowser();
            }
            else
            {
                auto theLink = String(EL_URL_LICENSE_UPGRADES)
                    .replace("LICENSE_ID", status.getLicenseID().toString().trim())
                    .replace("PAYMENT_ID", status.getPaymentID().toString().trim());
                if (URL::isProbablyAWebsiteURL (theLink))
                    URL(theLink).launchInDefaultBrowser();
            }
            #endif
        }
        else if (isForRegistration)
        {
            if (email.isEmpty() || username.isEmpty() || password.isEmpty())
                return;

            if (! URL::isProbablyAnEmailAddress (email.getText()))
            {
                AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                    "Invalid Email", "Please enter a valid email address");
                return;
            }

            if (password.getText().length() < 8)
            {
                AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                    "Password to Short", "Please enter a password with 8 or more characters");
                return;
            }

           #if 0 // ! EL_USE_LOCAL_AUTH
            if (Util::isGmailExtended (email.getText()))
            {
                AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                    "Invalid Email", "Gmail extended addresses are not permitted in registration");
                return;
            }
           #endif

            auto* unlockRef = new UnlockOverlay (unlock,
                status, UnlockOverlay::Register,
                String(), email.getText(),
                username.getText(), password.getText()
            );
            unlockRef->setOpacity (overlayOpacity);
            unlockRef->setShowText (overlayShowText);
            unlockRef->onFinished = [this](const UnlockStatus::UnlockResult result, UnlockOverlay::Action)
            {
                #if 0
                auto& status = gui.getWorld().getUnlockStatus();
                if (result.succeeded)
                {
                    bool shouldBail = false;
                    // AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                    //                                 "Trial Activation",
                    //                                 result.informativeMessage);
                    if (EL_IS_TRIAL_NOT_EXPIRED(status) ||
                        EL_IS_TRIAL_EXPIRED(status))
                    {
                        setForRegistration (false);
                        isForTrial = false;
                        setForTrial (true);
                    }
                    else if (EL_IS_NOT_ACTIVATED(status))
                    {
                        setForRegistration (false);
                    }
                    else
                    {
                        setForRegistration (false);
                        if (auto* dialog = findParentComponentOfClass<ActivationDialog>()) {
                            dialog->closeButtonPressed();
                            shouldBail = true;
                        }
                    }
                    status.refreshed();
                    if (shouldBail)
                        return;
                }
                else
                {
                    // AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                    //                                 "Trial Activation",
                    //                                 result.errorMessage);
                }
                #endif
            };
            addAndMakeVisible (unlock.get());
        }
        else
        {
            if (licenseKey->isEmpty())
                return;
            auto* unlockRef = new UnlockOverlay (unlock,
                status, UnlockOverlay::Activate,
                licenseKey->getText().trim(),
                String(), String(), String(),
                deactivateOthers->getToggleState()
            );
            unlockRef->setOpacity (overlayOpacity);
            unlockRef->setShowText (overlayShowText);
            unlockRef->onFinished = [this](const UnlockStatus::UnlockResult result, UnlockOverlay::Action)
            {
                auto* const dialog = findParentComponentOfClass<DialogWindow>();
                if (result.succeeded)
                {
                    auto& _status = status;
                   #if 0
                    if (EL_IS_TRIAL_EXPIRED(_status) ||
                        EL_IS_TRIAL_NOT_EXPIRED(_status))
                    {
                        // not in a dialog so want the manage option
                        if (nullptr == findParentComponentOfClass<ActivationDialog>())
                            setQuitButtonTextForTrial ("Manage");

                        isForTrial = false;
                        setForTrial (true);
                        resized();
                    }
                    else 
                   #endif
                    if (closeDialogWhenFinished && nullptr != dialog)
                    {
                        // if in the dialog, close it
                        dialog->closeButtonPressed();
                    }
                    else
                    {
                        // otherwise go to management
                        setForManagement (true);
                    }

                    _status.sendChangeMessage();
                }
            };

            addAndMakeVisible (unlock.get());
        }

        resized();
    }
    else if (buttonThatWasClicked == quitButton.get())
    {
        //[UserButtonCode_quitButton] -- add your button handler code here..
        if (isForManagement)
        {
            status.saveState (String());
            // TODO?: gui.getWorld().getSettings().saveIfNeeded();
            status.loadAll();
            status.sendChangeMessage();
            licenseKey->setText (String(), dontSendNotification);
            isForTrial = false;
            progressBar.setVisible (false);
            setForManagement (false);
        }
        else if (isForTrial)
        {
            if (quitButton->getButtonText() == "Quit")
            {
                JUCEApplication::getInstance()->systemRequestedQuit();
            }
            else if (quitButton->getButtonText() == "Continue")
            {
                if (auto* dialog = findParentComponentOfClass<ActivationDialog>())
                    dialog->closeButtonPressed();
            }
            else if (quitButton->getButtonText() == "Manage")
            {
                isForTrial = false;
                progressBar.setVisible (false);
                setForManagement (true);
            }
            else
            {
                jassertfalse; // unhandled quit button action
                if (auto* dialog = findParentComponentOfClass<ActivationDialog>())
                    dialog->closeButtonPressed();
            }
        }
        else if (isForRegistration)
        {
            setForRegistration (false);
        }
        else
        {
            if (quitButton->getButtonText() == "Quit")
                JUCEApplication::getInstance()->systemRequestedQuit();
            else if (dialog != nullptr && quitButton->getButtonText() == "Continue")
                dialog->closeButtonPressed();
            else
                setForRegistration (true);
           
            JUCEApplication::getInstance()->systemRequestedQuit();
        }
        //[/UserButtonCode_quitButton]
    }

    //[UserbuttonClicked_Post]
    else if (buttonThatWasClicked == &copyMachineButton)
    {
        const auto machine = status.getLocalMachineIDs()[0];
        SystemClipboard::copyTextToClipboard (machine);
    }
    else if (buttonThatWasClicked == syncButton.get())
    {
        auto* const unlockRef = new UnlockOverlay (unlock,
            status, UnlockOverlay::Check,
            status.getLicenseKey());
        unlockRef->setOpacity (overlayOpacity);
        unlockRef->setShowText (overlayShowText);
        unlockRef->onFinished = std::bind (&ActivationComponent::handleRefreshResult, this,
                                           std::placeholders::_1, std::placeholders::_2);
        addAndMakeVisible (unlock.get());
        resized();
    }
    //[/UserbuttonClicked_Post]
}

void ActivationComponent::visibilityChanged() { }

void ActivationComponent::setForRegistration (bool setupRegistration)
{
    if (isForRegistration == setupRegistration)
        return;
    isForRegistration = setupRegistration;

    if (isForRegistration)
    {
        copyMachineButton.setVisible (false);
        onlineActivateLink->setVisible(false);
        licenseKey->setVisible (false);
        deactivateOthers->setVisible (false);
        instructionLabel2->setVisible (true);
        myLicenseLink->setVisible (false);
        getLicenseLink->setVisible (false);
        activateButton->setButtonText ("Register");
        quitButton->setButtonText ("Cancel");
        textBeforeReg = instructionLabel->getText();
        instructionLabel->setText (
            String("Activate your APPNAME trial by registering on kushview.net").replace ("APPNAME", "The Application"),
            dontSendNotification);
        addAndMakeVisible (emailLabel);
        emailLabel.setText ("Email", dontSendNotification);
        emailLabel.setJustificationType (Justification::centredLeft);
        emailLabel.setFont (Font (13.f));
        addAndMakeVisible (email);

        addAndMakeVisible (userNameLabel);
        userNameLabel.setText ("Username", dontSendNotification);
        userNameLabel.setJustificationType (Justification::centredLeft);
        userNameLabel.setFont (Font (13.f));
        addAndMakeVisible (username);

        addAndMakeVisible (passwordLabel);
        passwordLabel.setText ("Password", dontSendNotification);
        passwordLabel.setJustificationType (Justification::centredLeft);
        passwordLabel.setFont (Font (13.f));
        addAndMakeVisible (password);
        email.grabKeyboardFocus();
    }
    else
    {
        copyMachineButton.setVisible (true);
        instructionLabel->setText (textBeforeReg, dontSendNotification);
        licenseKey->setVisible (true);
        deactivateOthers->setVisible (true);
        onlineActivateLink->setVisible (true);
        instructionLabel2->setVisible (false);
        myLicenseLink->setVisible (true);
        getLicenseLink->setVisible (true);
        activateButton->setButtonText ("Activate");
        quitButton->setButtonText ("Start Trial");
        licenseKey->grabKeyboardFocus();
        removeChildComponent (&emailLabel);
        removeChildComponent (&email);
        removeChildComponent (&userNameLabel);
        removeChildComponent (&username);
        removeChildComponent (&passwordLabel);
        removeChildComponent (&password);
    }

    resized();
    repaint();
}

void ActivationComponent::setForTrial (bool setupForTrial)
{
   #if 0
    if (! status.isTrial())
    {
        jassertfalse; // not a trial???
        return;
    }

    if (isForTrial == setupForTrial || setupForTrial == false)
        return;
    isForTrial = setupForTrial;
    licenseKey->setVisible (false);
    deactivateOthers->setVisible (false);
    registerTrialLink->setVisible (false);
    copyMachineButton.setVisible (false);
    addAndMakeVisible (progressBar);
    addAndMakeVisible (syncButton.get());
    progressBar.periodDays = status.getExpirationPeriodDays();

    if (EL_IS_TRIAL_EXPIRED (status))
    {
        progress = 1.0;
        activateButton->setButtonText ("Purchase");
        quitButton->setButtonText (trialQuitButtonText.isNotEmpty() ? trialQuitButtonText : "Quit");
        instructionLabel->setText (
            String("Your trial license for APPNAME has expired. Use the button "
            "below to purchase a license.").replace("APPNAME", Util::appName()),
            dontSendNotification);
    }
    else
    {
        auto remaining = static_cast<double> (status.getExpiryTime().toMilliseconds() - Time::getCurrentTime().toMilliseconds());
        //auto period = static_cast<double> (status.getExpiryTime().toMilliseconds() - status.getCreationTime().toMilliseconds());
        auto period = static_cast<double> (RelativeTime::days(progressBar.periodDays).inMilliseconds());
        progress = (period - remaining) / period;
        progress = jlimit(0.0, 0.9999, progress);
        activateButton->setButtonText ("Upgrade");
        quitButton->setButtonText (trialQuitButtonText.isNotEmpty() ? trialQuitButtonText : "Continue");
        instructionLabel->setText (
            String("We hope you're enjoying APPNAME! Upgrade your trial license before expiration "
            "for a discounted price.").replace("APPNAME", Util::appName()),
            dontSendNotification);
    }
   #endif

    resized();
}

void ActivationComponent::setForManagement (bool setupManagement)
{
    isForManagement = setupManagement;
    if (isForManagement)
    {
        auto managementText = String("Your license for APPNAME is active on this machine.")
                                   .replace("APPNAME", appName);
        
        // TODO?: if (gui.getUnlockStatus().isTrial())
        //     managementText = managementText.replace("Your license", "Your trial license");
        
        if (status.isExpiring())
        {
            // TODO: String verb = gui.getUnlockStatus().isTrial() ? "Expires on " : "Renews on ";
            String verb = "Expires on ";
            managementText << juce::newLine << verb << status.getExpiryTime().toString (true, false);
        }
        
        instructionLabel->setText (managementText, dontSendNotification);
        licenseKey->setEnabled (false);
        licenseKey->setVisible (true);
        deactivateOthers->setVisible (false);
        progressBar.setVisible (false);
        addAndMakeVisible (syncButton.get());
        activateButton->setButtonText ("Deactivate");
        quitButton->setButtonText ("Clear");
        copyMachineButton.setVisible (false);
    }
    else
    {
        licenseKey->setEnabled (true);
        deactivateOthers->setVisible (true);
        instructionLabel->setText (activateInstructions, dontSendNotification);
        if (isForTrial)
        {
            progressBar.setVisible (true);
        }
        removeChildComponent (syncButton.get());
        activateButton->setButtonText ("Activate");
        quitButton->setButtonText ("Quit");
        if (licenseKey->isShowing())
            licenseKey->grabKeyboardFocus();
        copyMachineButton.setVisible (true);
    }

    resized();
}

void ActivationComponent::timerCallback()
{
    if (isForTrial || isForRegistration || grabbedFirstFocus || isForManagement)
        return stopTimer();

    if (auto* toplevel = getTopLevelComponent())
    {
        if (! toplevel->isOnDesktop())
            return;
        grabbedFirstFocus = true;
        licenseKey->grabKeyboardFocus();
        stopTimer();
    }
}

void ActivationComponent::handleRefreshResult (const UnlockStatus::UnlockResult result, UnlockOverlay::Action)
{
    if (result.succeeded)
    {
        auto& _status = status;
        auto* const dialog = findParentComponentOfClass<ActivationDialog>();

       #if 0
        if (EL_IS_TRIAL_EXPIRED(_status) || EL_IS_TRIAL_NOT_EXPIRED(_status))
        {
            setForManagement (false);
            isForTrial = false;
            setForTrial (true);
        }
        else 
       #endif
        
        if (dialog && closeDialogWhenFinished)
        {            
            dialog->closeButtonPressed();
        }
        else
        {
            isForTrial = false;
            setForManagement (KV_IS_ACTIVATED (_status));
        }

        _status.sendChangeMessage();
    }
}

bool ActivationComponent::isInterestedInFileDrag (const StringArray& files)
{
    for (const auto& name : files)
    {
        const File file (name);
        if (file.hasFileExtension ("elc"))
            return true;
    }

    return false;
}

void ActivationComponent::filesDropped (const StringArray& files, int x, int y)
{
    ignoreUnused (x, y);
    
    for (const auto& name : files)
    {
        const File file (name);
        if (file.hasFileExtension ("elc"))
        {
            FileInputStream src (file);
            if (status.applyKeyFile (src.readString()))
            {
                status.save();
                status.loadAll();
               #if 0
                if (EL_IS_TRIAL_EXPIRED(status) || EL_IS_TRIAL_NOT_EXPIRED(status))
                {
                    setForManagement (false);
                    isForTrial = false;
                    setForTrial (true);
                }
                else
                {
                    isForTrial = false;
                    setForManagement (EL_IS_ACTIVATED (status));
                }
               #endif
                status.sendChangeMessage();
            }
        }
    }
}

} // namespace kv
