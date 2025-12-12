#include "MainComponent.h"

void MainComponent::initModelPathComboBox()
{
    // model path textbox
    std::vector<std::string> modelPaths = {
        "custom path...",
        // "hugggof/vampnet-music",  "lllindsey0615/pyharp_demucs",
        // "lllindsey0615/pyharp_AMT", "npruyne/timbre-trap",    "xribene/harmonic_percussive_v5",
        // "lllindsey0615/DEMUCS_GPU", "cwitkowitz/timbre-trap",
        // "npruyne/audio_similarity",
        // "xribene/pitch_shifter",
        // "xribene/midi_pitch_shifter",
        // "xribene/pitch_shifter_slow",
        "stability/text-to-audio",
        "stability/audio-to-audio",
        "teamup-tech/text2midi-symbolic-music-generation",
        "teamup-tech/demucs-source-separation",
        "teamup-tech/solo-piano-audio-to-midi-transcription",
        "teamup-tech/TRIA",
        "teamup-tech/anticipatory-music-transformer",
        "teamup-tech/vampnet-conditional-music-generation",
        "teamup-tech/harmonic-percussive-separation",
        "teamup-tech/Kokoro-TTS",
        "teamup-tech/MegaTTS3-Voice-Cloning",
        "teamup-tech/midi-synthesizer",
        // "xribene/HARP-UI-TEST-v3"
        // "http://localhost:7860",
        // "https://xribene-midi-pitch-shifter.hf.space/",
        // "https://huggingface.co/spaces/xribene/midi_pitch_shifter",
        // "xribene/midi_pitch_shifter",
        // "https://huggingface.co/spaces/xribene/pitch_shifter",
    };

    modelPathComboBox.setTextWhenNothingSelected("choose a model");
    for (auto i = 0u; i < modelPaths.size(); ++i)
    {
        modelPathComboBox.addItem(modelPaths[i], static_cast<int>(i) + 1);
    }
    modelPathComboBoxHandler.onMouseEnter = [this]()
    {
        setInstructions(
            "A drop-down menu with some available models. Any new model you add will automatically be added to the list");
    };
    modelPathComboBoxHandler.onMouseExit = [this]() { clearInstructions(); };
    modelPathComboBoxHandler.attach();

    // Usage within your existing onChange handler
    modelPathComboBox.onChange = [this]
    {
        // Check if the 'custom path...' option is selected
        if (modelPathComboBox.getSelectedItemIndex() == 0)
        {
            openCustomPathDialog("");
        }
        else
        {
            lastSelectedItemIndex = modelPathComboBox.getSelectedItemIndex();
        }
        loadModelButton.setEnabled(true);
    };

    addAndMakeVisible(modelPathComboBox);
}

void MainComponent::resetModelPathComboBox()
{
    // cb: why do we resetUI inside a function named resetModelPathComboBox ?
    resetUI();
    //should I clear this?
    // spaceUrlButton.setButtonText("");
    // spaceUrlButtonHandler.detach();

    int numItems = modelPathComboBox.getNumItems();
    std::vector<std::string> options;

    for (int i = 0; i < numItems; ++i) // item indexes are 1-based in JUCE
    {
        String itemText = modelPathComboBox.getItemText(i);
        options.push_back(itemText.toStdString());
        DBG("Item index" << i << ": " << itemText);
    }

    modelPathComboBox.clear();

    modelPathComboBox.setTextWhenNothingSelected("choose a model");
    for (auto i = 0u; i < options.size(); ++i)
    {
        modelPathComboBox.addItem(options[i], static_cast<int>(i) + 1);
    }
    lastSelectedItemIndex = -1;
}

void MainComponent::initLoadModelButton()
{
    loadButtonInfo = MultiButton::Mode {
        // "Load Model",
        "Load",
        [this] { loadModelCallback(); },
        Colours::lightgrey,
        "Click to load the selected model path",
        MultiButton::DrawingMode::TextOnly
        // MultiButton::DrawingMode::IconOnly,
        // fontawesome::Download,
    };
    loadModelButton.addMode(loadButtonInfo);
    loadModelButton.setMode(loadButtonInfo.label);
    loadModelButton.setEnabled(false);
    addAndMakeVisible(loadModelButton);
    loadBroadcaster.addChangeListener(this);
}

void MainComponent::initProcessCancelButton()
{
    // The Process/Cancel button
    processButtonInfo = MultiButton::Mode {
        "Process",
        [this] { processCallback(); },
        Colours::orangered,
        "Click to send the input for processing",
        MultiButton::DrawingMode::TextOnly,
        fontaudio::Pause,
    };

    cancelButtonInfo = MultiButton::Mode {
        "Cancel",
        [this] { cancelCallback(); },
        Colours::lightgrey,
        "Click to cancel the processing",
        MultiButton::DrawingMode::TextOnly,
        fontaudio::Pause,
    };

    processCancelButton.addMode(processButtonInfo);
    processCancelButton.addMode(cancelButtonInfo);
    processCancelButton.setMode(processButtonInfo.label);
    processCancelButton.setEnabled(false);
    addAndMakeVisible(processCancelButton);

    processBroadcaster.addChangeListener(this);
    //saveEnabled = false;

    ModelStatus currentStatus = model->getStatus();
    if (currentStatus == ModelStatus::LOADED || currentStatus == ModelStatus::FINISHED)
    {
        processCancelButton.setEnabled(true);
        processCancelButton.setMode(processButtonInfo.label);
    }
    else if (currentStatus == ModelStatus::PROCESSING || currentStatus == ModelStatus::STARTING
             || currentStatus == ModelStatus::SENDING)
    {
        processCancelButton.setEnabled(true);
        processCancelButton.setMode(cancelButtonInfo.label);
    }
    setStatus(currentStatus);
}

void MainComponent::resetUI()
{
    controlAreaWidget.resetUI();
    inputTrackAreaWidget.resetUI();
    outputTrackAreaWidget.resetUI();
    // Also clear the model card components
    ModelCard empty;
    setModelCard(empty);
    // modelAuthorLabelHandler.detach();
}

void MainComponent::resetProcessingButtons()
{
    processCancelButton.setMode(processButtonInfo.label);
    processCancelButton.setEnabled(true);
    //saveEnabled = true;
    //isProcessing = false;
    loadModelButton.setEnabled(true);
    modelPathComboBox.setEnabled(true);
    repaint();
}
