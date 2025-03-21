/**
 * @file CustomPathDialog.cpp
 * @brief Custom dialog for entering a custom path in the HARPPlugin
 * @author xribene
 */

#pragma once

#include "juce_gui_basics/juce_gui_basics.h"
#include <functional>

inline Colour getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour uiColour,
                                     Colour fallback = Colour(0xff4d4d4d)) noexcept
{
    if (auto* v4 = dynamic_cast<LookAndFeel_V4*>(&LookAndFeel::getDefaultLookAndFeel()))
        return v4->getCurrentColourScheme().getUIColour(uiColour);

    return fallback;
}

class CustomPathComponent : public Component
{
public:
    CustomPathComponent(std::function<void(const String&)> onLoadCallback,
                        std::function<void()> onCancelCallback)
    {
        // Set up the TextEditor for path input
        addAndMakeVisible(customPathEditor);
        customPathEditor.setMultiLine(false);
        customPathEditor.setReturnKeyStartsNewLine(false);
        customPathEditor.onTextChange = [this]()
        { loadButton.setEnabled(customPathEditor.getText().isNotEmpty()); };

        // Set up the Load button
        addAndMakeVisible(loadButton);
        loadButton.setButtonText("Load");
        loadButton.setEnabled(false); // Initially disabled
        loadButton.onClick = [this, onLoadCallback]()
        {
            onLoadCallback(customPathEditor.getText());
        };

        // Set up the Cancel button
        addAndMakeVisible(cancelButton);
        cancelButton.setButtonText("Cancel");
        cancelButton.onClick = [this, onCancelCallback]()
        {
            onCancelCallback();
        };

        setSize(400, 150); // Set a fixed size for the component
    }

    void visibilityChanged() override
    {
        if (isVisible())
        {
            MessageManager::callAsync([this] { customPathEditor.grabKeyboardFocus(); });
        }
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);
        customPathEditor.setBounds(area.removeFromTop(40));
        // button area
        auto buttonArea = area.removeFromBottom(40);
        loadButton.setBounds(buttonArea.removeFromLeft(buttonArea.getWidth() / 2));
        cancelButton.setBounds(buttonArea);
    }

    void paint(Graphics& g) override
    {
        g.fillAll(getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

private:
    TextEditor customPathEditor;
    TextButton loadButton, cancelButton;
};


class CustomPathDialog : public DialogWindow
{
public:
    CustomPathDialog(std::function<void(const String&)> onLoadCallback,
                     std::function<void()> onCancelCallback)
        : DialogWindow("Enter Custom Path", Colours::lightgrey, true),
          m_onLoadCallback(onLoadCallback), m_onCancelCallback(onCancelCallback)
    {
        // Create the content component
        auto* content = new CustomPathComponent(
            [this](const String& path) { loadButtonPressed(path); },
            [this]() { cancelButtonPressed(); }
        );

        // Add custom content
        setContentOwned(content, true);

        // Other dialog window options
        setUsingNativeTitleBar(false);
        setResizable(false, false);

        // Set size of the content component and center
        centreWithSize(400, 150);
        setVisible(true);

        // Ensure user cannot click off this window
        DialogWindow::enterModalState(true, nullptr, true);
    }

    void loadButtonPressed(const String& path)
    {
        if (m_onLoadCallback)
        {
            m_onLoadCallback(path);

            closeWindow();
        }
    }

    void cancelButtonPressed()
    {
        closeButtonPressed();
    }

    void closeButtonPressed() override
    {
        if (m_onCancelCallback)
        {
            m_onCancelCallback();
        }

        closeWindow();
    }

    void closeWindow()
    {
        setVisible(false);
        delete this;
    }

private:
    std::function<void(const String&)> m_onLoadCallback;
    std::function<void()> m_onCancelCallback;
};
