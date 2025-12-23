/*
 * @file ModelSelectionWidget.h
 * @brief TODO
 * @author cwitkowitz
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//#include "../utils.h"

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
        int margin = 2;

        juce::FlexBox row1;
        row1.flexDirection = juce::FlexBox::Direction::row;
        row1.items.add(juce::FlexItem(modelPathComboBox).withFlex(8).withMargin(margin));
        row1.items.add(juce::FlexItem(loadModelButton).withFlex(1).withMargin(margin));
        mainPanel.items.add(juce::FlexItem(row1).withHeight(30));
    }

    void initModelPathComboBox()
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

    void resetModelPathComboBox()
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
            DBG_AND_LOG("Item index" << i << ": " << itemText);
        }

        modelPathComboBox.clear();

        modelPathComboBox.setTextWhenNothingSelected("choose a model");
        for (auto i = 0u; i < options.size(); ++i)
        {
            modelPathComboBox.addItem(options[i], static_cast<int>(i) + 1);
        }
        lastSelectedItemIndex = -1;
    }

    void loadModelCallback()
    {
        // Get the URL/path the user provided in the comboBox
        std::string pathURL;
        if (modelPathComboBox.getSelectedItemIndex() == 0)
            pathURL = customPath;
        else
            pathURL = modelPathComboBox.getText().toStdString();

        std::map<std::string, std::any> params = {
            { "url", pathURL },
        };
        // resetUI();

        // disable the load button until the model is loaded
        loadModelButton.setEnabled(false);
        modelPathComboBox.setEnabled(false);
        loadModelButton.setButtonText("loading...");

        // disable the process button until the model is loaded
        processCancelButton.setEnabled(false);

        // loading happens asynchronously.
        threadPool.addJob(
            [this, params]
            {
                try
                {
                    juce::String loadingError;

                    // set the last status to the current status
                    // If loading of the new model fails,
                    // we want to go back to the status we had before the failed attempt
                    model->setLastStatus(model->getStatus());

                    OpResult loadingResult = model->load(params);
                    if (loadingResult.failed())
                    {
                        throw loadingResult.getError();
                    }

                    // loading succeeded
                    // Do some UI stuff to add the new model to the comboBox
                    // if it's not already there
                    // and update the lastSelectedItemIndex and lastLoadedModelItemIndex
                    MessageManager::callAsync(
                        [this, loadingResult]
                        {
                            resetUI();
                            if (modelPathComboBox.getSelectedItemIndex() == 0)
                            {
                                bool alreadyInComboBox = false;

                                for (int i = 0; i < modelPathComboBox.getNumItems(); ++i)
                                {
                                    if (modelPathComboBox.getItemText(i)
                                        == (juce::String) customPath)
                                    {
                                        alreadyInComboBox = true;
                                        modelPathComboBox.setSelectedId(i + 1);
                                        lastSelectedItemIndex = i;
                                        lastLoadedModelItemIndex = i;
                                    }
                                }

                                if (! alreadyInComboBox)
                                {
                                    int new_id = modelPathComboBox.getNumItems() + 1;
                                    modelPathComboBox.addItem(customPath, new_id);
                                    modelPathComboBox.setSelectedId(new_id);
                                    lastSelectedItemIndex = new_id - 1;
                                    lastLoadedModelItemIndex = new_id - 1;
                                }
                            }
                            else
                            {
                                lastLoadedModelItemIndex = modelPathComboBox.getSelectedItemIndex();
                            }
                            processLoadingResult(loadingResult);
                        });
                }
                catch (Error& loadingError)
                {
                    Error::fillUserMessage(loadingError);
                    DBG_AND_LOG("Error in Model Loading:\n" + loadingError.devMessage);
                    auto msgOpts =
                        MessageBoxOptions()
                            .withTitle("Loading Error")
                            .withIconType(AlertWindow::WarningIcon)
                            .withTitle("Error")
                            .withMessage("An error occurred while loading the WebModel: \n"
                                         + loadingError.userMessage);
                    // if (! String(e.what()).contains("404")
                    //     && ! String(e.what()).contains("Invalid URL"))
                    if (loadingError.type != ErrorType::InvalidURL)
                    {
                        msgOpts = msgOpts.withButton("Open Space URL");
                    }

                    msgOpts = msgOpts.withButton("Open HARP Logs").withButton("Ok");
                    auto alertCallback = [this, msgOpts, loadingError](int result)
                    {
                        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        // NOTE (hugo): there's something weird about the button indices assigned by the msgOpts here
                        // DBG_AND_LOG("ALERT-CALLBACK: buttonClicked alertCallback listener activated: chosen: " << chosen);
                        // auto chosen = msgOpts.getButtonText(result);
                        // they're not the same as the order of the buttons in the alert
                        // this is the order that I actually observed them to be.
                        // UPDATE/TODO (xribene): This should be fixed in Juce v8
                        // see: https://forum.juce.com/t/wrong-callback-value-for-alertwindow-showokcancelbox/55671/2
                        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        std::map<int, std::string> observedButtonIndicesMap = {};
                        if (msgOpts.getNumButtons() == 3)
                        {
                            observedButtonIndicesMap.insert(
                                { 1, "Open Space URL" }); // should actually be 0 right?
                        }
                        observedButtonIndicesMap.insert(
                            { msgOpts.getNumButtons() - 1,
                              "Open HARP Logs" }); // should actually be 1
                        observedButtonIndicesMap.insert({ 0, "Ok" }); // should be 2

                        auto chosen = observedButtonIndicesMap[result];

                        if (chosen == "Open HARP Logs")
                        {
                            HarpLogger::getInstance()->getLogFile().revealToUser();
                        }
                        else if (chosen == "Open Space URL")
                        {
                            // get the spaceInfo
                            SpaceInfo spaceInfo = model->getTempClient().getSpaceInfo();
                            if (spaceInfo.status == SpaceInfo::Status::GRADIO)
                            {
                                URL spaceUrl = this->model->getTempClient().getSpaceInfo().gradio;
                                spaceUrl.launchInDefaultBrowser();
                            }
                            else if (spaceInfo.status == SpaceInfo::Status::HUGGINGFACE)
                            {
                                URL spaceUrl =
                                    this->model->getTempClient().getSpaceInfo().huggingface;
                                spaceUrl.launchInDefaultBrowser();
                            }
                            else if (spaceInfo.status == SpaceInfo::Status::LOCALHOST)
                            {
                                // either choose hugingface or gradio, they are the same
                                URL spaceUrl =
                                    this->model->getTempClient().getSpaceInfo().huggingface;
                                spaceUrl.launchInDefaultBrowser();
                            }
                            else if (spaceInfo.status == SpaceInfo::Status::STABILITY)
                            {
                                URL spaceUrl =
                                    this->model->getTempClient().getSpaceInfo().stability;
                                spaceUrl.launchInDefaultBrowser();
                            }
                            // URL spaceUrl =
                            //     this->model->getGradioClient().getSpaceInfo().huggingface;
                            // spaceUrl.launchInDefaultBrowser();
                        }

                        if (lastLoadedModelItemIndex == -1)
                        {
                            // If before the failed attempt to load a new model, we HAD NO model loaded
                            // TODO: these two functions we call here might be an overkill for this case
                            // we need to simplify
                            MessageManager::callAsync(
                                [this, loadingError]
                                {
                                    resetModelPathComboBox();
                                    model->setStatus(ModelStatus::INITIALIZED);
                                    processLoadingResult(OpResult::fail(loadingError));
                                });
                        }
                        else
                        {
                            // If before the failed attempt to load a new model, we HAD a model loaded
                            MessageManager::callAsync(
                                [this, loadingError]
                                {
                                    // We set the status to
                                    // the status of the model before the failed attempt
                                    model->setStatus(model->getLastStatus());
                                    processLoadingResult(OpResult::fail(loadingError));
                                });
                        }

                        // This if/elseif/else block is responsible for setting the selected item
                        // in the modelPathComboBox to the correct item (i.e the model/path/app that
                        // was selected before the failed attempt to load a new model)
                        // cb: sometimes setSelectedId it doesn't work and I dont know why.
                        // I've tried nesting it in MessageManage::callAsync, but still nothing.
                        if (lastLoadedModelItemIndex != -1)
                        {
                            modelPathComboBox.setSelectedId(lastLoadedModelItemIndex + 1);
                        }
                        else if (lastLoadedModelItemIndex == -1 && lastSelectedItemIndex != -1)
                        {
                            modelPathComboBox.setSelectedId(lastSelectedItemIndex + 1);
                        }
                        else
                        {
                            resetModelPathComboBox();
                            MessageManager::callAsync([this, loadingError]
                                                      { loadModelButton.setEnabled(false); });
                        }
                        /*
                        if (loadingError.userMessage.containsIgnoreCase("sleeping"))
                        {
                            MessageManager::callAsync(
                                [this]
                                {
                                    addCustomPathToDropdown(customPath, true); // mark as sleeping
                                });
                        }
                        //NEW: reopen custom path dialog if sleeping or 404
                        if (loadingError.type == ErrorType::InvalidURL
                            || loadingError.devMessage.contains("404")
                            || loadingError.userMessage.containsIgnoreCase("sleeping"))
                        {
                            MessageManager::callAsync([this] { openCustomPathDialog(customPath); });
                        }
                        */
                    };

                    AlertWindow::showAsync(msgOpts, alertCallback);
                    //saveEnabled = false;
                }
                catch (const std::exception& e)
                {
                    // Catch any other standard exceptions (like std::runtime_error)
                    DBG_AND_LOG("Caught std::exception: " << e.what());
                    AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                                                     "Error",
                                                     "An unexpected error occurred: "
                                                         + juce::String(e.what()));
                }
                catch (...) // Catch any other exceptions
                {
                    DBG_AND_LOG("Caught unknown exception");
                    AlertWindow::showMessageBoxAsync(
                        AlertWindow::WarningIcon, "Error", "An unexpected error occurred.");
                }
            });
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
    HoverHandler modelPathComboBoxHandler { modelPathComboBox };

    MultiButton loadModelButton;
    MultiButton::Mode loadButtonInfo;
};
