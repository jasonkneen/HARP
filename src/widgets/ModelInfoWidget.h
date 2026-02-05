/**
 * @file ModelInfoWidget.h
 * @brief Component presenting model metadata and information.
 * @author hugofloresgarcia, xribene, cwitkowitz
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../widgets/StatusAreaWidget.h"

#include "../gui/HoverHandler.h"
#include "../gui/HoverableLabel.h"

#include "../utils/Logging.h"

using namespace juce;

class ModelAuthorLabel : public Component
{
public:
    ModelAuthorLabel(const String& modelName = "",
                     const String& author = "",
                     const URL& newURL = URL())
    {
        if (modelName.isNotEmpty())
        {
            setModelName(modelName);
        }

        if (author.isNotEmpty())
        {
            setAuthor(author);
        }

        setURL(newURL);

        modelLabel.setFont(Font(22.0f, Font::bold));

        modelLabel.onHover = [this]
        { instructionsMessage->setMessage("Click to view the model's webpage or documentation."); };
        modelLabel.onExit = [this] { instructionsMessage->clearMessage(); };
        modelLabel.onClick = [this] { url.launchInDefaultBrowser(); };

        addAndMakeVisible(modelLabel);
        addAndMakeVisible(authorLabel);
    }

    void resized() override
    {
        Rectangle<int> totalArea = getLocalBounds();

        float modelNameWidth = modelLabel.getFont().getStringWidthFloat(modelLabel.getText()) + 10;

        modelLabel.setBounds(totalArea.removeFromLeft(static_cast<int>(modelNameWidth)));
        authorLabel.setBounds(totalArea.translated(0, 3));
    }

    void setModelName(const String& modelName)
    {
        modelLabel.setText(modelName, dontSendNotification);

        resized();
    }
    void setAuthor(const String& author)
    {
        authorLabel.setText(author, dontSendNotification);

        resized();
    }
    void setURL(const URL& newURL)
    {
        if (newURL.isWellFormed())
        {
            url = newURL;

            modelLabel.setHoverColour(Colours::coral);
            modelLabel.setHoverable(true);

            resized();
        }
        else
        {
            modelLabel.setHoverColour(Colours::white);
            modelLabel.setHoverable(false);
        }
    }

private:
    HoverableLabel modelLabel;
    Label authorLabel;

    URL url;

    SharedResourcePointer<InstructionsMessage> instructionsMessage;
};

class ModelInfoWidget : public Component
{
public:
    ModelInfoWidget()
    {
        addAndMakeVisible(modelAuthorLabel);

        // Configure description as scrollable read-only text
        description.setMultiLine(true);
        description.setReadOnly(true);
        description.setScrollbarsShown(true);
        description.setCaretVisible(false); // no blinking cursor
        description.setPopupMenuEnabled(false); // disable right-click menu
        description.setFont(Font(15.0f));

        // Make it visually match the old TextLabel appearance
        description.setColour(TextEditor::backgroundColourId, Colours::transparentBlack);
        description.setColour(TextEditor::outlineColourId, Colours::transparentBlack);
        description.setColour(TextEditor::shadowColourId, Colours::transparentBlack);

        addAndMakeVisible(description);
    }

    ~ModelInfoWidget() {}

    //void paint(Graphics& g) {}

    void resized() override
    {
        Rectangle<int> bounds = getLocalBounds().reduced(marginSize);

        // Set fixed size for model and author labels
        modelAuthorLabel.setBounds(bounds.removeFromTop(headerHeight));
        // Grant remaining space to description
        description.setBounds(bounds);
    }

    void resetState()
    {
        ModelMetadata emptyMetadata;

        updateLabels(emptyMetadata);
        modelAuthorLabel.setURL(URL(""));
    }

    void updateLabels(const ModelMetadata& metadata)
    {
        if (metadata.name.empty())
        {
            modelAuthorLabel.setModelName("");
        }
        else
        {
            modelAuthorLabel.setModelName(String(metadata.name));
        }

        if (metadata.author.empty())
        {
            modelAuthorLabel.setAuthor("");
        }
        else
        {
            modelAuthorLabel.setAuthor("by " + String(metadata.author));
        }

        if (metadata.description.empty())
        {
            description.setText("");
        }
        else
        {
            description.setText(String(metadata.description));
        }

        resized();
    }

    void addOpenablePath(const String& openablePath) { modelAuthorLabel.setURL(URL(openablePath)); }

private:
    const float headerHeight = 30;
    const float marginSize = 2;

    ModelAuthorLabel modelAuthorLabel;

    TextEditor description;
};
