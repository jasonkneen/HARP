/*
 * @file ModelTab.h
 * @brief TODO
 * @author cwitkowitz
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "Model.h"

#include "widgets/ControlAreaWidget.h"
#include "widgets/ModelInfoWidget.h"
#include "widgets/ModelSelectionWidget.h"
#include "widgets/TrackAreaWidget.h"

#include "utils/Errors.h"
#include "utils/Logging.h"

using namespace juce;

class ModelTab : public Component, private ChangeListener
{
public:
    ModelTab()
    {
        modelSelectionWidget.addChangeListener(this);

        addAndMakeVisible(modelSelectionWidget);
        addAndMakeVisible(modelInfoWidget);
        addAndMakeVisible(controlAreaWidget);

        inputTracksLabel.setJustificationType(Justification::centred);
        inputTracksLabel.setFont(Font(20.0f, Font::bold));

        addAndMakeVisible(inputTracksLabel);
        addAndMakeVisible(inputTrackAreaWidget);

        initializeProcessCancelButton();

        outputTracksLabel.setJustificationType(Justification::centred);
        outputTracksLabel.setFont(Font(20.0f, Font::bold));

        addAndMakeVisible(outputTracksLabel);
        addAndMakeVisible(outputTrackAreaWidget);
    }

    ~ModelTab() { modelSelectionWidget.removeChangeListener(this); }

    //void paint(Graphics& g) {}

    void resized() override
    {
        FlexBox tabArea;
        tabArea.flexDirection = FlexBox::Direction::column;

        /* Model Selection */

        tabArea.items.add(FlexItem(modelSelectionWidget).withHeight(30));

        /* Model Info */

        tabArea.items.add(FlexItem(modelInfoWidget).withFlex(1.0).withMaxHeight(100));

        /* Model Controls */

        if (controlAreaWidget.getNumControls() > 0)
        {
            // TODO - set min/max height based on limits of control element scaling
            tabArea.items.add(FlexItem(controlAreaWidget).withFlex(1.0).withMaxHeight(200));
        }
        else
        {
            controlAreaWidget.setBounds(0, 0, 0, 0);
        }

        /* Input Tracks Area Widget */

        float numInputTracks = inputTrackAreaWidget.getNumTracks();
        float numOutputTracks = outputTrackAreaWidget.getNumTracks();
        float totalTracks = numInputTracks + numOutputTracks;

        if (numInputTracks > 0)
        {
            float inputTrackAreaFlex = 4 * (numInputTracks / totalTracks);

            tabArea.items.add(FlexItem(inputTracksLabel).withHeight(20).withMargin(marginSize));
            tabArea.items.add(
                FlexItem(inputTrackAreaWidget).withFlex(inputTrackAreaFlex).withMargin(marginSize));
        }
        else
        {
            inputTracksLabel.setBounds(0, 0, 0, 0);
            inputTrackAreaWidget.setBounds(0, 0, 0, 0);
        }

        /* Process / Cancel Button */

        FlexBox processCancelButtonRow;
        processCancelButtonRow.flexDirection = FlexBox::Direction::row;

        if (model->isLoaded())
        {
            processCancelButtonRow.items.add(FlexItem().withFlex(1)); // Filler space
            processCancelButtonRow.items.add(
                FlexItem(processCancelButton).withWidth(150).withMargin(marginSize));
            processCancelButtonRow.items.add(FlexItem().withFlex(1)); // Filler space

            tabArea.items.add(
                FlexItem(processCancelButtonRow).withHeight(30).withMargin(marginSize));
        }
        else
        {
            processCancelButton.setBounds(0, 0, 0, 0);
        }

        /* Output Tracks Area Widget */

        if (numOutputTracks > 0)
        {
            float outputTrackAreaFlex = 4 * (numOutputTracks / totalTracks);

            tabArea.items.add(FlexItem(outputTracksLabel).withHeight(20).withMargin(marginSize));
            tabArea.items.add(FlexItem(outputTrackAreaWidget)
                                  .withFlex(outputTrackAreaFlex)
                                  .withMargin(marginSize));
        }
        else
        {
            outputTracksLabel.setBounds(0, 0, 0, 0);
            outputTrackAreaWidget.setBounds(0, 0, 0, 0);
        }

        tabArea.performLayout(getLocalBounds());
    }

    void resetState()
    {
        model.reset();

        modelSelectionWidget.resetState();
        modelInfoWidget.resetState();
        controlAreaWidget.resetState();
        inputTrackAreaWidget.resetState();
        outputTrackAreaWidget.resetState();

        processCancelButton.setMode(processButtonInfo.displayLabel);
        processCancelButton.setEnabled(false);
    }

private:
    void initializeProcessCancelButton()
    {
        // Mode when a model is loaded and not currently processing (process enabled)
        processButtonInfo =
            MultiButton::Mode { "Process",
                                "Click to execute model with selected parameters and inputs.",
                                [this] {}, //{ processCallback(); },
                                MultiButton::DrawingMode::TextOnly };
        // Mode when a model is loaded and currently processing (cancel enabled)
        cancelButtonInfo = MultiButton::Mode { "Cancel",
                                               "Click to cancel processing.",
                                               [this] {}, //{ cancelCallback(); },
                                               MultiButton::DrawingMode::TextOnly };

        processCancelButton.addMode(processButtonInfo);
        processCancelButton.addMode(cancelButtonInfo);
        processCancelButton.setMode(processButtonInfo.displayLabel);

        addAndMakeVisible(processCancelButton);
    }

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

        // Disable processing until next model is loaded
        processCancelButton.setEnabled(false);

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

                            modelInfoWidget.updateLabels(model->getMetadata());
                            modelInfoWidget.addOpenablePath(model->getOpenablePath());

                            controlAreaWidget.updateControls(model->getControls());

                            inputTrackAreaWidget.updateTracks(model->getInputTracks());
                            outputTrackAreaWidget.updateTracks(model->getOutputTracks());

                            resized();

                            // Re-enable processing immediately
                            processCancelButton.setEnabled(true);
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

                                // Re-enable processing after closing error window
                                processCancelButton.setEnabled(true);
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
            //Apply saved Stability token
            SpaceInfo spaceInfo = model->getClient().getSpaceInfo();
            if (spaceInfo.status == SpaceInfo::Status::STABILITY && ! savedStabilityToken.isEmpty())
            {
                model->getClient().setToken(savedStabilityToken);
                setStatus("Applied saved Stability AI token to loaded model.");
            }
        }

        // now, we can enable the buttons
        if (model->ready())
        {
            processCancelButton.setEnabled(true);
            processCancelButton.setMode(processButtonInfo.label);
            tryLoadSavedToken();
        }

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
    ModelInfoWidget modelInfoWidget;
    ControlAreaWidget controlAreaWidget;

    Label inputTracksLabel { "Input Tracks", "Input Tracks" };
    TrackAreaWidget inputTrackAreaWidget { DisplayMode::Input };

    MultiButton processCancelButton;
    MultiButton::Mode processButtonInfo;
    MultiButton::Mode cancelButtonInfo;

    Label outputTracksLabel { "Output Tracks", "Output Tracks" };
    TrackAreaWidget outputTrackAreaWidget { DisplayMode::Output };

    ThreadPool loadingThreadPool { 1 };

    // TODO - cleanup below

    //ChangeBroadcaster loadBroadcaster;
};
