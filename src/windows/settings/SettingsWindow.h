#pragma once

#include <JuceHeader.h>

//#include "../../WebModel.h"
#include "AudioSettingsTab.h"
#include "GeneralSettingsTab.h"
#include "LoginTab.h"

class SettingsWindow : public juce::Component
{
public:
    //SettingsWindow(WebModel* m);
    SettingsWindow();
    ~SettingsWindow() override = default;

    void resized() override;

private:
    juce::TabbedComponent tabComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsWindow)
};
