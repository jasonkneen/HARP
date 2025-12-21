/*
 * @file StatusAreaWidget.h
 * @brief TODO
 * @author cwitkowitz
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../gui/StatusComponent.h"
//#include "../utils.h"

using namespace juce;

class StatusAreaWidget : public Component
{
public:
    StatusAreaWidget()
    {
        addAndMakeVisible(instructionsBox);
        addAndMakeVisible(statusBox);
    }

    ~StatusAreaWidget()
    {
        // TODO
    }

    void paint(Graphics& g)
    {
        // TODO
    }

    void resized() override
    {
        Rectangle<int> area = getLocalBounds();

        statusArea.flexDirection = FlexBox::Direction::row;

        statusArea.items.add(FlexItem(instructionsBox).withFlex(1).withMargin(marginSize));
        statusArea.items.add(FlexItem(statusBox).withFlex(1).withMargin(marginSize));

        statusArea.performLayout(area);
    }

private:
    const float marginSize = 2;

    // Flex for status area
    FlexBox statusArea;

    InstructionsBox instructionsBox;
    StatusBox statusBox;
};
