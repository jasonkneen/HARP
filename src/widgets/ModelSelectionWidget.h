/*
 * @file ModelSelectionWidget.h
 * @brief TODO
 * @author cwitkowitz
 */

#pragma once

#include "juce_gui_basics/juce_gui_basics.h"

#include "../utils.h"

using namespace juce;

// this is the callback for the add new path popup alert
class CustomPathAlertCallback : public juce::ModalComponentManager::Callback
{
   public:
    CustomPathAlertCallback(std::function<void(int)> const& callback) : userCallback(callback) {}

    void modalStateFinished(int result) override
    {
        if (userCallback != nullptr)
        {
            userCallback(result);
        }
    }

   private:
    std::function<void(int)> userCallback;
};

class ModelSelectionWidget : public Component
{
   public:
    ModelSelectionWidget()
    {
        // TODO
    }

    ~ModelSelectionWidget()
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

    /*
    // Adds a path to the model dropdown if it's not already present
    void addCustomPathToDropdown(const std::string& path, bool wasSleeping = false)
    {
        juce::String displayStr(path);
        if (wasSleeping)
            displayStr += " (sleeping)";

        bool alreadyExists = false;
        for (int i = 0; i < modelPathComboBox.getNumItems(); ++i)
        {
            if (modelPathComboBox.getItemText(i).startsWithIgnoreCase(path))
            {
                alreadyExists = true;
                break;
            }
        }

        if (! alreadyExists)
        {
            int newID = modelPathComboBox.getNumItems() + 1;
            modelPathComboBox.addItem(displayStr, newID);
        }

        modelPathComboBox.setText(displayStr, juce::dontSendNotification);
    }
    */

   private:
    ComboBox modelPathComboBox;
    std::string customPath;
    // Two usefull variables to keep track of the selected item in the modelPathComboBox
    // and the item index of the last loaded model
    // These are used to restore the selected item in the modelPathComboBox
    // after a failed attempt to load a new model
    int lastSelectedItemIndex = -1;
    int lastLoadedModelItemIndex = -1;
    HoverHandler modelPathComboBoxHandler{modelPathComboBox};

    MultiButton loadModelButton;
    MultiButton::Mode loadButtonInfo;
}
