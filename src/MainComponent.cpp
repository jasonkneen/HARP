#include "MainComponent.h"

JUCE_IMPLEMENT_SINGLETON(HARPLogger)

MainComponent::MainComponent()
{
    HARPLogger::getInstance()->initializeLogger();

    initializeMenuBar();

    addAndMakeVisible(mainModelTab);
    addAndMakeVisible(statusAreaWidget);
    addAndMakeVisible(mediaClipboardWidget);

    showStatusArea = Settings::getBoolValue("view.showStatusArea", true);
    showMediaClipboard = Settings::getBoolValue("view.showMediaClipboard", false);

    sharedTokens->initializeAPIKeys();

    setSize(800, 2000);

    //setOpaque(true);
    //setWantsKeyboardFocus(true); // Remove focus from modelPathTextBox after clicking off

    statusMessage->setMessage("Welcome to HARP!");
}

MainComponent::~MainComponent()
{
    // jobProcessorThread.signalThreadShouldExit();
    // This will not actually run any processing task
    // It'll just make sure that the thread is not waiting
    // and it'll allow it to check for the threadShouldExit flag
    // jobProcessorThread.signalTask();
    // jobProcessorThread.waitForThreadToExit(-1);

    deinitializeMenuBar();
    // commandManager.setFirstCommandTarget (nullptr);
}

void MainComponent::paint(Graphics& g)
{
    g.fillAll(getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
}

void MainComponent::resized()
{
    Rectangle<int> fullArea = getLocalBounds();

#if not JUCE_MAC
    menuBar->setBounds(
        fullArea.removeFromTop(LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight()));
#endif

    FlexBox fullWindow;
    fullWindow.flexDirection = FlexBox::Direction::row;

    FlexBox mainPanel;
    mainPanel.flexDirection = FlexBox::Direction::column;

    mainPanel.items.add(FlexItem(mainModelTab).withFlex(1.0));

    if (showStatusArea)
    {
        mainPanel.items.add(FlexItem(statusAreaWidget).withHeight(100));
    }
    else
    {
        statusAreaWidget.setBounds(0, 0, 0, 0);
    }

    fullWindow.items.add(FlexItem(mainPanel).withFlex(1.0));

    if (showMediaClipboard)
    {
        fullWindow.items.add(FlexItem(mediaClipboardWidget).withFlex(0.4));
    }
    else
    {
        mediaClipboardWidget.setBounds(0, 0, 0, 0);
    }

    fullWindow.performLayout(fullArea);
}

/* --File-- */

/**
 * Entry point for importing new files into HARP.
 */
void MainComponent::importNewFile(File mediaFile, bool fromDAW)
{
    mediaClipboardWidget.addTrackFromFilePath(URL(mediaFile), fromDAW);

    if (! showMediaClipboard)
    {
        viewMediaClipboardCallback();
    }
}

void MainComponent::openSettingsWindow()
{
    DialogWindow::LaunchOptions options;
    options.dialogTitle = "Settings";
    options.dialogBackgroundColour = Colours::darkgrey;
    //options.content.setOwned(new SettingsWindow(model.get()));
    options.content.setOwned(new SettingsWindow());

    options.useNativeTitleBar = true;
    options.resizable = true;
    options.escapeKeyTriggersCloseButton = true;

    options.launchAsync();
}

/* --Edit-- */

// TODO - The following are old callbacks from V2. They may be helpful in the future.

/*
void MainComponent::undoCallback()
{
    // DBG_AND_LOG("Undoing last edit");

    // // check if the audio file is loaded
    // if (! mediaDisplay->isFileLoaded())
    // {
    //     // TODO - gray out undo option in this case?
    //     // Fail with beep, we should just ignore this if it doesn't make sense
    //     DBG_AND_LOG("No file loaded to perform operation on");
    //     juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
    //     return;
    // }

    // if (isProcessing)
    // {
    //     DBG_AND_LOG("Can't undo while processing occurring!");
    //     juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
    //     return;
    // }

    // Iterate over all inputMediaDisplays and call the iteratePreviousTempFile()
    //auto& inputMediaDisplays = inputTrackAreaWidget.getMediaDisplays();

    // for (auto& inputMediaDisplay : inputMediaDisplays)
    //     {
    //         if (! inputMediaDisplay->iteratePreviousTempFile())
    //         {
    //             DBG_AND_LOG("Nothing to undo!");
    //             // juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
    //         }
    //         else
    //         {
    //             saveEnabled = true;
    //             DBG_AND_LOG("Undo callback completed successfully");
    //         }
    //     }
}
*/

/*
void MainComponent::redoCallback()
{
    // DBG_AND_LOG("Redoing last edit");

    // // check if the audio file is loaded
    // if (! mediaDisplay->isFileLoaded())
    // {
    //     // TODO - gray out undo option in this case?
    //     // Fail with beep, we should just ignore this if it doesn't make sense
    //     DBG_AND_LOG("No file loaded to perform operation on");
    //     juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
    //     return;
    // }

    // if (isProcessing)
    // {
    //     DBG_AND_LOG("Can't redo while processing occurring!");
    //     juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
    //     return;
    // }

    // Iterate over all inputMediaDisplays and call the iterateNextTempFile()
    //auto& inputMediaDisplays = inputTrackAreaWidget.getMediaDisplays();

    // for (auto& inputMediaDisplay : inputMediaDisplays)
    //     {
    //         if (! inputMediaDisplay->iterateNextTempFile())
    //         {
    //             DBG_AND_LOG("Nothing to redo!");
    //             // juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
    //         }
    //         else
    //         {
    //             saveEnabled = true;
    //             DBG_AND_LOG("Redo callback completed successfully");
    //         }
    //     }
}
*/

/* --View-- */

void MainComponent::viewStatusAreaCallback()
{
    // Toggle status Area visibility state
    showStatusArea = ! showStatusArea;

    // Find top-level window for resizing
    if (auto* window = findParentComponentOfClass<DocumentWindow>())
    {
        // Determine which display contains HARP
        auto* currentDisplay =
            Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds());

        int currentDisplayHeight;

        if (currentDisplay != nullptr)
        {
            if (window->isFullScreen())
            {
                currentDisplayHeight = currentDisplay->totalArea.getHeight();
            }
            else
            {
                currentDisplayHeight = currentDisplay->userArea.getHeight();
            }
        }

        // Get current bounds of top-level window
        Rectangle<int> windowBounds = window->getBounds();

        if (showStatusArea)
        {
            // Scale bounds to extend window by height of status area
            windowBounds.setHeight(jmin(currentDisplayHeight, windowBounds.getHeight() + 100));
        }
        else
        {
            if (! window->isFullScreen())
            {
                // Scale bounds to reduce window to main height
                windowBounds.setHeight(windowBounds.getHeight() - 100);
            }
        }

        // Set extended or reduced bounds
        window->setBounds(windowBounds);
    }

    // Add view preference to persistent settings
    Settings::setValue("view.showStatusArea", showStatusArea ? "1" : "0", true);

    // Send status message to add check to file menu
    commandManager.commandStatusChanged();

    resized();
}

void MainComponent::viewMediaClipboardCallback()
{
    // Toggle media clipboard visibility state
    showMediaClipboard = ! showMediaClipboard;

    // Find top-level window for resizing
    if (auto* window = findParentComponentOfClass<DocumentWindow>())
    {
        // Determine which display contains HARP
        auto* currentDisplay =
            Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds());

        int currentDisplayWidth;

        if (currentDisplay != nullptr)
        {
            if (window->isFullScreen())
            {
                currentDisplayWidth = currentDisplay->totalArea.getWidth();
            }
            else
            {
                currentDisplayWidth = currentDisplay->userArea.getWidth();
            }
        }

        //int totalDesktopWidth = Desktop::getInstance().getDisplays().getDisplayForRect(getBounds())->totalArea.getWidth();

        // Get current bounds of top-level window
        Rectangle<int> windowBounds = window->getBounds();

        if (showMediaClipboard)
        {
            // Scale bounds to extend window by 40% of main width
            windowBounds.setWidth(
                jmin(currentDisplayWidth, static_cast<int>(1.4 * windowBounds.getWidth())));
        }
        else
        {
            if (! window->isFullScreen())
            {
                // Scale bounds to reduce window to main width
                windowBounds.setWidth(static_cast<int>(windowBounds.getWidth() / 1.4));
            }
        }

        // Set extended or reduced bounds
        window->setBounds(windowBounds);
    }

    // Add view preference to persistent settings
    Settings::setValue("view.showMediaClipboard", showMediaClipboard ? "1" : "0", true);

    // Send status message to add check to file menu
    commandManager.commandStatusChanged();

    resized();
}

/* --Help-- */

void MainComponent::openAboutWindow()
{
    auto aboutComponent = std::make_unique<AboutWindow>();

    DialogWindow::LaunchOptions options;
    options.dialogTitle = "About " + String(APP_NAME);
    options.dialogBackgroundColour = Colours::grey;
    options.content.setOwned(aboutComponent.release());

    options.useNativeTitleBar = true;
    options.resizable = false;
    options.escapeKeyTriggersCloseButton = true;

    options.launchAsync();
}

// void MainComponent::openWelcomeWindow() {}

/* --Miscellaneous-- */

// TODO - The following is an old callback from V2. It may be helpful in the future.

/*
void MainComponent::focusCallback()
{
    if (mediaDisplay->isFileLoaded())
    {
        Time lastModTime =
            mediaDisplay->getTargetFilePath().getLocalFile().getLastModificationTime();
        if (lastModTime > lastLoadTime)
        {
            // Create an AlertWindow
            auto* reloadCheckWindow = new AlertWindow(
                "File has been modified",
                "The loaded file has been modified in a different editor! Would you like HARP to load the new version of the file?\nWARNING: This will clear the undo log and cause all unsaved edits to be lost!",
                AlertWindow::QuestionIcon);

            reloadCheckWindow->addButton("Yes", 1, KeyPress(KeyPress::returnKey));
            reloadCheckWindow->addButton("No", 0, KeyPress(KeyPress::escapeKey));

            // Show the window and handle the result asynchronously
            reloadCheckWindow->enterModalState(
                true,
                new CustomPathAlertCallback(
                    [this, reloadCheckWindow](int result)
                    {
                        if (result == 1)
                        { // Yes was clicked
                            DBG_AND_LOG("Reloading file");
                            loadMediaDisplay(mediaDisplay->getTargetFilePath().getLocalFile());
                        }
                        else
                        { // No was clicked or the window was closed
                            DBG_AND_LOG("Not reloading file");
                            lastLoadTime =
                                Time::getCurrentTime(); //Reset time so we stop asking
                        }
                        delete reloadCheckWindow;
                    }),
                true);
        }
    }
}
*/
