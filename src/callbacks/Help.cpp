#include "../MainComponent.h"

void MainComponent::showAboutDialog()
{
    auto aboutComponent = std::make_unique<AboutWindow>();

    DialogWindow::LaunchOptions dialog;
    dialog.content.setOwned(aboutComponent.release());
    dialog.dialogTitle = "About " + String(APP_NAME);
    dialog.dialogBackgroundColour = Colours::grey;
    dialog.escapeKeyTriggersCloseButton = true;
    dialog.useNativeTitleBar = true;
    dialog.resizable = false;

    dialog.launchAsync();
}

// void MainComponent::showWelcomeDialog() {}
