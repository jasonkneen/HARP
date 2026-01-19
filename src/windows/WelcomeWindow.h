#pragma once

#include "../AppSettings.h"
#include "../MainComponent.h"
#include <JuceHeader.h>
#include <functional>
#include <vector>

using namespace juce;

struct TutorialStep
{
    String title;
    String description;
    std::function<Rectangle<int>(MainComponent*)> getHighlightBounds;
    std::function<std::vector<Rectangle<int>>(MainComponent*)> getExtraHighlights = nullptr;
};

class WelcomeWindow : public DocumentWindow, public ChangeListener
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

        if (mainComponent)
        {
            mainComponent->getLoadBroadcaster().addChangeListener(this);
            mainComponent->setTutorialActive(true);
        }

        // Initial Build
        rebuildSteps();

        // UI Init
        addAndMakeVisible(&titleLabel);
        titleLabel.setFont(Font(24.0f, Font::bold));
        titleLabel.setJustificationType(Justification::centred);

        addAndMakeVisible(&descriptionEditor);
        descriptionEditor.setMultiLine(true);
        descriptionEditor.setReadOnly(true);
        descriptionEditor.setScrollbarsShown(true);
        descriptionEditor.setCaretVisible(false);
        // Make it look like a label (transparent)
        descriptionEditor.setColour(TextEditor::backgroundColourId, Colours::transparentBlack);
        descriptionEditor.setColour(TextEditor::outlineColourId, Colours::transparentBlack);
        descriptionEditor.setFont(Font(16.0f));

        addAndMakeVisible(&learnMoreLink);
        learnMoreLink.setButtonText("Learn more");
        learnMoreLink.setURL(URL("https://harp-plugin.netlify.app/content/intro.html"));
        learnMoreLink.setColour(HyperlinkButton::textColourId, Colours::skyblue);

        // pyHarpLink init removed

        // Step 2 Labels init
        // hostingEditor init removed

        // endpointLabel removed

        addAndMakeVisible(&copyrightLabel);
        copyrightLabel.setText("Copyright 2025 TEAMuP. All rights reserved.", dontSendNotification);
        copyrightLabel.setJustificationType(Justification::centred);
        copyrightLabel.setFont(Font(12.0f));
        copyrightLabel.setColour(Label::textColourId, Colours::grey);

        addAndMakeVisible(&nextButton);
        nextButton.setButtonText("Next");
        nextButton.onClick = [this]
        {
            DBG("Next Button Clicked");
            nextStep();
        };

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
        pageIndicator.setInterceptsMouseClicks(false, false);

        addAndMakeVisible(&showDetailsButton);
        showDetailsButton.setButtonText("Show detailed control descriptions");
        showDetailsButton.onClick = [this]
        {
            showingDetails = ! showingDetails;
            showDetailsButton.setButtonText(showingDetails ? "Hide detailed control descriptions"
                                                           : "Show detailed control descriptions");
            rebuildSteps();
            updateStep();
        };

        // Initial Update
        updateStep();
        setVisible(true);
    }

    ~WelcomeWindow() override
    {
        if (mainComponent)
        {
            mainComponent->getLoadBroadcaster().removeChangeListener(this);
            mainComponent->setTutorialActive(false);
        }
    }

    void changeListenerCallback(ChangeBroadcaster*) override
    {
        // Model loaded/changed
        rebuildSteps();

        if (currentStep > 0)
        {
            // Auto-jump to Step 3 ("Quick Start") if user loads a new model
            // (Index 2 corresponds to "Quick Start")
            if (steps.size() > 2)
            {
                currentStep = 2;
            }
            updateStep();
        }
    }

    void rebuildSteps()
    {
        steps.clear();

        // 1. Static Intro
        steps.push_back(
            { "Welcome to HARP",
              "HARP is your hub for hosted, asynchronous, remote audio processing.\n\n"
              "HARP operates as a standalone app or plugin-like editor within your DAW, allowing you to process tracks using ML models hosted on platforms like Hugging Face or Stability AI.\n\n"
              "This quick tutorial will guide you through the main features of the application. Click 'Learn more' for additional online documentation.",
              [](MainComponent*) { return Rectangle<int>(); } });

        // 2. Select Model
        steps.push_back(
            { "Select a Model",
              "Select a model from the dropdown menu at the top and click Load.\n\n"
              "Once loaded, the model's details and controls will appear below.\n\n"
              "If you don't select another model, HARP uses the Demucs Stem Separator by default.",
              [](MainComponent* c) { return c->getModelSelectBounds(); } });

        // 3. Model Overview (Dynamic)
        String modelName = "Demucs Source Separation"; // Default
        String modelId = "demucs";

        if (mainComponent)
        {
            auto model = mainComponent->getModel();
            if (model && model->ready())
            {
                modelName = model->card().name;
                modelId = modelName; // Use full name for matching
            }
        }

        // 3. Quick Start (Dynamic)
        String stepTitle = "Quick Start";

        steps.push_back(
            { stepTitle,
              "This is the " + modelName + ".\n" + getFriendlyModelSummary(modelName)
                  + "\n\n"
                    "1. Add input\n"
                    "Drag an audio or MIDI file into the Input track, use the folder icon to browse, or the play button to preview.\n\n"
                    "2. Process\n"
                    "Click Process to run the model.\n\n"
                    "3. Output\n"
                    "Processed tracks appear in the Output section, where you can play or save them.",
              [](MainComponent* c)
              {
                  auto bounds = c->getLocalBounds();
                  auto clipboard = c->getClipboardBounds();
                  if (! clipboard.isEmpty())
                  {
                      // Exclude clipboard (assume it's on the right)
                      bounds.setRight(clipboard.getX());
                  }
                  return bounds;
              }, // Highlight entire interface except clipboard
              [](MainComponent* c)
              {
                  std::vector<Rectangle<int>> v;
                  v.push_back(c->getInputFolderBounds());
                  v.push_back(c->getInputPlayBounds());
                  v.push_back(c->getProcessButtonBounds());
                  return v;
              } });

        // 4. Configure Parameters (Dynamic)
        // 4. Configure Parameters (Dynamic)
        if (mainComponent)
        {
            auto model = mainComponent->getModel();
            if (model && model->ready())
            {
                String stepTitle = "Configure Parameters (Optional)";
                String baseText =
                    "Some models will have controls to customize model behavior. Typical controls include ones that balance quality vs speed, allow selection of model variants, or provide advanced options for experienced users.\n\n"
                    "To see descriptions of what controls do for a model Click on the button below. Do this now to see the control descriptions for "
                    + modelName + ".\n\n";

                String fullText = baseText;

                if (showingDetails)
                {
                    fullText +=
                        "--------------------------------------------------\nDetailed Control Descriptions:\n\n";
                    auto& controls = model->getControlsInfo();
                    if (controls.empty())
                    {
                        fullText += "- No adjustable controls for this model.";
                    }
                    else
                    {
                        int index = 1;

                        // First pass: All except Model
                        for (const auto& pair : controls)
                        {
                            if (String(pair.second->label).equalsIgnoreCase("Model"))
                                continue;

                            auto info = pair.second;
                            String friendlyCtrlDesc = getFriendlyControlDescription(
                                model->card().name, info->label, info->info);

                            fullText += String(index++) + ". " + info->label + ": "
                                        + friendlyCtrlDesc + "\n\n";
                        }

                        // Second pass: Only Model
                        for (const auto& pair : controls)
                        {
                            if (! String(pair.second->label).equalsIgnoreCase("Model"))
                                continue;

                            auto info = pair.second;
                            String friendlyCtrlDesc = getFriendlyControlDescription(
                                model->card().name, info->label, info->info);

                            fullText += String(index++) + ". " + info->label + ": "
                                        + friendlyCtrlDesc + "\n\n";
                        }
                        fullText +=
                            "Tip: We suggest sticking with the default parameters for the best balance of processing time vs. quality.";
                    }
                }

                steps.push_back({ stepTitle, fullText, [](MainComponent* c) {
                                     return c->getControlsBounds();
                                 } });
            }
        }

        // 5. Manage Tracks
        steps.push_back(
            { "Manage Tracks",
              "The highlighted panel contains your input and output tracks.\n\n"
              "You can drag and drop audio/MIDI files here, or click on the folder icon in the input audio section to choose from a local folder.\n\n"
              "Processed results will appear in the output tracks section, which you can play here or save directly.\n\n"
              "Please note that these tracks interact with the model.",
              [](MainComponent* c) { return c->getTracksBounds(); },
              nullptr });

        steps.push_back(
            { "Track Status",
              "The bottom bar shows the connection and processing status.\n\n"
              "LOADED: The model is ready to use.\n\n"
              "PROCESSING: The model is busy working. Please wait.\n\n"
              "ERROR: Something went wrong. Check the description below it and try reloading the model.",
              [](MainComponent* c) { return c->getInfoBarBounds(); },
              nullptr });

        steps.push_back(
            { "Media Clipboard",
              "This clipboard is a scratch space. Tracks here do not interact with the models directly.\n\n"
              "However, you can stash model outputs here to reuse them later.",
              [](MainComponent* c) { return c->getClipboardBounds(); },
              nullptr });

        // 8. Interface Summary
        steps.push_back(
            { "Interface Summary",
              "Top Bar: Select a model from the dropdown and click Load to initialize it.\n\n"
              "Left Panel: This is where your Input and Output tracks live. Models read from and write to these tracks.\n\n"
              "Right Panel: A scratch pad (Clipboard) to stash tracks you want to save or reuse later.",
              [](MainComponent* c) { return Rectangle<int>(); },
              [](MainComponent* c)
              {
                  std::vector<Rectangle<int>> v;
                  v.push_back(c->getModelSelectBounds());
                  v.push_back(c->getInputTrackBounds());
                  v.push_back(c->getClipboardBounds());
                  return v;
              } });

        steps.push_back(
            { "All Set!",
              "Most models in HARP are hosted on external service like Hugging Face and Stability AI. To use them, you will need API tokens.\n\n"
              "An API token is a private access key that lets HARP securely connect to these services on your behalf. It tells the service that you are allowed to use the model\n\n"
              "You can get these tokens from your account settings on the respective service websites. You can find the exact link in 'File -> Settings' to help you add your token.\n\n"
              "You are now ready to start creating!",
              [](MainComponent*) { return Rectangle<int>(); } });

        // Refresh if we are showing the tutorial
        if (isVisible())
        {
            if (currentStep >= (int) steps.size())
                currentStep = (int) steps.size() - 1;
            updateStep();
        }
    }

    String getFriendlyModelSummary(const String& name)
    {
        if (name.containsIgnoreCase("demucs"))
        {
            return "It separates music audio into drums, bass, vocals, and 'other' stems.";
        }
        if (name.containsIgnoreCase("megatts") || name.containsIgnoreCase("voice"))
        {
            return "It generates realistic speech or clones voices from reference audio.";
        }
        if (name.containsIgnoreCase("musicgen") || name.containsIgnoreCase("audioldm"))
        {
            return "It generates new music or audio based on text descriptions.";
        }
        if (name.containsIgnoreCase("stability") || name.containsIgnoreCase("stable audio"))
        {
            return "It generates or transforms audio using state-of-the-art diffusion models.";
        }

        return "It is an advanced audio processing model hosted in the cloud.";
    }

    String getFriendlyModelOverview(const String& name)
    {
        // DEMUCS (Default or Selected)
        if (name.containsIgnoreCase("demucs"))
        {
            return "About:\n"
                   "The Demucs model is a state-of-the-art music source separation model. "
                   "It separates a stereo specific mixture into four distinct tracks: Drums, Bass, Vocals, and Other instruments.\n\n"
                   "Inputs:\n"
                   "A generic audio file (mp3, wav, etc.) containing a song or music mix.\n\n"
                   "Outputs:\n"
                   "Four separate audio tracks corresponding to the separated stems.";
        }

        // MEGATTS / Voice
        if (name.containsIgnoreCase("megatts") || name.containsIgnoreCase("voice"))
        {
            return "About:\n"
                   "This model generates realistic speech or clones voices from reference audio.\n\n"
                   "Inputs:\n"
                   "Reference audio of the voice to clone and the text you want it to speak.\n\n"
                   "Outputs:\n"
                   "A generated audio clip of the spoken text.";
        }

        // MUSICGEN / Audio Generation
        if (name.containsIgnoreCase("musicgen") || name.containsIgnoreCase("audioldm"))
        {
            return "About:\n"
                   "Generates new music or audio based on text descriptions.\n\n"
                   "Inputs:\n"
                   "A text prompt describing the music (e.g., 'lo-fi beat with piano').\n\n"
                   "Outputs:\n"
                   "A generated audio track matching the description.";
        }

        // Fallback
        return "About:\n"
               "This is an advanced audio processing model hosted in the cloud.\n\n"
               "Inputs:\n"
               "Refer to the specific model documentation or parameters for input requirements.\n\n"
               "Outputs:\n"
               "Processed audio or MIDI results.";
    }

    String getFriendlyModelDescription(const String& /*name*/, const String& rawDesc)
    {
        // Unused but kept for API consistency if needed later
        return rawDesc;
    }

    String getFriendlyControlDescription(const String& modelName,
                                         const String& label,
                                         const String& rawInfo)
    {
        // DEMUCS
        if (modelName.containsIgnoreCase("demucs"))
        {
            if (label.equalsIgnoreCase("shifts"))
            {
                // Roman numerals for sub-points
                return "Determines how many times the audio is shifted and processed.\n"
                       "     i. Increasing this value can improve quality but will take longer to process.\n"
                       "     ii. A value of 0 or 1 is usually sufficient for good results.";
            }
            if (label.equalsIgnoreCase("overlap"))
            {
                return "Controls the smoothness between processed audio chunks. The default value is optimized for a seamless sound.";
            }
        }

        // STABILITY AI (Text-to-Audio / Audio-to-Audio)
        if (modelName.containsIgnoreCase("stability")
            || modelName.containsIgnoreCase("text-to-audio")
            || modelName.containsIgnoreCase("audio-to-audio")
            || modelName.containsIgnoreCase("stable audio"))
        {
            if (label.containsIgnoreCase("Duration"))
                return "Sets the length of the generated audio in seconds.";

            if (label.containsIgnoreCase("steps"))
                return "Number of diffusion steps. Higher values improve quality but take longer to process.";

            if (label.containsIgnoreCase("cfg"))
                return "Classifier Free Guidance. Higher values make the output adhere more strictly to the prompt.";

            if (label.containsIgnoreCase("Output Format"))
                return "The file format for the generated audio (e.g. wav).";

            if (label.containsIgnoreCase("Text Prompt"))
                return "Description of the audio content you want to generate.";
        }

        // MEGATTS 3
        if (modelName.containsIgnoreCase("megatts"))
        {
            if (label.containsIgnoreCase("timesteps"))
            {
                return "Controls the quality of the voice generation.\n"
                       "     i. Higher values (e.g. 50+) result in clearer, higher-quality audio but take longer to generate.";
            }
            if (label.containsIgnoreCase("intelligibility"))
            {
                return "Ensures the generated speech is clear and easy to understand.\n"
                       "     i. Increase this value if the output sounds mumbled or unclear.";
            }
            if (label.containsIgnoreCase("similarity"))
            {
                return "Controls how closely the cloned voice mimics the reference audio.\n"
                       "     i. Increase this for a stronger likeness to the original voice.";
            }
            if (label.containsIgnoreCase("text"))
            {
                return "Enter the text you want the cloned voice to speak.";
            }
        }

        // TRIA
        if (modelName.containsIgnoreCase("tria"))
        {
            if (label.equalsIgnoreCase("Model"))
            {
                return "Choose which variant to you want to use. Currently this is the only one available, however more models would be available soon.";
            }
        }

        // GENERIC FALLBACK
        if (rawInfo.isNotEmpty())
            return rawInfo;

        return "Controls this specific parameter of the model.";
    }

    void closeButtonPressed() override
    {
        // Reset state before closing
        if (mainComponent)
        {
            mainComponent->setTutorialHighlight({});
            mainComponent->setTutorialActive(false);
        }

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

        area.removeFromBottom(10); // Spacer

        if (currentStep == 0)
        {
            copyrightLabel.setBounds(area.removeFromBottom(20));
            descriptionEditor.setBounds(area.removeFromTop(200));
            auto linkArea = area.removeFromTop(30);
            learnMoreLink.setBounds(linkArea.reduced(linkArea.getWidth() / 2 - 50, 0));
        }
        else
        {
            if (currentStep == (int) steps.size() - 1)
            {
                auto checkArea = area.removeFromBottom(30);
                dontShowAgainToggle.setBounds(checkArea.removeFromRight(200));
            }

            // Fix for UI overlap: Reserve space for the "show details" button at the bottom FIRST
            if (showDetailsButton.isVisible())
            {
                auto btnArea = area.removeFromBottom(30);
                showDetailsButton.setBounds(btnArea.reduced(20, 0));
                // Add some spacing between button and description
                area.removeFromBottom(10);
            }

            // Finally, the description editor takes the remaining space
            descriptionEditor.setBounds(area);
        }
    }

private:
    void updateStep()
    {
        if (steps.empty())
            return;

        const auto& step = steps[size_t(currentStep)];

        titleLabel.setText(step.title, dontSendNotification);
        descriptionEditor.setText(step.description);
        // Scroll to top when changing steps
        descriptionEditor.setCaretPosition(0);

        // Highlight logic
        {
            auto bounds = step.getHighlightBounds(mainComponent);
            mainComponent->setTutorialHighlight(bounds);

            if (step.getExtraHighlights)
            {
                mainComponent->setTutorialExtraHighlights(step.getExtraHighlights(mainComponent));
            }
            else
            {
                mainComponent->setTutorialExtraHighlights({});
            }
        }

        // Buttons
        prevButton.setVisible(currentStep > 0);

        bool isLast = currentStep == (int) steps.size() - 1;
        bool isFirst = currentStep == 0;
        // bool isSecond = currentStep == 1; // "Select a Model"

        nextButton.setButtonText(isLast ? "Finish" : "Next");
        skipButton.setVisible(! isLast);
        dontShowAgainToggle.setVisible(isLast);

        // Items specific to first page
        learnMoreLink.setVisible(isFirst);
        copyrightLabel.setVisible(isFirst);

        // Item specific to second page (Step 2)
        // hostingEditor removed
        // pyHarpLink removed

        pageIndicator.setText("Step " + String(currentStep + 1) + " of " + String(steps.size()),
                              dontSendNotification);

        // Show details button only on Configure Parameters step
        bool isConfigParams = step.title.contains("Configure Parameters");
        showDetailsButton.setVisible(isConfigParams);

        // Trigger layout update
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
    TextEditor descriptionEditor; // Changed from Label

    // Step 2 specific labels removed

    TextButton nextButton;
    TextButton prevButton;
    TextButton skipButton;
    ToggleButton dontShowAgainToggle;
    Label pageIndicator;
    HyperlinkButton learnMoreLink;
    // HyperlinkButton pyHarpLink; // Removed
    Label copyrightLabel;

    TextButton showDetailsButton;
    bool showingDetails = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WelcomeWindow)
};