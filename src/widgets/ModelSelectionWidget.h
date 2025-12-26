/*
 * @file ModelSelectionWidget.h
 * @brief TODO
 * @author cwitkowitz
 */

#pragma once

#include <any>
#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../widgets/StatusAreaWidget.h"

#include "../gui/HoverHandler.h"
#include "../gui/MultiButton.h"

#include "../utils/Interface.h"
#include "../utils/Logging.h"

using namespace juce;

struct SharedChoices //: public ChangeBroadcaster
{
    /* TODO - functions for updating savedModelPaths
    void setMessage(const String& m)
    {
        message = m;
        sendChangeMessage();
    }

    void clearMessage()
    {
        message.clear();
        sendChangeMessage();
    }
    */

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

/* TODO
// this is the callback for the add new path popup alert
class CustomPathAlertCallback : public ModalComponentManager::Callback
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

class CustomPathComponent : public Component
{
public:
    CustomPathComponent(std::function<void(String)> onLoad, std::function<void()> onCancel)
        : onLoadCallback(std::move(onLoad)), onCancelCallback(std::move(onCancel))
    {
        pathEditor.setMultiLine(false);
        pathEditor.setReturnKeyStartsNewLine(false);
        pathEditor.onTextChange = [this] { loadButton.setEnabled(! pathEditor.isEmpty()); };
        pathEditor.onReturnKey = [this]
        {
            if (loadButton.isEnabled())
            {
                loadButton.triggerClick();
            }
        };
        addAndMakeVisible(pathEditor);

        loadButton.setEnabled(false);
        loadButton.onClick = [this]
        {
            wasLoadPressed = true;

            if (onLoadCallback)
            {
                onLoadCallback(pathEditor.getText());
            }

            closePopup();
        };
        addAndMakeVisible(loadButton);

        cancelButton.onClick = [this] { closePopup(); };
        addAndMakeVisible(cancelButton);

        setSize(400, 80);
    }

    ~CustomPathComponent() override
    {
        if (! wasLoadPressed && onCancelCallback)
        {
            // Treat as cancel if closed without load
            onCancelCallback();
        }
    }

    void visibilityChanged() override
    {
        if (isVisible())
        {
            MessageManager::callAsync([this] { pathEditor.grabKeyboardFocus(); });
        }
    }

    void resized() override
    {
        Rectangle<int> fullArea = getLocalBounds();

        FlexBox fullPopup;
        fullPopup.flexDirection = FlexBox::Direction::column;

        fullPopup.items.add(FlexItem(pathEditor).withHeight(30).withMargin(2));

        FlexBox buttonsArea;
        buttonsArea.flexDirection = FlexBox::Direction::row;

        buttonsArea.items.add(FlexItem(loadButton).withFlex(1).withMargin(10));
        buttonsArea.items.add(FlexItem().withFlex(0.25));
        buttonsArea.items.add(FlexItem(cancelButton).withFlex(1).withMargin(10));

        fullPopup.items.add(FlexItem(buttonsArea).withFlex(1));

        fullPopup.performLayout(fullArea);
    }

    void paint(Graphics& g) override
    {
        g.fillAll(getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void setTextFieldValue(const String& text)
    {
        pathEditor.setText(text, dontSendNotification);
        pathEditor.selectAll();
    }

private:
    void closePopup()
    {
        if (auto* popup = findParentComponentOfClass<DialogWindow>())
        {
            popup->exitModalState(0);
        }
    }

    TextEditor pathEditor;
    TextButton loadButton { "Load" };
    TextButton cancelButton { "Cancel" };

    bool wasLoadPressed = false;

    std::function<void(String)> onLoadCallback;
    std::function<void()> onCancelCallback;
};

class ModelSelectionWidget : public Component, public ChangeBroadcaster
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

    String getCurrentlySelectedPath() { return selectedPath; }

    void resetState()
    {
        lastLoadedPathIndex = -1;
        lastSelectedPathIndex = -1;
        modelPathComboBox.setSelectedId(lastSelectedPathIndex);

        //loadModelButton.setMode(loadButtonInactiveInfo.label);
        loadModelButton.setEnabled(false);
    }

    void setLoadingState()
    {
        modelPathComboBox.setEnabled(false);

        loadModelButton.setEnabled(false);
    }

    /*void setFinishedState()
    {
        modelPathComboBox.setEnabled(true);

        loadModelButton.setEnabled(true);
    }

    // TODO - set to successfully loaded custom path

    void setSuccessfulState()
    {
        lastLoadedPathIndex = lastSelectedPathIndex;

        setFinishedState();
    }*/

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

                    openCustomPathPopup();
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
        std::function<void()> loadCallback = [this]()
        {
            if (modelPathComboBox.getSelectedItemIndex() != 0)
            {
                selectedPath = modelPathComboBox.getText();
                sendChangeMessage();
            }
        };

        // Mode when a model is selected and not currently being loaded (load enabled)
        loadButtonActiveInfo = MultiButton::Mode { "Load",
                                                   loadCallback,
                                                   Colours::lightgrey,
                                                   "Click to load currently selected model path.",
                                                   MultiButton::DrawingMode::TextOnly };
        // Mode when a model has not been selected or is currently being loaded (load disabled)
        /*loadButtonInactiveInfo = MultiButton::Mode { "Load",
                                                     [this] {},
                                                     Colours::darkgrey,
                                                     "No model is currently selected.",
                                                     MultiButton::DrawingMode::TextOnly };*/
        loadModelButton.addMode(loadButtonActiveInfo);
        //loadModelButton.addMode(loadButtonInactiveInfo);
        loadModelButton.setMode(loadButtonActiveInfo.label);
        addAndMakeVisible(loadModelButton);

        //loadBroadcaster.addChangeListener(this); // TODO - necessary?
    }

    /* TODO
    // Adds a path to the model dropdown if it's not already present
    void addCustomPathToDropdown(const std::string& path, bool wasSleeping = false)
    {
        String displayStr(path);
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

        modelPathComboBox.setText(displayStr, dontSendNotification);
    }
    */

    /**
     * Create callbacks for and launch the custom path popup.
     */
    void openCustomPathPopup(const String& prefillText = "")
    {
        std::function<void(String)> loadCallback = [this](String path)
        {
            DBG_AND_LOG("ModelSelectionWidget::openCustomPathPopup::loadCallback: "
                        << "Custom path \"" << path << "\" entered.");

            selectedPath = path;
            sendChangeMessage();
        };

        std::function<void()> cancelCallback = [this]()
        {
            DBG_AND_LOG("ModelSelectionWidget::openCustomPathPopup::cancelCallback: "
                        << "Custom path selection canceled.");

            if (lastLoadedPathIndex >= 0)
            {
                // Set combo box selection to last successfully loaded model
                modelPathComboBox.setSelectedId(lastLoadedPathIndex + 1);
            }
            else if (lastSelectedPathIndex >= 0)
            {
                // Set combo box selection to previous valid selection
                modelPathComboBox.setSelectedId(lastSelectedPathIndex + 1);
            }
            else
            {
                resetState();
            }
        };

        CustomPathComponent* content =
            new CustomPathComponent(std::move(loadCallback), std::move(cancelCallback));

        if (prefillText.isNotEmpty())
        {
            content->setTextFieldValue(prefillText);
        }

        DialogWindow::LaunchOptions options;
        options.dialogTitle = "Enter Custom Path";
        options.dialogBackgroundColour = Colours::darkgrey;
        options.content.setOwned(content);

        options.useNativeTitleBar = false;
        options.resizable = false;
        options.escapeKeyTriggersCloseButton = true;
        options.componentToCentreAround = getParentComponent();

        options.launchAsync();
    }

    const float marginSize = 2;

    SharedResourcePointer<SharedChoices> sharedChoices;

    ComboBox modelPathComboBox;
    HoverHandler modelPathComboBoxHandler { modelPathComboBox };

    int lastLoadedPathIndex; // Keep track of last loaded index for load failure cases
    int lastSelectedPathIndex;

    String selectedPath;

    MultiButton loadModelButton;
    MultiButton::Mode loadButtonActiveInfo;
    MultiButton::Mode loadButtonInactiveInfo;

    SharedResourcePointer<InstructionsMessage> instructionsMessage;
};
