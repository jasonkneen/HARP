/**
 * @file ToggleWithLabel.h
 * @brief Custom toggle button component with label.
 * @author gemini
 */

#pragma once

#include "ControlComponent.h"

using namespace juce;

class ToggleWithLabel : public ControlComponent
{
public:
    ToggleWithLabel(const String& labelText = {})
    {
        label.setText(labelText, dontSendNotification);
        label.setJustificationType(Justification::centred);

        toggleButton.setButtonText(labelText);

        addAndMakeVisible(label);
        addAndMakeVisible(toggleButton);
    }

    void resized() override
    {
        auto toggleArea = getLocalBounds();
        auto labelArea = toggleArea.removeFromTop(20);

        label.setBounds(labelArea);
        toggleButton.setBounds(toggleArea);
    }

    int getMinimumRequiredWidth() const override
    {
        const int labelWidth = getLabelWidth(label);
        return jmax(minToggleWidth, labelWidth + defaultPadding);
    }

    ToggleButton& getToggleButton() { return toggleButton; }

    void setToggleState(bool state, NotificationType notification)
    {
        toggleButton.setToggleState(state, notification);
    }

    bool getToggleState() const { return toggleButton.getToggleState(); }

private:
    static constexpr int minToggleWidth = 60;

    Label label;
    ToggleButton toggleButton;
};
