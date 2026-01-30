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
                                [this] { processCallback(); },
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

        /*
        if (source == &processBroadcaster)
        {
            // TODO - load output file paths into display components
        }
        */
    }

    void openErrorPopup(const Error error, std::function<void()> onExit)
    {
        MessageBoxOptions errorPopup =
            MessageBoxOptions()
                .withIconType(AlertWindow::WarningIcon)
                .withTitle("Error") // TODO - Name of error family would be nice here
                // error ? toUserMessage(*error) : "An unknown error occurred."
                .withMessage(toUserMessage(error));

        std::optional<String> openablePath = getOpenablePath(error);

        if (openablePath.has_value())
        {
            errorPopup = errorPopup.withButton("Open URL");
        }

        errorPopup = errorPopup.withButton("Open Logs").withButton("Ok");

        auto alertCallback = [this, error, openablePath, onExit, errorPopup](int choice)
        {
            DBG_AND_LOG("ModelTab::loadModelCallback::alertCallback: Chose button index: " << choice
                                                                                           << ".");

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

            if (onExit)
            {
                // Perform optional state cleanup
                onExit();
            }
        };

        AlertWindow::showAsync(errorPopup, alertCallback);
    }

    void loadModelCallback()
    {
        modelSelectionWidget.setDisabled();

        // Disable processing until model is loaded
        processCancelButton.setEnabled(false);

        // Obtain currently selected path
        String selectedPath = modelSelectionWidget.getCurrentlySelectedPath();

        DBG_AND_LOG("ModelTab::loadModelCallback: Attempting to load path \"" << selectedPath
                                                                              << "\".");

        loadingThreadPool.addJob(
            [this, selectedPath]
            {
                OpResult result = model->load(selectedPath);

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

                            std::function<void()> onExit = [this, error]
                            {
                                modelSelectionWidget.setUnsuccessfulState(error);

                                // Re-enable processing after closing error window
                                processCancelButton.setEnabled(true);
                            };

                            openErrorPopup(error, onExit);
                        }
                    });
            });
    }

    void processCallback()
    {
        /*
        // Get new processID
        String processID = juce::Uuid().toString();
        processMutex.lock();
        currentProcessID = processID;
        DBG_AND_LOG("Set Process ID: " + processID);
        processMutex.unlock();
        */

        modelSelectionWidget.setDisabled();
        processCancelButton.setMode(cancelButtonInfo.displayLabel);

        std::map<Uuid, File> loadedInputFiles;

        for (std::unique_ptr<MediaDisplayComponent>& inputTrack :
             inputTrackAreaWidget.getMediaDisplays())
        {
            if (inputTrack->isRequired() && ! inputTrack->isFileLoaded())
            {
                // Make sure all required inputs have been set
                AlertWindow::showMessageBoxAsync(
                    AlertWindow::WarningIcon,
                    "Error",
                    "Required input track \"" + inputTrack->getTrackName()
                        + "\" is empty. Please load a file before processing.");

                return;
            }
            else if (inputTrack->isFileLoaded())
            {
                loadedInputFiles[inputTrack->getTrackID()] =
                    inputTrack->getOriginalFilePath().getLocalFile();
            }
            else
            {
                // Optional track skipped
            }
        }

        /*
        // Directly add the job to the thread pool
        jobProcessorThread.addJob(
            new CustomThreadPoolJob(
                [this, localInputTrackFiles](String jobProcessID) { // &jobsFinished, totalJobs
                    // Individual job code for each iteration
                    // copy the audio file, with the same filename except for an added _harp to the stem
        */

        processingThreadPool.addJob(
            [this, loadedInputFiles]
            {
                std::vector<File> outputFiles;
                LabelList labels;

                OpResult result = model->process(loadedInputFiles, outputFiles, labels);

                auto outputFilesPtr = std::make_shared<std::vector<File>>(std::move(outputFiles));
                auto labelsPtr = std::make_shared<LabelList>(std::move(labels));

                // Perform updates on message (GUI) thread
                MessageManager::callAsync(
                    [this, result, outputFilesPtr, labelsPtr]
                    {
                        std::function<void()> onExit = [this]
                        {
                            // Re-enable processing immediately
                            modelSelectionWidget
                                .setFinishedState(); // TODO - should this be last selected?
                            processCancelButton.setMode(processButtonInfo.displayLabel);
                        };

                        if (result.wasOk())
                        {
                            auto& outputMediaDisplays = outputTrackAreaWidget.getMediaDisplays();

                            for (size_t i = 0; i < outputMediaDisplays.size(); ++i)
                            {
                                outputMediaDisplays[i]->initializeDisplay(
                                    URL((*outputFilesPtr)[i]));
                                outputMediaDisplays[i]->addLabels(*labelsPtr);
                            }

                            onExit();
                        }
                        else
                        {
                            openErrorPopup(result.getError(), onExit);
                        }
                    });
            });

        /*
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
        */
    }

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
    ThreadPool processingThreadPool { 1 };

    // TODO - cleanup below

    //String currentProcessID;

    //std::mutex processMutex;

    //ThreadPool jobProcessorThread { 10 };
};
