/*
 * @file ModelSelectionWidget.h
 * @brief TODO
 * @author cwitkowitz
 */

#pragma once

#include "juce_gui_basics/juce_gui_basics.h"

#include "../utils.h"

using namespace juce;

// this is the callback for the add new path popup alert
class CustomPathAlertCallback : public juce::ModalComponentManager::Callback
{
   public:
    CustomPathAlertCallback(std::function<void(int)> const& callback) : userCallback(callback) {}

    void modalStateFinished(int result) override
    {
        if (userCallback != nullptr)
        {
            userCallback(result);
        }
    }

   private:
    std::function<void(int)> userCallback;
};

class ModelSelectionWidget : public Component
{
   public:
    ModelSelectionWidget()
    {
        loadButtonInfo = MultiButton::Mode{
            // "Load Model",
            "Load",
            [this]
            { loadModelCallback(); },
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

    ~ModelSelectionWidget()
    {
        // TODO
    }

    void paint(Graphics& g)
    {
        // TODO
    }

    void resized() override
    {
        juce::FlexBox row1;
        row1.flexDirection = juce::FlexBox::Direction::row;
        row1.items.add(juce::FlexItem(modelPathComboBox).withFlex(8).withMargin(margin));
        row1.items.add(juce::FlexItem(loadModelButton).withFlex(1).withMargin(margin));
        mainPanel.items.add(juce::FlexItem(row1).withHeight(30));
    }

    void MainComponent::initModelPathComboBox()
    {
        // TODO - make this a shared resource pointer?
        std::vector<std::string> savedModelPaths = {
            "custom path ...",
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
        };

        // TODO - the following patterns should be supported
        // "xribene/midi_pitch_shifter",
        // "http://localhost:7860",
        // "https://xribene-midi-pitch-shifter.hf.space/",
        // "https://huggingface.co/spaces/xribene/midi_pitch_shifter"

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
        modelPathComboBoxHandler.onMouseExit = [this]()
        { clearInstructions(); };
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
        // should I clear this?
        //  spaceUrlButton.setButtonText("");
        //  spaceUrlButtonHandler.detach();

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

    /*
    // Adds a path to the model dropdown if it's not already present
    void addCustomPathToDropdown(const std::string& path, bool wasSleeping = false)
    {
        juce::String displayStr(path);
        if (wasSleeping)
            displayStr += " (sleeping)";

        bool alreadyExists = false;
        for (int i = 0; i < modelPathComboBox.getNumItems(); ++i)
        {
            if (modelPathComboBox.getItemText(i).startsWithIgnoreCase(path))
            {
                alreadyExists = true;
                break;
            }
        }

        if (! alreadyExists)
        {
            int newID = modelPathComboBox.getNumItems() + 1;
            modelPathComboBox.addItem(displayStr, newID);
        }

        modelPathComboBox.setText(displayStr, juce::dontSendNotification);
    }
    */

   private:
    ComboBox modelPathComboBox;
    std::string customPath;
    // Two usefull variables to keep track of the selected item in the modelPathComboBox
    // and the item index of the last loaded model
    // These are used to restore the selected item in the modelPathComboBox
    // after a failed attempt to load a new model
    int lastSelectedItemIndex = -1;
    int lastLoadedModelItemIndex = -1;
    HoverHandler modelPathComboBoxHandler{modelPathComboBox};

    MultiButton loadModelButton;
    MultiButton::Mode loadButtonInfo;
};
