#include "../MainComponent.h"

void MainComponent::openCustomPathDialog(const std::string& prefillPath = "")
{
    // Create and show the custom path dialog with a callback
    std::function<void(const juce::String&)> loadCallback = [this](const juce::String& customPath2)
    {
        DBG_AND_LOG("Custom path entered: " + customPath2);
        this->customPath = customPath2.toStdString(); // Store the custom path
        loadModelButton.triggerClick(); // Trigger the load model button click
    };
    std::function<void()> cancelCallback = [this]()
    {
        // modelPathComboBox.setSelectedId(lastSelectedItemIndex);
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
            MessageManager::callAsync([this] { loadModelButton.setEnabled(false); });
        }
    };

    CustomPathDialog* dialog = new CustomPathDialog(loadCallback, cancelCallback);
    if (! prefillPath.empty())
        dialog->setTextFieldValue(prefillPath);
}

void MainComponent::loadModelCallback()
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
                                if (modelPathComboBox.getItemText(i) == (juce::String) customPath)
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
                auto msgOpts = MessageBoxOptions()
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
                        { msgOpts.getNumButtons() - 1, "Open HARP Logs" }); // should actually be 1
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
                            URL spaceUrl = this->model->getTempClient().getSpaceInfo().huggingface;
                            spaceUrl.launchInDefaultBrowser();
                        }
                        else if (spaceInfo.status == SpaceInfo::Status::LOCALHOST)
                        {
                            // either choose hugingface or gradio, they are the same
                            URL spaceUrl = this->model->getTempClient().getSpaceInfo().huggingface;
                            spaceUrl.launchInDefaultBrowser();
                        }
                        else if (spaceInfo.status == SpaceInfo::Status::STABILITY)
                        {
                            URL spaceUrl = this->model->getTempClient().getSpaceInfo().stability;
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

void MainComponent::processLoadingResult(OpResult result)
{
    // return;
    if (result.wasOk())
    {
        setModelCard(model->card());
        controlAreaWidget.setModel(model);
        mModelStatusTimer->setModel(model);
        controlAreaWidget.populateControls();

        populateTracks();

        //Apply saved Stability token
        SpaceInfo spaceInfo = model->getClient().getSpaceInfo();
        if (spaceInfo.status == SpaceInfo::Status::STABILITY && ! savedStabilityToken.isEmpty())
        {
            model->getClient().setToken(savedStabilityToken);
            setStatus("Applied saved Stability AI token to loaded model.");
        }

        // juce::String spaceUrlButtonText;
        if (spaceInfo.status == SpaceInfo::Status::LOCALHOST)

        {
            // spaceUrlButton.setButtonText("open localhost in browser");
            // nameLabelButton.setURL(URL(spaceInfo.gradio));
            modelAuthorLabel.setURL(URL(spaceInfo.gradio));
        }
        else if (spaceInfo.status == SpaceInfo::Status::HUGGINGFACE)
        {
            // spaceUrlButton.setButtonText("open " + spaceInfo.userName + "/"
            //                              + spaceInfo.modelName + " in browser");
            // nameLabelButton.setURL(URL(spaceInfo.huggingface));
            modelAuthorLabel.setURL(URL(spaceInfo.huggingface));
        }
        else if (spaceInfo.status == SpaceInfo::Status::GRADIO)
        {
            // spaceUrlButton.setButtonText("open " + spaceInfo.userName + "-"
            //                              + spaceInfo.modelName + " in browser");
            // nameLabelButton.setURL(URL(spaceInfo.gradio));
            modelAuthorLabel.setURL(URL(spaceInfo.gradio));
        }
        else if (spaceInfo.status == SpaceInfo::Status::STABILITY)
        {
            modelAuthorLabel.setURL(URL(spaceInfo.stability));
        }
        // spaceUrlButton.setFont(Font(15.00f, Font::plain));
        addAndMakeVisible(modelAuthorLabel);
        // modelAuthorLabelHandler.onMouseEnter = [this]()
        // {
        //     setInstructions("Click to visit "
        //                     + model->getGradioClient().getSpaceInfo().getModelSlashUser()
        //                     + "\nin your browser");
        // };
        // modelAuthorLabelHandler.onMouseExit = [this]() { clearInstructions(); };
        // modelAuthorLabelHandler.attach();
    }

    // now, we can enable the buttons
    if (model->ready())
    {
        processCancelButton.setEnabled(true);
        processCancelButton.setMode(processButtonInfo.label);
        tryLoadSavedToken();
    }

    loadModelButton.setEnabled(true);
    modelPathComboBox.setEnabled(true);
    loadModelButton.setButtonText("Load");

    // Set the focus to the process button
    // so that the user can press SPACE to trigger the playback
    // cb: I don't understand this.
    processCancelButton.grabKeyboardFocus();
    resized();
    repaint();
}
