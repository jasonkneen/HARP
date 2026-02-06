/**
 * @file ComboBoxWithLabel.h
 * @brief Custom dropdown component with label.
 * @author xribene
 */

#pragma once

#include "ControlComponent.h"

using namespace juce;

class ComboBoxWithLabel : public ControlComponent
{
public:
    ComboBoxWithLabel(const String& labelText = {})
    {
        label.setText(labelText, dontSendNotification);
        label.setJustificationType(Justification::centred);

        addAndMakeVisible(label);
        addAndMakeVisible(comboBox);
    }

    void resized() override
    {
        auto comboBoxArea = getLocalBounds();
        auto labelArea = comboBoxArea.removeFromTop(20);

        label.setBounds(labelArea);
        comboBox.setBounds(comboBoxArea);
    }

    int getMinimumRequiredWidth() const override
    {
        const int labelWidth = getLabelWidth(label);
        return jmax(minComboWidth, labelWidth + defaultPadding);
    }

    ComboBox& getComboBox() { return comboBox; }

private:
    static constexpr int minComboWidth = 80;

    Label label;
    ComboBox comboBox;
};
