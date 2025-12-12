/*
 * @file StatusAreaWidget.h
 * @brief TODO
 * @author cwitkowitz
 */

#pragma once

#include "juce_gui_basics/juce_gui_basics.h"

#include "gui/StatusComponent.h"
#include "../utils.h"

using namespace juce;

class StatusAreaWidget : public Component
{
public:
    StatusAreaWidget()
    {
        // TODO
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
        // TODO
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
