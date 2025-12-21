#include "../MainComponent.h"

void MainComponent::openCustomPathDialog(const std::string& prefillPath = "")
{
    // Create and show the custom path dialog with a callback
    std::function<void(const juce::String&)> loadCallback = [this](const juce::String& customPath2)
    {
        DBG("Custom path entered: " + customPath2);
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
