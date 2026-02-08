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
        const int contentWidth = minimumContentWidth + comboChromeWidth;
        return jmax(minComboWidth, jmax(labelWidth + defaultPadding, contentWidth));
    }

    ComboBox& getComboBox() { return comboBox; }
    void setMinimumContentWidth(int width) { minimumContentWidth = jmax(0, width); }

private:
    static constexpr int minComboWidth = 80;
    static constexpr int comboChromeWidth = 54;

    Label label;
    ComboBox comboBox;
    int minimumContentWidth = 0;
};
