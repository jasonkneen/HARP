#include "MainComponent.h"

#include <cmath>

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

void MainComponent::resized()
{
    constexpr int statusAreaHeight = 100;
    constexpr float mediaClipboardFlex = 0.4f;
    constexpr float mediaClipboardScale = 1.4f;
    constexpr int baseMinWindowWidth = 700;
    constexpr int baseMinWindowHeight = 500;
    constexpr int minMainPanelPadding = 32;
    constexpr int minMainPanelBaselineWidth = 320;
    constexpr int windowFramePadding = 32;

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
        mainPanel.items.add(FlexItem(statusAreaWidget).withHeight(statusAreaHeight));
    }
    else
    {
        statusAreaWidget.setBounds(0, 0, 0, 0);
    }

    fullWindow.items.add(FlexItem(mainPanel).withFlex(1.0));

    if (showMediaClipboard)
    {
        fullWindow.items.add(FlexItem(mediaClipboardWidget).withFlex(mediaClipboardFlex));
    }
    else
    {
        mediaClipboardWidget.setBounds(0, 0, 0, 0);
    }

    fullWindow.performLayout(fullArea);

    if (auto* window = findParentComponentOfClass<DocumentWindow>())
    {
        const float mainPanelShare = showMediaClipboard ? (1.0f / mediaClipboardScale) : 1.0f;
        const int minMainPanelWidth = jmax(minMainPanelBaselineWidth,
                                           mainModelTab.getMinimumRequiredControlWidth()
                                               + minMainPanelPadding);
        const int minWindowWidth = jmax(baseMinWindowWidth,
                                        (int) std::ceil((float) minMainPanelWidth / mainPanelShare));

        const int currentMainPanelWidth =
            jmax(minMainPanelWidth, mainModelTab.getWidth());
        const int minMainPanelHeight =
            mainModelTab.getMinimumRequiredHeightForWidth(currentMainPanelWidth)
            + (showStatusArea ? statusAreaHeight : 0);
        const int minWindowHeight = jmax(baseMinWindowHeight, minMainPanelHeight + windowFramePadding);

        auto& displays = Desktop::getInstance().getDisplays();
        auto* currentDisplay = displays.getDisplayForRect(window->getScreenBounds());
        auto* primaryDisplay = displays.getPrimaryDisplay();
        auto userArea = currentDisplay != nullptr ? currentDisplay->userArea
                        : (primaryDisplay != nullptr ? primaryDisplay->userArea
                                                     : Rectangle<int>(0, 0, minWindowWidth, minWindowHeight));

        const int maxWindowWidth = userArea.getWidth();
        const int maxWindowHeight = userArea.getHeight();

        int clampedMinWidth = jmin(minWindowWidth, maxWindowWidth);
        int clampedMinHeight = jmin(minWindowHeight, maxWindowHeight);

        window->setResizeLimits(
            clampedMinWidth, clampedMinHeight, maxWindowWidth, maxWindowHeight);

        if (! window->isFullScreen())
        {
            auto bounds = window->getBounds();
            int boundedWidth = jlimit(clampedMinWidth, maxWindowWidth, bounds.getWidth());
            int boundedHeight = jlimit(clampedMinHeight, maxWindowHeight, bounds.getHeight());

            if (boundedWidth != bounds.getWidth() || boundedHeight != bounds.getHeight())
            {
                bounds.setSize(boundedWidth, boundedHeight);
                window->setBounds(bounds);
            }
        }
    }
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
    constexpr int statusAreaHeight = 100;

    showStatusArea = ! showStatusArea;

    if (auto* window = findParentComponentOfClass<DocumentWindow>())
    {
        auto* currentDisplay =
            Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds());

        Rectangle<int> windowBounds = window->getBounds();
        int currentDisplayHeight = windowBounds.getHeight();

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

        if (showStatusArea)
        {
            windowBounds.setHeight(jmin(currentDisplayHeight, windowBounds.getHeight() + statusAreaHeight));
        }
        else
        {
            if (! window->isFullScreen())
            {
                windowBounds.setHeight(windowBounds.getHeight() - statusAreaHeight);
            }
        }

        window->setBounds(windowBounds);
    }

    Settings::setValue("view.showStatusArea", showStatusArea ? "1" : "0", true);

    commandManager.commandStatusChanged();

    resized();
}

void MainComponent::viewMediaClipboardCallback()
{
    constexpr float mediaClipboardScale = 1.4f;

    showMediaClipboard = ! showMediaClipboard;

    if (auto* window = findParentComponentOfClass<DocumentWindow>())
    {
        auto* currentDisplay =
            Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds());

        Rectangle<int> windowBounds = window->getBounds();
        int currentDisplayWidth = windowBounds.getWidth();

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

        if (showMediaClipboard)
        {
            windowBounds.setWidth(
                jmin(currentDisplayWidth, static_cast<int>(mediaClipboardScale * windowBounds.getWidth())));
        }
        else
        {
            if (! window->isFullScreen())
            {
                windowBounds.setWidth(static_cast<int>(windowBounds.getWidth() / mediaClipboardScale));
            }
        }

        window->setBounds(windowBounds);
    }

    Settings::setValue("view.showMediaClipboard", showMediaClipboard ? "1" : "0", true);

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

// void MainComponent::openWelcomeWindow() { TODO }

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
