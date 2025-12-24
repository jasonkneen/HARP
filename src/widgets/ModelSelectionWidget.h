/*
 * @file ModelSelectionWidget.h
 * @brief TODO
 * @author cwitkowitz
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../widgets/StatusAreaWidget.h"

#include "../gui/HoverHandler.h"
#include "../gui/MultiButton.h"

#include "../utils/Logging.h"

using namespace juce;

struct SharedChoices //: public ChangeBroadcaster
{
    /*void setMessage(const String& m)
    {
        message = m;
        sendChangeMessage();
    }

    void clearMessage()
    {
        message.clear();
        sendChangeMessage();
    }*/

    std::vector<std::string> savedModelPaths = {
        "click here to enter a custom path...",
        "stability/text-to-audio",
        "stability/audio-to-audio",
        "teamup-tech/text2midi-symbolic-music-generation",
        "teamup-tech/demucs-source-separation",
        "teamup-tech/solo-piano-audio-to-midi-transcription",
        "teamup-tech/TRIA", // TODO - more intuitive name: (The Rhythm In Anything) conditional drum generation
        "teamup-tech/anticipatory-music-transformer",
        "teamup-tech/vampnet-conditional-music-generation",
        "teamup-tech/harmonic-percussive-separation",
        "teamup-tech/Kokoro-TTS",
        "teamup-tech/MegaTTS3-Voice-Cloning",
        "teamup-tech/midi-synthesizer",
        // "xribene/HARP-UI-TEST-v3"
    };
};

// this is the callback for the add new path popup alert
/*class CustomPathAlertCallback : public juce::ModalComponentManager::Callback
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
};*/

class ModelSelectionWidget : public Component
{
public:
    ModelSelectionWidget()
    {
        initializeLoadModelButton();
        initializeModelPathComboBox();

        resetState();
    }

    ~ModelSelectionWidget()
    {
        // TODO
    }

    //void paint(Graphics& g) {}

    void resized() override
    {
        FlexBox selectionArea;
        selectionArea.flexDirection = FlexBox::Direction::row;

        selectionArea.items.add(FlexItem(modelPathComboBox).withFlex(1).withMargin(marginSize));
        selectionArea.items.add(FlexItem(loadModelButton).withWidth(100).withMargin(marginSize));

        selectionArea.performLayout(getLocalBounds());
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
    void initializeModelPathComboBox()
    {
        modelPathComboBox.setTextWhenNothingSelected("click here to select a model...");

        for (unsigned int i = 0; i < sharedChoices->savedModelPaths.size(); ++i)
        {
            // Add saved path to combo box (skipping 0 for custom path)
            modelPathComboBox.addItem(sharedChoices->savedModelPaths[i], static_cast<int>(i) + 1);
        }

        modelPathComboBox.onChange = [this]
        {
            if (modelPathComboBox.getSelectedItemIndex() == -1)
            {
                DBG_AND_LOG("modelPathComboBox::onChange: Combo box reset.");
            }
            else
            {
                if (modelPathComboBox.getSelectedItemIndex() == 0)
                {
                    DBG_AND_LOG("modelPathComboBox::onChange: Custom path selected.");

                    //openCustomPathDialog("");  // TODO
                }
                else
                {
                    lastSelectedPathIndex = modelPathComboBox.getSelectedItemIndex();

                    DBG_AND_LOG("modelPathComboBox::onChange: Entry " << lastSelectedPathIndex
                                                                      << " selected.");
                }

                //loadModelButton.setMode(loadButtonActiveInfo.label);
                loadModelButton.setEnabled(true);
            }
        };

        addAndMakeVisible(modelPathComboBox);

        modelPathComboBoxHandler.onMouseEnter = [this]()
        {
            if (instructionsMessage != nullptr)
            {
                instructionsMessage->setMessage(
                    "A drop-down menu with featured available models. Any custom paths "
                    "successfully loaded will automatically be added to the list.");
            }
        };
        modelPathComboBoxHandler.onMouseExit = [this]()
        {
            if (instructionsMessage != nullptr)
            {
                instructionsMessage->clearMessage();
            }
        };
        modelPathComboBoxHandler.attach();
    }

    void initializeLoadModelButton()
    {
        // Mode when a model is selected and not currently being loaded (load enabled)
        loadButtonActiveInfo =
            MultiButton::Mode { "Load",
                                //[this] { loadModelCallback(); }, // TODO
                                [this] { resetState(); },
                                Colours::lightgrey,
                                "Click to load currently selected model path.",
                                MultiButton::DrawingMode::TextOnly };
        // Mode when a model has not been selected or is currently being loaded (load disabled)
        /*loadButtonInactiveInfo = MultiButton::Mode { "Load", // Cannot 
                                                     [this] {},
                                                     Colours::darkgrey,
                                                     "No model is currently selected.",
                                                     MultiButton::DrawingMode::TextOnly };*/
        loadModelButton.addMode(loadButtonActiveInfo);
        //loadModelButton.addMode(loadButtonInactiveInfo);
        loadModelButton.setMode(loadButtonActiveInfo.label);
        addAndMakeVisible(loadModelButton);

        //loadBroadcaster.addChangeListener(this); // TODO
    }

    void resetState()
    {
        lastSelectedPathIndex = -1;
        modelPathComboBox.setSelectedId(lastSelectedPathIndex);

        //loadModelButton.setMode(loadButtonInactiveInfo.label);
        loadModelButton.setEnabled(false);
    }

    const float marginSize = 2;

    SharedResourcePointer<SharedChoices> sharedChoices;

    ComboBox modelPathComboBox;
    HoverHandler modelPathComboBoxHandler { modelPathComboBox };

    int lastSelectedPathIndex;

    MultiButton loadModelButton;
    MultiButton::Mode loadButtonActiveInfo;
    MultiButton::Mode loadButtonInactiveInfo;

    SharedResourcePointer<InstructionsMessage> instructionsMessage;

    // --

    //std::string customPath;

    // Two usefull variables to keep track of the selected item in the modelPathComboBox
    // and the item index of the last loaded model
    // These are used to restore the selected item in the modelPathComboBox
    // after a failed attempt to load a new model
    //int lastLoadedModelItemIndex = -1;
};
