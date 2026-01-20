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

    SharedResourcePointer<SharedAPIKeys> sharedTokens;
    sharedTokens->initializeAPIKeys();

    setSize(800, 2000);

    //setOpaque(true);
    //setWantsKeyboardFocus(true); // Remove focus from modelPathTextBox after clicking off

    //resized();

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
    //mainPanel.alignContent = FlexBox::AlignContent::flexStart;
    //mainPanel.alignItems = FlexBox::AlignItems::stretch;
    //mainPanel.justifyContent = FlexBox::JustifyContent::flexStart;

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
