#include "MainComponent.h"

JUCE_IMPLEMENT_SINGLETON(Logger)

MainComponent::MainComponent()
{
    Logger::getInstance()->initializeLogger();

    initializeMenuBar();

    // TODO - initialize model tab
    // TODO - initialize status area widget

    addAndMakeVisible(mediaClipboardWidget);

    showMediaClipboard = AppSettings::getBoolValue("showMediaClipboard", false);

    //fontaudioHelper = std::make_shared<fontaudio::IconHelper>();
    //fontawesomeHelper = std::make_shared<fontawesome::IconHelper>();

    setSize(800, 2000);

    setOpaque(true);
    setWantsKeyboardFocus(true); // Remove focus from modelPathTextBox when clicking off of it

    resized();
}

MainComponent::~MainComponent()
{
    // remove listeners
    loadBroadcaster.removeChangeListener(this);
    processBroadcaster.removeChangeListener(this);

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
    auto mainArea = getLocalBounds();

#if not JUCE_MAC
    menuBar->setBounds(
        mainArea.removeFromTop(LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight()));
#endif
    auto margin = 2; // Adjusted margin value for top and bottom spacing

    juce::FlexBox fullWindow;
    fullWindow.flexDirection = juce::FlexBox::Direction::row;

    juce::FlexBox mainPanel;
    mainPanel.flexDirection = juce::FlexBox::Direction::column;
    mainPanel.alignContent = juce::FlexBox::AlignContent::flexStart;
    mainPanel.alignItems = juce::FlexBox::AlignItems::stretch;
    mainPanel.justifyContent = juce::FlexBox::JustifyContent::flexStart;

    fullWindow.items.add(juce::FlexItem(mainPanel).withFlex(1));

    // Right Column: Media Clipboard Area
    if (showMediaClipboard)
    {
        fullWindow.items.add(juce::FlexItem(mediaClipboardWidget).withFlex(0.4));
    }
    else
    {
        mediaClipboardWidget.setBounds(0, 0, 0, 0);
    }

    // Apply the FlexBox layout to the full area
    fullWindow.performLayout(mainArea);
}
