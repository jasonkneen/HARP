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

    statusMessage->setMessage("Welcome to HARP!");
}

MainComponent::~MainComponent() { deinitializeMenuBar(); }

void MainComponent::paint(Graphics& g)
{
    g.fillAll(getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
}

void MainComponent::paintOverChildren(Graphics& g)
{
    if (isTutorialActive)
    {
        auto area = getLocalBounds();
        g.setColour(Colours::black.withAlpha(0.6f));

        if (tutorialHighlightRect.isEmpty() && tutorialExtraHighlights.empty())
        {
            // Full dim if no highlight
            g.fillAll();
        }
        else
        {
            // Dim with cutout
            Path backgroundPath;
            backgroundPath.addRectangle(area.toFloat());

            Path highlightPath;
            if (! tutorialHighlightRect.isEmpty())
                highlightPath.addRoundedRectangle(tutorialHighlightRect.toFloat(), 5.0f);

            // Add extra highlights to the cutout path
            for (auto& rect : tutorialExtraHighlights)
            {
                highlightPath.addRoundedRectangle(rect.toFloat(), 5.0f);
            }

            backgroundPath.setUsingNonZeroWinding(false);
            backgroundPath.addPath(highlightPath);

            g.fillPath(backgroundPath);

            g.setColour(Colours::white);
            g.drawRoundedRectangle(tutorialHighlightRect.toFloat(), 5.0f, 2.0f);

            for (auto& rect : tutorialExtraHighlights)
            {
                g.drawRoundedRectangle(rect.toFloat(), 5.0f, 2.0f);
            }
        }
    }
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
    options.content.setOwned(new SettingsWindow());

    options.useNativeTitleBar = true;
    options.resizable = true;
    options.escapeKeyTriggersCloseButton = true;

    options.launchAsync();
}

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

void MainComponent::setTutorialActive(bool active)
{
    isTutorialActive = active;
    repaint();
}

void MainComponent::setTutorialHighlight(Rectangle<int> bounds)
{
    tutorialHighlightRect = bounds;
    repaint();
}

void MainComponent::setTutorialExtraHighlights(std::vector<Rectangle<int>> bounds)
{
    tutorialExtraHighlights = bounds;
    repaint();
}

void MainComponent::openWelcomeWindow()
{
    MessageManager::callAsync(
        [this]()
        {
            welcomeWindow.reset(new WelcomeWindow(mainComp));

            // Handle window closing
            welcomeWindow->onClose = [this]() { welcomeWindow.reset(); };

            // Position relative to main window or center
            welcomeWindow->centreWithSize(500, 420);
            welcomeWindow->setVisible(true);
            welcomeWindow->toFront(true);
        });
}

// Bounds accessors for tutorial steps
Rectangle<int> MainComponent::getModelSelectBounds()
{
    // Combine Combobox and Load Button (Row 1)
    if (modelPathComboBox.isVisible())
    {
        auto bounds = modelPathComboBox.getBounds();
        bounds = bounds.getUnion(loadModelButton.getBounds());
        return bounds.expanded(2, 2);
    }
    return {};
}

Rectangle<int> MainComponent::getLoadButtonBounds()
{
    if (loadModelButton.isVisible())
        return loadModelButton.getBounds().expanded(5, 5);
    return {};
}

Rectangle<int> MainComponent::getControlsBounds()
{
    if (controlAreaWidget.isVisible())
        return controlAreaWidget.getBounds().expanded(5, 5);
    if (controlAreaWidget.isVisible())
        return controlAreaWidget.getBounds().expanded(5, 5);
    return {};
}

Rectangle<int> MainComponent::getInputFolderBounds()
{
    auto bounds = inputTrackAreaWidget.getFirstTrackFolderButtonBounds();
    return getLocalArea(&inputTrackAreaWidget, bounds);
}

Rectangle<int> MainComponent::getInputPlayBounds()
{
    auto bounds = inputTrackAreaWidget.getFirstTrackPlayButtonBounds();
    return getLocalArea(&inputTrackAreaWidget, bounds);
}

Rectangle<int> MainComponent::getInputTrackBounds() { return inputTrackAreaWidget.getBounds(); }

Rectangle<int> MainComponent::getProcessButtonBounds() { return processCancelButton.getBounds(); }

Rectangle<int> MainComponent::getTracksBounds()
{
    auto bounds = inputTrackAreaWidget.getBounds();
    if (outputTrackAreaWidget.isVisible())
        bounds = bounds.getUnion(outputTrackAreaWidget.getBounds());

    // Include labels if visible
    if (inputTracksLabel.isVisible())
        bounds = bounds.getUnion(inputTracksLabel.getBounds());
    if (outputTracksLabel.isVisible())
        bounds = bounds.getUnion(outputTracksLabel.getBounds());

    return bounds.expanded(5, 5);
}

Rectangle<int> MainComponent::getInfoBarBounds()
{
    auto bounds = instructionBox->getBounds();
    if (statusBox->isVisible())
        bounds = bounds.getUnion(statusBox->getBounds());
    return bounds.expanded(5, 5);
}

Rectangle<int> MainComponent::getClipboardBounds()
{
    if (showMediaClipboard && mediaClipboardWidget.isVisible())
        return mediaClipboardWidget.getBounds().expanded(5, 5);
    return {};
}

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
