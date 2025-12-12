#pragma once

#include "../AppSettings.h"
#include "../MainComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;

struct TutorialStep
{
    String title;
    String description;
    std::function<Rectangle<int>(MainComponent*)> getHighlightBounds;
};

class WelcomeWindow : public DocumentWindow
{
public:
    WelcomeWindow(MainComponent* mainComp)
        : DocumentWindow("Welcome to HARP",
                         Desktop::getInstance().getDefaultLookAndFeel().findColour(
                             ResizableWindow::backgroundColourId),
                         DocumentWindow::allButtons),
          mainComponent(mainComp)
    {
        setUsingNativeTitleBar(true);
        setResizable(true, true);
        setSize(500, 420);
        setAlwaysOnTop(true);

        // Define steps
        steps = {
            { "Welcome to HARP",
              "HARP is your hub for hosted, asynchronous, remote audio processing.\n\n"
              "HARP operates as a standalone app or plugin-like editor within your DAW, allowing you to process tracks using ML models hosted on platforms like Hugging Face or Stability AI.\n\n"
              "This quick tutorial will guide you through the main features of the application.",
              [](MainComponent*) { return Rectangle<int>(); } },

            { "Select a Model",
              "Start by choosing a model from the dropdown list. We support HuggingSpace, Stability AI, and custom endpoints.\n\n"
              "Click 'Load' to initialize the selected model.",
              [](MainComponent* c) { return c->getModelSelectBounds(); } },

            { "Configure Parameters",
              "Adjust the model parameters here. These controls change dynamically based on the selected model.\n\n"
              "You can hover over the components to view its description below in the status bar.",
              [](MainComponent* c) { return c->getControlsBounds(); } },

            { "Manage Tracks",
              "Select or drag and drop your audio or MIDI files onto the input tracks.\n"
              "Processed results will appear in the output tracks.",
              [](MainComponent* c) { return c->getTracksBounds(); } },

            { "Track Status",
              "\nKeep an eye on the bottom bar for status updates, processing progress, and helpful instructions.",
              [](MainComponent* c) { return c->getInfoBarBounds(); } },

            { "Media Clipboard",
              "Use the right-hand clipboard to manage your media files. You can drag processed files back to your DAW or reuse them as inputs.",
              [](MainComponent* c) { return c->getClipboardBounds(); } },

            { "All Set!",
              "You're ready to start creating!\n\n"
              "Don't forget to set up your API tokens in the Settings menu if you plan to use hosted models.\n"
              "Add tokens under File -> Settings -> Hugging Face",
              [](MainComponent*) { return Rectangle<int>(); } }
        };

        // UI Init
        addAndMakeVisible(&titleLabel);
        titleLabel.setFont(Font(24.0f, Font::bold));
        titleLabel.setJustificationType(Justification::centred);

        addAndMakeVisible(&descriptionLabel);
        descriptionLabel.setFont(Font(16.0f));
        descriptionLabel.setJustificationType(Justification::centredTop);

        addAndMakeVisible(&learnMoreLink);
        learnMoreLink.setButtonText("Learn more");
        learnMoreLink.setURL(URL("https://harp-plugin.netlify.app/content/intro.html"));
        learnMoreLink.setColour(HyperlinkButton::textColourId, Colours::skyblue);

        addAndMakeVisible(&copyrightLabel);
        copyrightLabel.setText("Copyright 2025 TEAMuP. All rights reserved.", dontSendNotification);
        copyrightLabel.setJustificationType(Justification::centred);
        copyrightLabel.setFont(Font(12.0f));
        copyrightLabel.setColour(Label::textColourId, Colours::grey);

        addAndMakeVisible(&nextButton);
        nextButton.setButtonText("Next");
        nextButton.onClick = [this] { nextStep(); };

        addAndMakeVisible(&prevButton);
        prevButton.setButtonText("Back");
        prevButton.onClick = [this] { prevStep(); };

        addAndMakeVisible(&skipButton);
        skipButton.setButtonText("Skip Tutorial");
        skipButton.onClick = [this] { skipTutorial(); };

        addAndMakeVisible(&dontShowAgainToggle);
        dontShowAgainToggle.setButtonText("Don't show this again");
        dontShowAgainToggle.setToggleState(false, dontSendNotification);

        addAndMakeVisible(&pageIndicator);
        pageIndicator.setJustificationType(Justification::centred);
        pageIndicator.setFont(Font(12.0f));

        // Initial Update
        updateStep();
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        // Reset highlight before closing
        if (mainComponent)
            mainComponent->setTutorialHighlight({});

        if (onClose)
            onClose();
    }

    std::function<void()> onClose;

    void paint(Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(20);

        // Header
        titleLabel.setBounds(area.removeFromTop(40));
        area.removeFromTop(10);

        // Footer Buttons
        auto footer = area.removeFromBottom(30);
        auto buttonWidth = 100;

        skipButton.setBounds(footer.removeFromLeft(buttonWidth));

        nextButton.setBounds(footer.removeFromRight(buttonWidth));
        footer.removeFromRight(10);
        prevButton.setBounds(footer.removeFromRight(buttonWidth));

        pageIndicator.setBounds(footer);

        // Spacer above footer
        area.removeFromBottom(10);

        if (currentStep == 0)
        {
            // Step 1 Specific Layout

            // Copyright remains at the very bottom
            copyrightLabel.setBounds(area.removeFromBottom(20));

            // Layout from top down to keep Link close to Text

            // Description Area - explicit height to contain the text without massive gap
            // 3 paragraphs of text at ~16px font
            descriptionLabel.setBounds(area.removeFromTop(200));

            // Link directly below description
            auto linkArea = area.removeFromTop(30);
            learnMoreLink.setBounds(linkArea.reduced(linkArea.getWidth() / 2 - 50, 0));

            // Remaining area is empty space between link and copyright
        }
        else
        {
            // Standard Layout for other steps

            // Checkbox area (only visible on last page)
            if (currentStep == (int) steps.size() - 1)
            {
                auto checkArea = area.removeFromBottom(30);
                dontShowAgainToggle.setBounds(checkArea.removeFromRight(200));
            }

            // Description fills the rest
            descriptionLabel.setBounds(area);
        }
    }

private:
    void updateStep()
    {
        if (steps.empty())
            return;

        const auto& step = steps[size_t(currentStep)];

        titleLabel.setText(step.title, dontSendNotification);
        descriptionLabel.setText(step.description, dontSendNotification);

        // Highlight logic
        if (mainComponent)
        {
            auto bounds = step.getHighlightBounds(mainComponent);
            mainComponent->setTutorialHighlight(bounds);
        }

        // Buttons
        prevButton.setVisible(currentStep > 0);

        bool isLast = currentStep == (int) steps.size() - 1;
        bool isFirst = currentStep == 0;

        nextButton.setButtonText(isLast ? "Finish" : "Next");
        skipButton.setVisible(! isLast);
        dontShowAgainToggle.setVisible(isLast);

        // Items specific to first page
        learnMoreLink.setVisible(isFirst);
        copyrightLabel.setVisible(isFirst);

        pageIndicator.setText("Step " + String(currentStep + 1) + " of " + String(steps.size()),
                              dontSendNotification);

        // Trigger layout update since we change component visibility and position logic
        resized();
    }

    void nextStep()
    {
        if (currentStep < (int) steps.size() - 1)
        {
            currentStep++;
            updateStep();
        }
        else
        {
            finishTutorial();
        }
    }

    void prevStep()
    {
        if (currentStep > 0)
        {
            currentStep--;
            updateStep();
        }
    }

    void skipTutorial()
    {
        currentStep = (int) steps.size() - 1;
        updateStep();
    }

    void finishTutorial()
    {
        if (dontShowAgainToggle.getToggleState())
        {
            AppSettings::setValue("showWelcomePopup", 0);
            AppSettings::saveIfNeeded();
        }

        closeButtonPressed();
    }

    MainComponent* mainComponent;
    std::vector<TutorialStep> steps;
    int currentStep = 0;

    Label titleLabel;
    Label descriptionLabel;
    TextButton nextButton;
    TextButton prevButton;
    TextButton skipButton;
    ToggleButton dontShowAgainToggle;
    Label pageIndicator;
    HyperlinkButton learnMoreLink;
    Label copyrightLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WelcomeWindow)
};