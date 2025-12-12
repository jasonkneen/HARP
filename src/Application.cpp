#include "MainComponent.h"

/**
 * Fills commands array with commands this component/target supports.
 */
void MainComponent::getAllCommands(Array<CommandID>& commands)
{
    const CommandID ids[] = { CommandIDs::open,     CommandIDs::save,
                              CommandIDs::saveAs,   CommandIDs::undo,
                              CommandIDs::redo,     CommandIDs::about,
                              CommandIDs::settings, CommandIDs::viewMediaClipboard,
                              CommandIDs::welcome };

    commands.addArray(ids, numElementsInArray(ids));
}

void MainComponent::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        /*
          Note that the third argument of setInfo() serves only as a tag for
          categorization and does not need to match the corresponding file menu option.
        */
        case CommandIDs::open:
            result.setInfo("Open", "Add a track to the media clipboard", "File", 0);
            result.addDefaultKeypress('o', ModifierKeys::commandModifier);

            break;
        case CommandIDs::save:
            result.setInfo("Save", "Saves currently selected media clipboard track", "File", 0);
            result.addDefaultKeypress('s', ModifierKeys::commandModifier);

            break;
        case CommandIDs::saveAs:
            result.setInfo("Save As", "Saves currently selected track with a new name", "File", 0);
            // TODO = is logical or (|) correct here?
            result.addDefaultKeypress('s',
                                      ModifierKeys::shiftModifier | ModifierKeys::commandModifier);

            break;
        case CommandIDs::settings:
            result.setInfo("Settings", "Open settings window", "File", 0);

            break;

        case CommandIDs::undo:
            result.setInfo("Undo", "Reverses most recent action", "Edit", 0);
            result.addDefaultKeypress('z', ModifierKeys::commandModifier);

            break;
        case CommandIDs::redo:
            result.setInfo("Redo", "Repeats next future action", "Edit", 0);
            // TODO = is logical or (|) correct here?
            result.addDefaultKeypress('z',
                                      ModifierKeys::shiftModifier | ModifierKeys::commandModifier);

            break;

        case CommandIDs::viewMediaClipboard:
            result.setInfo("Media Clipboard", "Toggles display of media clipboard", "View", 0);
            result.setTicked(showMediaClipboard);

            break;

        case CommandIDs::welcome:
            result.setInfo(
                "Welcome Page", "Provides helpful information for first-time users", "Welcome", 0);

            break;
        case CommandIDs::about:
            result.setInfo(
                "About HARP", "Provides helpful information and links for application", "About", 0);

            break;
    }
}

bool MainComponent::perform(const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::open:
            DBG("MainComponent::perform: \"open\" command invoked.");
            mediaClipboardWidget.addFileCallback();

            break;
        case CommandIDs::save:
            DBG("MainComponent::perform: \"save\" command invoked.");
            // TODO - saveCallback();

            break;
        case CommandIDs::saveAs:
            DBG("MainComponent::perform: \"saveAs\" command invoked.");
            mediaClipboardWidget.saveFileCallback();

            break;
        case CommandIDs::settings:
            DBG("MainComponent::perform: \"settings\" command invoked.");
            showSettingsDialog();

            break;

        case CommandIDs::undo:
            DBG("MainComponent::perform: \"undo\" command invoked.");
            undoCallback();

            break;
        case CommandIDs::redo:
            DBG("MainComponent::perform: \"redo\" command invoked.");
            redoCallback();

            break;

        case CommandIDs::viewMediaClipboard:
            DBG("MainComponent::perform: \"viewMediaClipboard\" command invoked.");
            viewMediaClipboardCallback();

            break;

        case CommandIDs::welcome:
            DBG("MainComponent::perform: \"welcome\" command invoked.");
            // TODO - showWelcomeDialog();

            break;
        case CommandIDs::about:
            DBG("MainComponent::perform: \"about\" command invoked.");
            showAboutDialog();

            break;

        default:
            return false;
    }

    return true;
}
