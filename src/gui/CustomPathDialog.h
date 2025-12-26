/**
 * @file CustomPathDialog.cpp
 * @brief Custom dialog for entering a custom path in the HARPPlugin
 * @author xribene
 */

#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../utils/Interface.h"

using namespace juce;

class CustomPathComponent : public Component
{
public:
    CustomPathComponent(std::function<void(String)> onLoad, std::function<void()> onCancel)
        : onLoadCallback(std::move(onLoad)), onCancelCallback(std::move(onCancel))
    {
        pathEditor.setMultiLine(false);
        pathEditor.setReturnKeyStartsNewLine(false);
        pathEditor.onTextChange = [this] { loadButton.setEnabled(! pathEditor.isEmpty()); };
        pathEditor.onReturnKey = [this]
        {
            if (loadButton.isEnabled())
            {
                loadButton.triggerClick();
            }
        };
        addAndMakeVisible(pathEditor);

        //loadButton.setButtonText("Load");
        loadButton.setEnabled(false);
        /*loadButton.onClick = [this]
        {
            if (onLoadCallback)
            {
                onLoadCallback(pathEditor.getText());
            }
        };*/
        loadButton.onClick = [this]
        {
            wasLoadPressed = true;

            if (onLoadCallback)
            {
                onLoadCallback(pathEditor.getText());
            }

            closeDialog();
        };
        addAndMakeVisible(loadButton);

        //cancelButton.setButtonText("Cancel");
        /*cancelButton.onClick = [this]
        {
            if (onCancelCallback)
            {
                onCancelCallback();
            }
        };*/
        cancelButton.onClick = [this] { closeDialog(); };
        addAndMakeVisible(cancelButton);

        setSize(400, 80);
    }

    ~CustomPathComponent() override
    {
        if (! wasLoadPressed && onCancelCallback)
        {
            // Treat as cancel if closed without load
            onCancelCallback();
        }
    }

    void visibilityChanged() override
    {
        if (isVisible())
        {
            MessageManager::callAsync([this] { pathEditor.grabKeyboardFocus(); });
        }
    }

    void resized() override
    {
        /*Rectangle<int> totalArea = getLocalBounds().reduced(10);

        pathEditor.setBounds(totalArea.removeFromTop(40));

        Rectangle<int> buttonsArea = totalArea.removeFromBottom(40);

        loadButton.setBounds(buttonsArea.removeFromLeft(buttonsArea.getWidth() / 2));
        cancelButton.setBounds(buttonsArea);*/

        Rectangle<int> fullArea = getLocalBounds();

        FlexBox fullPopup;
        fullPopup.flexDirection = FlexBox::Direction::column;

        fullPopup.items.add(FlexItem(pathEditor).withHeight(30).withMargin(2));

        FlexBox buttonsArea;
        buttonsArea.flexDirection = FlexBox::Direction::row;

        buttonsArea.items.add(FlexItem(loadButton).withFlex(1).withMargin(10));
        buttonsArea.items.add(FlexItem().withFlex(0.25));
        buttonsArea.items.add(FlexItem(cancelButton).withFlex(1).withMargin(10));

        fullPopup.items.add(FlexItem(buttonsArea).withFlex(1));

        fullPopup.performLayout(fullArea);
    }

    void paint(Graphics& g) override
    {
        g.fillAll(getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void setTextFieldValue(const String& text)
    {
        pathEditor.setText(text, dontSendNotification);
        pathEditor.selectAll();
    }

private:
    void closeDialog()
    {
        if (auto* popup = findParentComponentOfClass<DialogWindow>())
        {
            popup->exitModalState(0);
        }
    }

    TextEditor pathEditor;
    TextButton loadButton { "Load" };
    TextButton cancelButton { "Cancel" };

    bool wasLoadPressed = false;

    std::function<void(String)> onLoadCallback;
    std::function<void()> onCancelCallback;
};

/*class CustomPathDialog : public DialogWindow
{
public:
    CustomPathDialog(std::function<void(const String&)> onLoadCallback,
                     std::function<void()> onCancelCallback)
        : DialogWindow("Enter Custom Path", Colours::lightgrey, true),
          m_onLoadCallback(onLoadCallback),
          m_onCancelCallback(onCancelCallback)
    {
        // Create the content component
        //auto* content =
        // new CustomPathComponent([this](const String& path) { loadButtonPressed(path); },
        //[this]() { cancelButtonPressed(); });
        pathComponent =
            new CustomPathComponent([this](const String& path) { loadButtonPressed(path); },
                                    [this]() { cancelButtonPressed(); });

        setContentOwned(pathComponent, true);

        // Add custom content
        //setContentOwned(content, true);

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

    void cancelButtonPressed() { closeButtonPressed(); }

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

    void setTextFieldValue(const std::string& path)
    {
        if (pathComponent != nullptr)
            pathComponent->setTextFieldValue(path);
    }

private:
    std::function<void(const String&)> m_onLoadCallback;
    std::function<void()> m_onCancelCallback;
    CustomPathComponent* pathComponent = nullptr;
};*/
