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

    // TODO - what does processing really consist of:
    //        - todo - input file paths
    //        - todo - client requests
    //        - todo - output file paths
    //        - todo - labels
    //        - tracking status / stages of processing
    //        - handling modes of error

    /*
    void processCallback()
    {
        if (model == nullptr)
        {
            AlertWindow("Error",
                        "Model is not loaded. Please load a model first.",
                        AlertWindow::WarningIcon);
            return;
        }

        // Get new processID
        String processID = juce::Uuid().toString();
        processMutex.lock();
        currentProcessID = processID;
        DBG_AND_LOG("Set Process ID: " + processID);
        processMutex.unlock();

        processCancelButton.setEnabled(true);
        processCancelButton.setMode(cancelButtonInfo.label);
        loadModelButton.setEnabled(false);
        modelPathComboBox.setEnabled(false);
        //saveEnabled = false;
        //isProcessing = true;

        // mediaDisplay->addNewTempFile();
        auto& inputMediaDisplays = inputTrackAreaWidget.getMediaDisplays();

        // Get all the getTempFilePaths from the inputMediaDisplays
        // and store them in a map/dictionary with the track name as the key
        std::vector<std::tuple<Uuid, String, File>> localInputTrackFiles;
        for (auto& inputMediaDisplay : inputMediaDisplays)
        {
            if (! inputMediaDisplay->isFileLoaded() && inputMediaDisplay->isRequired())
            {
                AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                                                 "Error",
                                                 "Input file is not loaded for track "
                                                     + inputMediaDisplay->getTrackName()
                                                     + ". Please load an input file first.");
                processCancelButton.setMode(processButtonInfo.label);
                //isProcessing = false;
                //saveEnabled = true;
                loadModelButton.setEnabled(true);
                modelPathComboBox.setEnabled(true);
                return;
            }
            if (inputMediaDisplay->isFileLoaded())
            {
                //inputMediaDisplay->addNewTempFile();
                localInputTrackFiles.push_back(
                    std::make_tuple(inputMediaDisplay->getDisplayID(),
                                    inputMediaDisplay->getTrackName(),
                                    //inputMediaDisplay->getTempFilePath().getLocalFile()));
                                    inputMediaDisplay->getOriginalFilePath().getLocalFile()));
            }
        }

        // Directly add the job to the thread pool
        jobProcessorThread.addJob(
            new CustomThreadPoolJob(
                [this, localInputTrackFiles](String jobProcessID) { // &jobsFinished, totalJobs
                    // Individual job code for each iteration
                    // copy the audio file, with the same filename except for an added _harp to the stem
                    OpResult processingResult =
                        model->process(localInputTrackFiles);
                    processMutex.lock();
                    if (jobProcessID != currentProcessID)
                    {
                        DBG_AND_LOG("ProcessID " + jobProcessID + " not found");
                        DBG_AND_LOG("NumJobs: " + std::to_string(jobProcessorThread.getNumJobs()));
                        DBG_AND_LOG("NumThrds: " + std::to_string(jobProcessorThread.getNumThreads()));
                        processMutex.unlock();
                        return;
                    }
                    if (processingResult.failed())
                    {
                        Error processingError = processingResult.getError();
                        Error::fillUserMessage(processingError);
                        DBG_AND_LOG("Error in Processing:\n"
                                  + processingError.devMessage.toStdString());
                        AlertWindow::showMessageBoxAsync(
                            AlertWindow::WarningIcon,
                            "Processing Error",
                            "An error occurred while processing the audio file: \n"
                                + processingError.userMessage);
                        // cb: I commented this out, and it doesn't seem to change anything
                        // it was also causing a crash. If we need it, it needs to run on
                        // the message thread using MessageManager::callAsync
                        // hy: Now this line works.
                        // resetProcessingButtons();
                        // cb: Needs to be in the message thread or else it crashes
                        // It's used when the processing fails to reset the process/cancel
                        // button back to the process mode.
                        MessageManager::callAsync([this] { resetProcessingButtons(); });
                        processMutex.unlock();
                        return;
                    }
                    // load the audio file again
                    DBG_AND_LOG("ProcessID " + jobProcessID + " succeed");
                    currentProcessID = "";
                    model->setStatus(ModelStatus::FINISHED);
                    processBroadcaster.sendChangeMessage();
                    processMutex.unlock();
                },
                processID),
            true);
        DBG_AND_LOG("NumJobs: " + std::to_string(jobProcessorThread.getNumJobs()));
        DBG_AND_LOG("NumThrds: " + std::to_string(jobProcessorThread.getNumThreads()));
    }
    */

    // TODO - what does canceling really consist of:
    //        - todo - TODO
    //        - tracking status / stages of canceling
    //        - handling modes of error

    /*
    void cancelCallback()
    {
        DBG_AND_LOG("HARPProcessorEditor::buttonClicked cancel button listener activated");

        OpResult cancelResult = model->cancel();

        if (cancelResult.failed())
        {
            DBG_AND_LOG(cancelResult.getError().devMessage.toStdString());
            AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                                             "Cancel Error",
                                             "An error occurred while cancelling the processing: \n"
                                                 + cancelResult.getError().devMessage);
            return;
        }
        // Update current process to empty
        processMutex.lock();
        DBG_AND_LOG("Cancel ProcessID: " + currentProcessID);
        currentProcessID = "";
        processMutex.unlock();
        // We already added a temp file, so we need to undo that
        // TODO: this is functionality that I need to add back // #TODO
        // mediaDisplay->iteratePreviousTempFile();
        // mediaDisplay->clearFutureTempFiles();

        // processCancelButton.setEnabled(false); // this is the og v3
        resetProcessingButtons(); // This is the new way
    }
    */

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

    /* Processing */

    //String currentProcessID;

    //std::mutex processMutex;

    //ChangeBroadcaster processBroadcaster;

    //ThreadPool jobProcessorThread { 10 };

    //bool isProcessing = false;
};
