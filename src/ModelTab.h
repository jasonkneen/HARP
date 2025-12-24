/*
 * @file ModelTab.h
 * @brief TODO
 * @author cwitkowitz
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "widgets/ModelSelectionWidget.h"
//#include "widgets/ModelDisplayWidget.h"

//#include "../utils.h"

using namespace juce;

class ModelTab : public Component
{
public:
    ModelTab()
    {
        addAndMakeVisible(modelSelectionWidget);
        //addAndMakeVisible(modelDisplayWidget);
    }

    ~ModelTab()
    {
        // TODO
    }

    //void paint(Graphics& g) {}

    void resized() override
    {
        FlexBox tabArea;
        tabArea.flexDirection = FlexBox::Direction::column;

        tabArea.items.add(FlexItem(modelSelectionWidget).withHeight(30));

        tabArea.performLayout(getLocalBounds());
    }

private:
    const float marginSize = 2;

    ModelSelectionWidget modelSelectionWidget;
    //ModelDisplayWidget modelDisplayWidget;
};
