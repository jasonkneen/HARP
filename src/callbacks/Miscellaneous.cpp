#include "../MainComponent.h"

void MainComponent::focusCallback()
{
    /*if (mediaDisplay->isFileLoaded())
    {
        Time lastModTime =
            mediaDisplay->getTargetFilePath().getLocalFile().getLastModificationTime();
        if (lastModTime > lastLoadTime)
        {
            // Create an AlertWindow
            auto* reloadCheckWindow = new AlertWindow(
                "File has been modified",
                "The loaded file has been modified in a different editor! Would you like HARP to load the new version of the file?\nWARNING: This will clear the undo log and cause all unsaved edits to be lost!",
                AlertWindow::QuestionIcon);

            reloadCheckWindow->addButton("Yes", 1, KeyPress(KeyPress::returnKey));
            reloadCheckWindow->addButton("No", 0, KeyPress(KeyPress::escapeKey));

            // Show the window and handle the result asynchronously
            reloadCheckWindow->enterModalState(
                true,
                new CustomPathAlertCallback(
                    [this, reloadCheckWindow](int result)
                    {
                        if (result == 1)
                        { // Yes was clicked
                            DBG("Reloading file");
                            loadMediaDisplay(mediaDisplay->getTargetFilePath().getLocalFile());
                        }
                        else
                        { // No was clicked or the window was closed
                            DBG("Not reloading file");
                            lastLoadTime =
                                Time::getCurrentTime(); //Reset time so we stop asking
                        }
                        delete reloadCheckWindow;
                    }),
                true);
        }
    }*/
}

void MainComponent::changeListenerCallback(ChangeBroadcaster* source)
{
    // The processBroadcaster should be also replaced in a similar way
    // as the loadBroadcaster (see processLoadingResult)
    if (source == &processBroadcaster)
    {
        // // refresh the display for the new updated file
        // URL tempFilePath = outputMediaDisplays[0]->getTempFilePath();
        // outputMediaDisplays[0]->updateDisplay(tempFilePath);

        // // extract generated labels from the model

        // // add the labels to the display component
        // outputMediaDisplays[0]->addLabels(labels);

        // The above commented code was for the case of a single output media display.
        // Now, we get from model all the outputPaths using model->getOutputPaths()
        // and we iterate over both outputMediaDisplays and outputPaths to update the display

        // Additionally, we filter the labels to only show the audio labels to audio output media displays
        // and midi labels to midi output media displays.

        LabelList& labels = model->getLabels();
        auto outputProcessedPaths = model->getOutputFilePaths();
        auto& outputMediaDisplays = outputTrackAreaWidget.getMediaDisplays();
        for (size_t i = 0; i < outputMediaDisplays.size(); ++i)
        {
            URL tempFile = outputProcessedPaths[i];
            outputMediaDisplays[i]->initializeDisplay(tempFile);
            outputMediaDisplays[i]->addLabels(labels);
        }
        // URL tempFilePath = outputProcessedPaths[0];
        // outputMediaDisplays[0]->setupDisplay(tempFilePath);

        // now, we can enable the process button
        resetProcessingButtons();
        return;
    }

    if (source == mModelStatusTimer.get())
    {
        // update the status label
        DBG("HARPProcessorEditor::changeListenerCallback: updating status label");
        // statusLabel.setText(model->getStatus(), dontSendNotification);
        setStatus(model->getStatus());
        return;
    }

    DBG("HARPProcessorEditor::changeListenerCallback: unhandled change broadcaster");
    return;
}
