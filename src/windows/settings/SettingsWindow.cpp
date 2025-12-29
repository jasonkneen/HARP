#include "SettingsWindow.h"

SettingsWindow::SettingsWindow() : tabComponent(juce::TabbedButtonBar::TabsAtTop)
{
    tabComponent.addTab("General", juce::Colours::darkgrey, new GeneralSettingsTab(), true);
    /*
    tabComponent.addTab(
        "Hugging Face", juce::Colours::darkgrey, new LoginTab("huggingface"), true);
    tabComponent.addTab(
        "Stability AI", juce::Colours::darkgrey, new LoginTab("stability"), true);
    */
    // tabComponent.addTab("Audio", juce::Colours::darkgrey, new AudioSettingsTab(), true);
    addAndMakeVisible(tabComponent);
    setSize(400, 300);
}

/*SettingsWindow::SettingsWindow(WebModel* m) : tabComponent(juce::TabbedButtonBar::TabsAtTop)
{
    tabComponent.addTab("General", juce::Colours::darkgrey, new GeneralSettingsTab(), true);
    tabComponent.addTab(
        "Hugging Face", juce::Colours::darkgrey, new LoginTab("huggingface", m), true);
    tabComponent.addTab(
        "Stability AI", juce::Colours::darkgrey, new LoginTab("stability", m), true);
    // tabComponent.addTab("Audio", juce::Colours::darkgrey, new AudioSettingsTab(), true);
    addAndMakeVisible(tabComponent);
    setSize(400, 300);
}*/

void SettingsWindow::resized() { tabComponent.setBounds(getLocalBounds()); }
