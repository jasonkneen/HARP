#include "MainComponent.h"

// explicit MainComponent(const URL& initialFilePath = URL()) : jobsFinished(0), totalJobs(0)
MainComponent::MainComponent() //: jobsFinished(0), totalJobs(0)
//   jobProcessorThread(customJobs, jobsFinished, totalJobs, processBroadcaster)
{
    HarpLogger::getInstance()->initializeLogger();
    fontaudioHelper = std::make_shared<fontaudio::IconHelper>();
    fontawesomeHelper = std::make_shared<fontawesome::IconHelper>();

    // initSomeButtons();
    // initPlayStopButton();

    // initialize HARP UI
    // TODO: what happens if the model is nullptr rn?
    if (model == nullptr)
    {
        DBG("FATAL HARPProcessorEditor::HARPProcessorEditor: model is null");
        jassertfalse;
        return;
    }

    // Set setWantsKeyboardFocus to true for this component
    // Doing that, everytime we click outside the modelPathTextBox,
    // the focus will be taken away from the modelPathTextBox
    setWantsKeyboardFocus(true);

    initializeMenuBar();

    showMediaClipboard = AppSettings::getBoolValue("showMediaClipboard", false);

    initProcessCancelButton();

    initLoadModelButton();

    // add a status timer to update the status label periodically
    mModelStatusTimer = std::make_unique<ModelStatusTimer>(model);
    mModelStatusTimer->addChangeListener(this);
    mModelStatusTimer->startTimer(50); // 100 ms interval

    initModelPathComboBox();

    addAndMakeVisible(mediaClipboardWidget);

    // model controls
    controlAreaWidget.setModel(model);
    addAndMakeVisible(controlAreaWidget);
    controlAreaWidget.populateControls();

    inputTracksLabel.setJustificationType(juce::Justification::centred);
    inputTracksLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    addAndMakeVisible(inputTracksLabel);

    outputTracksLabel.setJustificationType(juce::Justification::centred);
    outputTracksLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    addAndMakeVisible(outputTracksLabel);

    populateTracks();
    addAndMakeVisible(inputTrackAreaWidget);
    addAndMakeVisible(outputTrackAreaWidget);

    addAndMakeVisible(descriptionLabel);
    // addAndMakeVisible(tagsLabel);
    // addAndMakeVisible(audioOrMidiLabel);

    addAndMakeVisible(statusBox);
    addAndMakeVisible(instructionBox);

    // model card component
    // Get the modelCard from the EditorView
    auto& card = model->card();
    setModelCard(card);

    // jobProcessorThread.startThread();
    //tryLoadSavedToken();

    setOpaque(true);
    setSize(800, 2000);
    // set to full screen
    // setFullScreen(true);
    resized();
}

MainComponent::~MainComponent()
{
    // remove listeners
    mModelStatusTimer->removeChangeListener(this);
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

    // Row 1: Model Path ComboBox and Load Model Button
    juce::FlexBox row1;
    row1.flexDirection = juce::FlexBox::Direction::row;
    row1.items.add(juce::FlexItem(modelPathComboBox).withFlex(8).withMargin(margin));
    row1.items.add(juce::FlexItem(loadModelButton).withFlex(1).withMargin(margin));
    mainPanel.items.add(juce::FlexItem(row1).withHeight(30));

    // Row 2: ModelName / AuthorName Labels
    juce::FlexBox row2;
    row2.flexDirection = juce::FlexBox::Direction::row;
    row2.items.add(juce::FlexItem(modelAuthorLabel).withFlex(0.5).withMargin(margin));
    row2.items.add(juce::FlexItem().withFlex(0.5).withMargin(margin));
    mainPanel.items.add(juce::FlexItem(row2).withHeight(30).withMargin(margin));

    // Row 3: Description
    auto font = Font(15.0f);
    descriptionLabel.setFont(font);
    // descriptionLabel.setColour(Label::backgroundColourId, Colours::red);
    auto maxLabelWidth = mainArea.getWidth() - 2 * margin;
    auto numberOfLines = font.getStringWidthFloat(descriptionLabel.getText(false)) / maxLabelWidth;
    float textHeight = (font.getHeight() + 5) * (std::floor(numberOfLines) + 1) + font.getHeight();
    mainPanel.items.add(juce::FlexItem(descriptionLabel).withHeight(textHeight).withMargin(margin));

    // Row 4: Control Area Widget
    // TODO - set min/max height based on limits of control element scaling
    mainPanel.items.add(juce::FlexItem(controlAreaWidget).withFlex(1).withMargin(margin));

    // Row 5: Process Cancel Button
    juce::FlexBox rowProcessCancelButton;
    rowProcessCancelButton.flexDirection = juce::FlexBox::Direction::row;
    rowProcessCancelButton.justifyContent = juce::FlexBox::JustifyContent::center;
    rowProcessCancelButton.items.add(juce::FlexItem().withFlex(1));
    rowProcessCancelButton.items.add(
        juce::FlexItem(processCancelButton).withWidth(150).withMargin(margin));
    rowProcessCancelButton.items.add(juce::FlexItem().withFlex(1));
    mainPanel.items.add(juce::FlexItem(rowProcessCancelButton).withHeight(30).withMargin(margin));

    // Row 6: Input Tracks Area Widget
    float numInputTracks = inputTrackAreaWidget.getNumTracks();
    float numOutputTracks = outputTrackAreaWidget.getNumTracks();
    float totalTracks = numInputTracks + numOutputTracks;

    if (numInputTracks > 0)
    {
        float inputTrackAreaFlex = 4 * (numInputTracks / totalTracks);
        mainPanel.items.add(juce::FlexItem(inputTracksLabel).withHeight(20).withMargin(margin));
        mainPanel.items.add(
            juce::FlexItem(inputTrackAreaWidget).withFlex(inputTrackAreaFlex).withMargin(margin));
    }
    else
    {
        inputTracksLabel.setBounds(0, 0, 0, 0);
        inputTrackAreaWidget.setBounds(0, 0, 0, 0);
    }

    // Row 7: Output Tracks Area Widget
    if (numOutputTracks > 0)
    {
        float outputTrackAreaFlex = 4 * (numOutputTracks / totalTracks);
        mainPanel.items.add(juce::FlexItem(outputTracksLabel).withHeight(20).withMargin(margin));
        mainPanel.items.add(
            juce::FlexItem(outputTrackAreaWidget).withFlex(outputTrackAreaFlex).withMargin(margin));
    }
    else
    {
        outputTracksLabel.setBounds(0, 0, 0, 0);
        outputTrackAreaWidget.setBounds(0, 0, 0, 0);
    }

    // Row 8: Instructions Area and Status Area
    juce::FlexBox row8;
    row8.flexDirection = juce::FlexBox::Direction::row;
    row8.items.add(juce::FlexItem(*instructionBox).withFlex(1).withMargin(margin));
    row8.items.add(juce::FlexItem(*statusBox).withFlex(1).withMargin(margin));
    // TODO - fix maximum height?
    mainPanel.items.add(juce::FlexItem(row8).withFlex(0.4f));

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
