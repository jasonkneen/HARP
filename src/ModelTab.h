/*
 * @file ModelTab.h
 * @brief TODO
 * @author cwitkowitz
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "Model.h"

#include "widgets/ModelSelectionWidget.h"
//#include "widgets/ModelDisplayWidget.h"

#include "utils/Errors.h"
#include "utils/Logging.h"

using namespace juce;

class ModelTab : public Component, private ChangeListener
{
public:
    ModelTab()
    {
        addAndMakeVisible(modelSelectionWidget);
        //addAndMakeVisible(modelDisplayWidget);

        modelSelectionWidget.addChangeListener(this);
    }

    ~ModelTab() { modelSelectionWidget.removeChangeListener(this); }

    //void paint(Graphics& g) {}

    void resized() override
    {
        FlexBox tabArea;
        tabArea.flexDirection = FlexBox::Direction::column;

        tabArea.items.add(FlexItem(modelSelectionWidget).withHeight(30));

        tabArea.performLayout(getLocalBounds());
    }

private:
    void changeListenerCallback(ChangeBroadcaster* source)
    {
        if (source == &modelSelectionWidget)
        {
            loadModelCallback();
        }
    }

    void loadModelCallback()
    {
        modelSelectionWidget.setLoadingState();

        /* TODO - other state changes
        // disable the process button until the model is loaded
        //processCancelButton.setEnabled(false);
        */

        // Obtain currently selected path
        String selectedPath = modelSelectionWidget.getCurrentlySelectedPath();

        DBG_AND_LOG("ModelTab::loadModelCallback: Attempting to load path \"" << selectedPath
                                                                              << "\".");

        loadingThreadPool.addJob(
            [this, selectedPath]
            {
                OpResult result = model->loadPath(selectedPath);

                // Perform updates on message (GUI) thread
                MessageManager::callAsync(
                    [this, result]
                    {
                        if (result.wasOk())
                        {
                            modelSelectionWidget.setSuccessfulState();
                            // TODO - set other state for successful load here
                            // TODO - set model / author label URL to model's documentation URL
                            model->getLoadedPath();
                        }
                        else
                        {
                            const Error error = result.getError();

                            MessageBoxOptions errorPopup =
                                MessageBoxOptions()
                                    .withIconType(AlertWindow::WarningIcon)
                                    .withTitle(
                                        "Error") // TODO - Name of error family would be nice here
                                    // error ? toUserMessage(*error) : "An unknown error occurred."
                                    .withMessage(toUserMessage(error));

                            std::optional<String> openablePath = getOpenablePath(error);

                            if (openablePath.has_value())
                            {
                                errorPopup = errorPopup.withButton("Open URL");
                            }

                            errorPopup = errorPopup.withButton("Open Logs").withButton("Ok");

                            auto alertCallback = [this, error, openablePath, errorPopup](int choice)
                            {
                                DBG_AND_LOG(
                                    "ModelTab::loadModelCallback::alertCallback: Chose button index: "
                                    << choice << ".");

                                enum Choice
                                {
                                    OpenURL,
                                    OpenLogs,
                                    OK
                                };

                                /*
                                  TODO - The button indices assigned by MessageBoxOptions do not follow the order in which
                                  they were added. This should be fixed in JUCE v8. The following is a temporary workaround.

                                  See https://forum.juce.com/t/wrong-callback-value-for-alertwindow-showokcancelbox/55671/2

                                  When this is fixed, remove errorPopup from the argument list.
                                */
                                {
                                    std::map<int, int> observedButtonIndicesMap = {};

                                    if (errorPopup.getNumButtons() == 3)
                                    {
                                        observedButtonIndicesMap.insert({ 1, Choice::OpenURL });
                                    }

                                    observedButtonIndicesMap.insert(
                                        { errorPopup.getNumButtons() - 1, Choice::OpenLogs });

                                    observedButtonIndicesMap.insert({ 0, Choice::OK });

                                    choice = observedButtonIndicesMap[choice];
                                }

                                if (choice == Choice::OpenURL)
                                {
                                    URL(*openablePath).launchInDefaultBrowser();
                                }
                                else if (choice == Choice::OpenLogs)
                                {
                                    HARPLogger::getInstance()->getLogFile().revealToUser();
                                }
                                else
                                {
                                    // Nothing to do
                                }

                                modelSelectionWidget.setUnsuccessfulState(error);
                            };

                            AlertWindow::showAsync(errorPopup, alertCallback);
                        }
                    });
            });
    }

    /*void processLoadingResult(OpResult result)
    {
        // return;
        if (result.wasOk())
        {
            mModelStatusTimer->setModel(model);

            //Apply saved Stability token
            SpaceInfo spaceInfo = model->getClient().getSpaceInfo();
            if (spaceInfo.status == SpaceInfo::Status::STABILITY && ! savedStabilityToken.isEmpty())
            {
                model->getClient().setToken(savedStabilityToken);
                setStatus("Applied saved Stability AI token to loaded model.");
            }


            modelAuthorLabel.setURL(URL(spaceInfo.gradio));
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
    }*/

    // TODO - what does processing really consist of:
    //        - todo - input file paths
    //        - todo - client requests
    //        - todo - output file paths
    //        - todo - labels
    //        - tracking status / stages of processing
    //        - handling modes of error

    // TODO - what does canceling really consist of:
    //        - todo - TODO
    //        - tracking status / stages of canceling
    //        - handling modes of error

    const float marginSize = 2;

    std::shared_ptr<Model> model { new Model() };

    ModelSelectionWidget modelSelectionWidget;
    //ModelDisplayWidget modelDisplayWidget;

    ThreadPool loadingThreadPool { 1 };

    // TODO - cleanup below

    //ChangeBroadcaster loadBroadcaster;
};
