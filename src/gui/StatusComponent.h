/**
 * @file StatusComponent.h
 * @brief Defines shared resources and components for instructions and status.
 * @author xribene, cwitkowitz
 */

#pragma once

#include "juce_gui_basics/juce_gui_basics.h"

using namespace juce;

struct SharedMessage : public ChangeBroadcaster
{
    void setMessage(const String& m)
    {
        message = m;
        sendChangeMessage();
    }

    void clearMessage()
    {
        message.clear();
        sendChangeMessage();
    }

    String message;
};

struct StatusMessage : SharedMessage
{
};
struct InstructionsMessage : SharedMessage
{
};

template <typename MessageType>
class MessageBox : public Component, ChangeListener
{
public:
    MessageBox(float fontSize = 15.0f, Justification justification = Justification::centred)
    {
        messageLabel.setFont(fontSize);
        messageLabel.setColour(Label::textColourId, Colour(0xE0, 0xE0, 0xE0));

        messageLabel.setJustificationType(justification);
        addAndMakeVisible(messageLabel);

        sharedMessage->addChangeListener(this);
    }

    ~MessageBox() override { sharedMessage->removeChangeListener(this); }

    void paint(Graphics& g)
    {
        g.setColour(Colour(0x33, 0x33, 0x33));
        g.fillAll();

        g.setColour(Colour(0x44, 0x44, 0x44));
        g.drawRect(getLocalBounds(), 1);
    }

    void resized() { messageLabel.setBounds(getLocalBounds()); }

    void changeListenerCallback(ChangeBroadcaster* /*source*/)
    {
        messageLabel.setText(sharedMessage->message, juce::dontSendNotification);
    }

private:
    SharedResourcePointer<MessageType> sharedMessage;
    Label messageLabel;
};

using StatusBox = MessageBox<StatusMessage>;
using InstructionsBox = MessageBox<InstructionsMessage>;

/*void setStatus(const ModelStatus& status)
{
    String statusName = std::string(magic_enum::enum_name(status)).c_str();
    statusBox->setStatusMessage("ModelStatus::" + statusName);
}*/
