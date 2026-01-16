#include <juce_gui_basics/juce_gui_basics.h>

#include "../WebModel.h"
#include "../gui/SliderWithLabel.h"
#include "../gui/TitledTextBox.h"
#include "../gui/ComboBoxWithLabel.h"
#include "../gui/StatusComponent.h"
#include "../gui/HoverHandler.h"

#include "../utils/Controls.h"

class ControlAreaWidget : public juce::Component,
                          public Button::Listener,
                          public Slider::Listener,
                          public ComboBox::Listener,
                          public TextEditor::Listener
{
public:
    ControlAreaWidget() {}

    void buttonClicked(Button* button) override
    {
        auto id = juce::Uuid(button->getName().toStdString());

        // ComponentInfoList& controlsInfo = mModel->getControlsInfo();
        auto componentInfo = mModel->findComponentInfoByUuid(id);
        if (componentInfo == nullptr)
        {
            DBG_AND_LOG("buttonClicked: ctrl not found");
            return;
        }
        if (auto toggleInfo = dynamic_cast<ToggleInfo*>(componentInfo.get()))
        {
            toggleInfo->value = button->getToggleState();
        }
        else
        {
            DBG_AND_LOG("buttonClicked: ctrl is not a toggle");
        }
    }

    void comboBoxChanged(ComboBox* comboBox) override
    {
        auto id = juce::Uuid(comboBox->getName().toStdString());

        // ComponentInfoList& controlsInfo = mModel->getControlsInfo();
        auto componentInfo = mModel->findComponentInfoByUuid(id);
        if (componentInfo == nullptr)
        {
            DBG_AND_LOG("comboBoxChanged: ctrl not found");
            return;
        }
        if (auto comboBoxInfo = dynamic_cast<ComboBoxInfo*>(componentInfo.get()))
        {
            comboBoxInfo->value = comboBox->getText().toStdString();
        }
        else
        {
            DBG_AND_LOG("comboBoxChanged: ctrl is not a combobox");
        }
    }

    void textEditorTextChanged(TextEditor& textEditor) override
    {
        auto id = juce::Uuid(textEditor.getName().toStdString());

        // ComponentInfoList& controlsInfo = mModel->getControlsInfo();
        auto componentInfo = mModel->findComponentInfoByUuid(id);
        if (componentInfo == nullptr)
        {
            DBG_AND_LOG("textEditorTextChanged: ctrl not found");
            return;
        }
        if (auto textBoxInfo = dynamic_cast<TextBoxInfo*>(componentInfo.get()))
        {
            textBoxInfo->value = textEditor.getText().toStdString();
        }
        else
        {
            DBG_AND_LOG("textEditorTextChanged: ctrl is not a text box");
        }
    }

    void sliderValueChanged(Slider* slider) override { ignoreUnused(slider); }

    void sliderDragEnded(Slider* slider) override
    {
        auto id = juce::Uuid(slider->getName().toStdString());

        // ComponentInfoList& controlsInfo = mModel->getControlsInfo();
        auto componentInfo = mModel->findComponentInfoByUuid(id);
        // Check if pair is nullptr
        if (componentInfo == nullptr)
        {
            DBG_AND_LOG("sliderDragEnded: ctrl not found");
            return;
        }
        if (auto sliderInfo = dynamic_cast<SliderInfo*>(componentInfo.get()))
        {
            sliderInfo->value = slider->getValue();
        }
        else if (auto numberBoxInfo = dynamic_cast<NumberBoxInfo*>(componentInfo.get()))
        {
            numberBoxInfo->value = slider->getValue();
        }
        else
        {
            DBG_AND_LOG("sliderDragEnded: ctrl is not a slider");
        }
    }

private:

    // Vectors of unique pointers to widgets
    std::vector<std::unique_ptr<SliderWithLabel>> sliders;
    std::vector<std::unique_ptr<juce::ToggleButton>> toggles;
    std::vector<std::unique_ptr<ComboBoxWithLabel>> optionCtrls;
    std::vector<std::unique_ptr<TitledTextBox>> textCtrls;

    std::vector<std::unique_ptr<HoverHandler>> handlers;
};
