/**
 * @file ComboBoxWithLabel.h
 * @brief Custom dropdown component with label.
 * @author xribene
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;

class ComboBoxWithLabel : public Component
{
public:
    ComboBoxWithLabel(const juce::String& labelText = {})
    {
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);

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

    ComboBox& getComboBox() { return comboBox; }

private:
    Label label;
    ComboBox comboBox;
};
