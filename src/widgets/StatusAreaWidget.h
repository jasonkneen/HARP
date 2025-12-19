/*
 * @file StatusAreaWidget.h
 * @brief TODO
 * @author cwitkowitz
 */

#pragma once

#include "juce_gui_basics/juce_gui_basics.h"

#include "../utils.h"
#include "gui/StatusComponent.h"

using namespace juce;

class StatusAreaWidget : public Component
{
   public:
    StatusAreaWidget()
    {
        addAndMakeVisible(instructionBox);
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
        // Row 8: Instructions Area and Status Area
        juce::FlexBox row8;
        row8.flexDirection = juce::FlexBox::Direction::row;
        row8.items.add(juce::FlexItem(*instructionBox).withFlex(1).withMargin(margin));
        row8.items.add(juce::FlexItem(*statusBox).withFlex(1).withMargin(margin));
        // TODO - fix maximum height?
        mainPanel.items.add(juce::FlexItem(row8).withFlex(0.4f));
    }

    void setStatus(const ModelStatus& status)
    {
        String statusName = std::string(magic_enum::enum_name(status)).c_str();
        statusBox->setStatusMessage("ModelStatus::" + statusName);
    }

    void setStatus(const String& message) { statusBox->setStatusMessage(message); }

    void clearStatus() { statusBox->clearStatusMessage(); }

    void setInstructions(const String& message) { instructionBox->setStatusMessage(message); }

    void clearInstructions() { instructionBox->clearStatusMessage(); }

   private:
    SharedResourcePointer<InstructionBox> instructionBox;
    SharedResourcePointer<StatusBox> statusBox;
}
