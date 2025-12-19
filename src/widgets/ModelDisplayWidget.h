/*
 * @file ModelDisplayWidget.h
 * @brief TODO
 * @author cwitkowitz
 */

#pragma once

#include "juce_gui_basics/juce_gui_basics.h"

#include "../gui/HoverableLabel.h"
#include "../utils.h"
#include "ControlAreaWidget.h"
#include "TrackAreaWidget.h"

using namespace juce;

class ModelAuthorLabel : public Component
{
   public:
    ModelAuthorLabel()
    {
        modelLabel.onHover = [this]
        { instructionBox->setStatusMessage("Click to view the model's page"); };

        modelLabel.onExit = [this]
        { instructionBox->clearStatusMessage(); };

        modelLabel.onClick = [this]
        { url.launchInDefaultBrowser(); };
        modelLabel.setHoverColour(Colours::coral);

        addAndMakeVisible(modelLabel);
        addAndMakeVisible(authorLabel);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        modelLabel.setFont(Font(22.0f, Font::bold));
        auto nameWidth = modelLabel.getFont().getStringWidthFloat(modelLabel.getText()) + 10;
        modelLabel.setBounds(area.removeFromLeft(static_cast<int>(nameWidth)));
        authorLabel.setBounds(area.translated(0, 3));
    }

    void paint(Graphics& g) override
    {
        g.fillAll(getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void setModelText(const String& name)
    {
        modelLabel.setText(name, dontSendNotification);
    }
    void setAuthorText(const String& name)
    {
        authorLabel.setText(name, dontSendNotification);
    }
    void setURL(const URL& newURL) { url = newURL; }

   private:
    HoverableLabel modelLabel;
    Label authorLabel;
    URL url;
    SharedResourcePointer<InstructionBox> instructionBox;
};

class ModelDisplayWidget : public Component
{
   public:
    ModelDisplayWidget()
    {
        // TODO: what happens if the model is nullptr rn?
        if (model == nullptr)
        {
            DBG("FATAL HARPProcessorEditor::HARPProcessorEditor: model is null");
            jassertfalse;
            return;
        }

        // add a status timer to update the status label periodically
        mModelStatusTimer = std::make_unique<ModelStatusTimer>(model);
        mModelStatusTimer->addChangeListener(this);
        mModelStatusTimer->startTimer(50); // 100 ms interval

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

        addAndMakeVisible(descriptionLabel);

        // model card component
        // Get the modelCard from the EditorView
        auto& card = model->card();
        setModelCard(card);
    }

    ~ModelDisplayWidget()
    {
        mModelStatusTimer->removeChangeListener(this);
    }

    void paint(Graphics& g)
    {
        // TODO
    }

    void resized() override
    {
        // Row 2: ModelName / AuthorName Labels
        juce::FlexBox row2;
        row2.flexDirection = juce::FlexBox::Direction::row;
        row2.items.add(juce::FlexItem(modelAuthorLabel).withFlex(0.5).withMargin(margin));
        row2.items.add(juce::FlexItem().withFlex(0.5).withMargin(margin));
        mainPanel.items.add(juce::FlexItem(row2).withHeight(30).withMargin(margin));

        // Row 3: Description
        auto font = Font(15.0f);
        descriptionLabel.setFont(font);
        // descriptionLabel.setColour(Label::backgroundColourId, Colours::red);
        auto maxLabelWidth = mainArea.getWidth() - 2 * margin;
        auto numberOfLines = font.getStringWidthFloat(descriptionLabel.getText(false)) / maxLabelWidth;
        float textHeight = (font.getHeight() + 5) * (std::floor(numberOfLines) + 1) + font.getHeight();
        mainPanel.items.add(juce::FlexItem(descriptionLabel).withHeight(textHeight).withMargin(margin));

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
        mainPanel.items.add(juce::FlexItem(rowProcessCancelButton).withHeight(30).withMargin(margin));

        // Row 6: Input Tracks Area Widget
        float numInputTracks = inputTrackAreaWidget.getNumTracks();
        float numOutputTracks = outputTrackAreaWidget.getNumTracks();
        float totalTracks = numInputTracks + numOutputTracks;

        if (numInputTracks > 0)
        {
            float inputTrackAreaFlex = 4 * (numInputTracks / totalTracks);
            mainPanel.items.add(juce::FlexItem(inputTracksLabel).withHeight(20).withMargin(margin));
            mainPanel.items.add(
                juce::FlexItem(inputTrackAreaWidget).withFlex(inputTrackAreaFlex).withMargin(margin));
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
            mainPanel.items.add(juce::FlexItem(outputTracksLabel).withHeight(20).withMargin(margin));
            mainPanel.items.add(
                juce::FlexItem(outputTrackAreaWidget).withFlex(outputTrackAreaFlex).withMargin(margin));
        }
        else
        {
            outputTracksLabel.setBounds(0, 0, 0, 0);
            outputTrackAreaWidget.setBounds(0, 0, 0, 0);
        }
    }

    void setModelCard(const ModelCard& card)
    {
        modelAuthorLabel.setModelText(String(card.name));
        descriptionLabel.setText(String(card.description), dontSendNotification);
        // set the author label text to "by {author}" only if {author} isn't empty
        card.author.empty() ? modelAuthorLabel.setAuthorText("")
                            : modelAuthorLabel.setAuthorText("by " + String(card.author));
        modelAuthorLabel.resized();
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
        processButtonInfo = MultiButton::Mode{
            "Process",
            [this]
            { processCallback(); },
            Colours::orangered,
            "Click to send the input for processing",
            MultiButton::DrawingMode::TextOnly,
            fontaudio::Pause,
        };

        cancelButtonInfo = MultiButton::Mode{
            "Cancel",
            [this]
            { cancelCallback(); },
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
        else if (currentStatus == ModelStatus::PROCESSING || currentStatus == ModelStatus::STARTING || currentStatus == ModelStatus::SENDING)
        {
            processCancelButton.setEnabled(true);
            processCancelButton.setMode(cancelButtonInfo.label);
        }
        setStatus(currentStatus);
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

   private:
    std::shared_ptr<WebModel> model{new WebModel()};

    std::unique_ptr<ModelStatusTimer> mModelStatusTimer{nullptr};

    ModelAuthorLabel modelAuthorLabel;

    Label descriptionLabel;

    ControlAreaWidget controlAreaWidget;

    Label inputTracksLabel{"Input Tracks", "Input Tracks"};
    TrackAreaWidget inputTrackAreaWidget{DisplayMode::Input};

    MultiButton processCancelButton;
    MultiButton::Mode processButtonInfo;
    MultiButton::Mode cancelButtonInfo;

    Label outputTracksLabel{"Output Tracks", "Output Tracks"};
    TrackAreaWidget outputTrackAreaWidget{DisplayMode::Output};

    std::unique_ptr<ModelStatusTimer> mModelStatusTimer{nullptr};
};
