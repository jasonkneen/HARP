#include "../MainComponent.h"

void MainComponent::processCallback()
{
    if (model == nullptr)
    {
        AlertWindow(
            "Error", "Model is not loaded. Please load a model first.", AlertWindow::WarningIcon);
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

void MainComponent::cancelCallback()
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
