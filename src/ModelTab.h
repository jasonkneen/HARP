/*
 * @file ModelTab.h
 * @brief TODO
 * @author cwitkowitz
 */

#pragma once

#include "juce_gui_basics/juce_gui_basics.h"

#include "widgets/ModelSelectionWidget.h"
#include "widgets/ModelDisplayWidget.h"
#include "../utils.h"

using namespace juce;

class ModelTab : public Component
{
public:
    ModelTab()
    {
        // TODO
    }

    ~ModelTab()
    {
        // TODO
    }

    void paint(Graphics& g)
    {
        // TODO
    }

    void resized() override
    {
        // TODO
    }

private:

    ModelSelectionWidget modelSelectionWidget;
    ModelDisplayWidget modelDisplayWidget;
}
