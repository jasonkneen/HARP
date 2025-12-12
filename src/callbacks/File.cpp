#include "../MainComponent.h"

/**
 * Entry point for importing new files into HARP.
 */
void MainComponent::importNewFile(File mediaFile, bool fromDAW)
{
    mediaClipboardWidget.addTrackFromFilePath(URL(mediaFile), fromDAW);

    if (! showMediaClipboard)
    {
        viewMediaClipboardCallback();
    }
}

void MainComponent::showSettingsDialog()
{
    juce::DialogWindow::LaunchOptions options;
    options.dialogTitle = "Settings";
    options.content.setOwned(new SettingsBox(model.get()));
    options.useNativeTitleBar = true;
    options.resizable = true;
    options.escapeKeyTriggersCloseButton = true;
    options.dialogBackgroundColour = juce::Colours::darkgrey;
    options.launchAsync();
}
