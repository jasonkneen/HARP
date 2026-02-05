/**
 * @file SliderWithLabel.h
 * @brief Custom slider component with label.
 * @author xribene
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;

class SliderWithLabel : public Component
{
public:
    SliderWithLabel(const String& labelText, Slider::SliderStyle style)
        : slider(style, Slider::TextBoxBelow)
    {
        label.setText(labelText, dontSendNotification);
        label.setJustificationType(Justification::centred);

        slider.setColour(Slider::thumbColourId, Colours::coral);

        addAndMakeVisible(label);
        addAndMakeVisible(slider);
    }

    void resized() override
    {
        auto sliderArea = getLocalBounds();
        auto labelArea = sliderArea.removeFromTop(sliderArea.getHeight() / 6);

        label.setBounds(labelArea);
        slider.setBounds(sliderArea);
    }

    int getMinimumRequiredWidth() const
    {
        Font font = label.getFont();

        const int labelWidth = font.getStringWidth(label.getText());

        int minRequiredWidth = jmax(minSliderBody, labelWidth + padding);

        return minRequiredWidth;
    }

    Slider& getSlider() { return slider; }

private:
    const int padding = 20;
    const int minSliderBody = 60;

    Label label;
    Slider slider;
};
