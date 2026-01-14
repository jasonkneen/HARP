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
    ModelDisplayWidget()
    {
        // TODO: what happens if the model is nullptr rn?
        if (model == nullptr)
        {
            DBG_AND_LOG("FATAL HARPProcessorEditor::HARPProcessorEditor: model is null");
            jassertfalse;
            return;
        }

        // model controls
        controlAreaWidget.setModel(model);
        addAndMakeVisible(controlAreaWidget);
        controlAreaWidget.populateControls();

        inputTracksLabel.setJustificationType(juce::Justification::centred);
        inputTracksLabel.setFont(juce::Font(20.0f, juce::Font::bold));
        addAndMakeVisible(inputTracksLabel);

        outputTracksLabel.setJustificationType(juce::Justification::centred);
        outputTracksLabel.setFont(juce::Font(20.0f, juce::Font::bold));
        addAndMakeVisible(outputTracksLabel);

        populateTracks();
        addAndMakeVisible(inputTrackAreaWidget);
        addAndMakeVisible(outputTrackAreaWidget);

        // model card component
        // Get the modelCard from the EditorView
        auto& card = model->card();
        setModelCard(card);
    }

    ~ModelDisplayWidget()
    {
        // remove listeners
        loadBroadcaster.removeChangeListener(this);
        processBroadcaster.removeChangeListener(this);
    }

    void paint(Graphics& g)
    {
        // TODO
    }

    void resized() override
    {
        int margin = 2;

        // Row 4: Control Area Widget
        // TODO - set min/max height based on limits of control element scaling
        mainPanel.items.add(juce::FlexItem(controlAreaWidget).withFlex(1).withMargin(margin));

        // Row 5: Process Cancel Button
        juce::FlexBox rowProcessCancelButton;
        rowProcessCancelButton.flexDirection = juce::FlexBox::Direction::row;
        rowProcessCancelButton.justifyContent = juce::FlexBox::JustifyContent::center;
        rowProcessCancelButton.items.add(juce::FlexItem().withFlex(1));
        rowProcessCancelButton.items.add(
            juce::FlexItem(processCancelButton).withWidth(150).withMargin(margin));
        rowProcessCancelButton.items.add(juce::FlexItem().withFlex(1));
        mainPanel.items.add(
            juce::FlexItem(rowProcessCancelButton).withHeight(30).withMargin(margin));

        // Row 6: Input Tracks Area Widget
        float numInputTracks = inputTrackAreaWidget.getNumTracks();
        float numOutputTracks = outputTrackAreaWidget.getNumTracks();
        float totalTracks = numInputTracks + numOutputTracks;

        if (numInputTracks > 0)
        {
            float inputTrackAreaFlex = 4 * (numInputTracks / totalTracks);
            mainPanel.items.add(juce::FlexItem(inputTracksLabel).withHeight(20).withMargin(margin));
            mainPanel.items.add(juce::FlexItem(inputTrackAreaWidget)
                                    .withFlex(inputTrackAreaFlex)
                                    .withMargin(margin));
        }
        else
        {
            inputTracksLabel.setBounds(0, 0, 0, 0);
            inputTrackAreaWidget.setBounds(0, 0, 0, 0);
        }

        // Row 7: Output Tracks Area Widget
        if (numOutputTracks > 0)
        {
            float outputTrackAreaFlex = 4 * (numOutputTracks / totalTracks);
            mainPanel.items.add(
                juce::FlexItem(outputTracksLabel).withHeight(20).withMargin(margin));
            mainPanel.items.add(juce::FlexItem(outputTrackAreaWidget)
                                    .withFlex(outputTrackAreaFlex)
                                    .withMargin(margin));
        }
        else
        {
            outputTracksLabel.setBounds(0, 0, 0, 0);
            outputTrackAreaWidget.setBounds(0, 0, 0, 0);
        }
    }

    void populateTracks()
    {
        for (const ComponentInfo& info : model->getInputTracksInfo())
        {
            inputTrackAreaWidget.addTrackFromComponentInfo(info);
        }

        for (const ComponentInfo& info : model->getOutputTracksInfo())
        {
            outputTrackAreaWidget.addTrackFromComponentInfo(info);
        }
    }

    void initProcessCancelButton()
    {
        // The Process/Cancel button
        processButtonInfo = MultiButton::Mode {
            "Process",
            [this] { processCallback(); },
            Colours::orangered,
            "Click to send the input for processing",
            MultiButton::DrawingMode::TextOnly,
            fontaudio::Pause,
        };

        cancelButtonInfo = MultiButton::Mode {
            "Cancel",
            [this] { cancelCallback(); },
            Colours::lightgrey,
            "Click to cancel the processing",
            MultiButton::DrawingMode::TextOnly,
            fontaudio::Pause,
        };

        processCancelButton.addMode(processButtonInfo);
        processCancelButton.addMode(cancelButtonInfo);
        processCancelButton.setMode(processButtonInfo.label);
        processCancelButton.setEnabled(false);
        addAndMakeVisible(processCancelButton);

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

    void resetUI()
    {
        controlAreaWidget.resetUI();
        inputTrackAreaWidget.resetUI();
        outputTrackAreaWidget.resetUI();
        // Also clear the model card components
        ModelCard empty;
        setModelCard(empty);
        // modelAuthorLabelHandler.detach();
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

    void processLoadingResult(OpResult result);

private
    ControlAreaWidget controlAreaWidget;

    Label inputTracksLabel { "Input Tracks", "Input Tracks" };
    TrackAreaWidget inputTrackAreaWidget { DisplayMode::Input };

    MultiButton processCancelButton;
    MultiButton::Mode processButtonInfo;
    MultiButton::Mode cancelButtonInfo;

    Label outputTracksLabel { "Output Tracks", "Output Tracks" };
    TrackAreaWidget outputTrackAreaWidget { DisplayMode::Output };
};
