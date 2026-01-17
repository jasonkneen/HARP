/*
 * @file ModelDisplayWidget.h
 * @brief TODO
 * @author cwitkowitz
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../gui/HoverableLabel.h"
#include "../gui/StatusComponent.h"
#include "ControlAreaWidget.h"
#include "TrackAreaWidget.h"

#include "../utils/Controls.h"
#include "../utils/Interface.h"

using namespace juce;

class ModelDisplayWidget : public Component
{
public:
    ~ModelDisplayWidget()
    {
        // remove listeners
        loadBroadcaster.removeChangeListener(this);
        processBroadcaster.removeChangeListener(this);
    }

    /*
    void initProcessCancelButton()
    {
        processCancelButton.setEnabled(false);

        processBroadcaster.addChangeListener(this);
        // saveEnabled = false;

        ModelStatus currentStatus = model->getStatus();
        if (currentStatus == ModelStatus::LOADED || currentStatus == ModelStatus::FINISHED)
        {
            processCancelButton.setEnabled(true);
            processCancelButton.setMode(processButtonInfo.label);
        }
        else if (currentStatus == ModelStatus::PROCESSING || currentStatus == ModelStatus::STARTING
                 || currentStatus == ModelStatus::SENDING)
        {
            processCancelButton.setEnabled(true);
            processCancelButton.setMode(cancelButtonInfo.label);
        }
        setStatus(currentStatus);
    }
    */

    void resetProcessingButtons()
    {
        processCancelButton.setMode(processButtonInfo.label);
        processCancelButton.setEnabled(true);
        // saveEnabled = true;
        // isProcessing = false;
        loadModelButton.setEnabled(true);
        modelPathComboBox.setEnabled(true);
        repaint();
    }

    void changeListenerCallback(ChangeBroadcaster* source)
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

        DBG_AND_LOG("HARPProcessorEditor::changeListenerCallback: unhandled change broadcaster");
        return;
    }
};
