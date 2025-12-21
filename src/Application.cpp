#include "MainComponent.h"

/**
 * Fills commands array with commands this component/target supports.
 */
void MainComponent::getAllCommands(Array<CommandID>& commands)
{
    const CommandID fileIDs[] = {
        CommandIDs::open, CommandIDs::save, CommandIDs::saveAs, CommandIDs::settings
    };

    commands.addArray(fileIDs, numElementsInArray(fileIDs));

    const CommandID editIDs[] = { CommandIDs::undo, CommandIDs::redo };

    commands.addArray(editIDs, numElementsInArray(editIDs));

    const CommandID viewIDs[] = { CommandIDs::viewStatusArea, CommandIDs::viewMediaClipboard };

    commands.addArray(viewIDs, numElementsInArray(viewIDs));

    const CommandID helpIDs[] = { CommandIDs::about, CommandIDs::welcome };

    commands.addArray(helpIDs, numElementsInArray(helpIDs));
}

void MainComponent::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        /*
          Note that the third argument of setInfo() serves only as a tag for
          categorization and does not need to match the corresponding file menu option.
        */

        /* --File-- */
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

        /* --Edit-- */
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

        /* --View-- */
        case CommandIDs::viewStatusArea:
            result.setInfo("Status Area", "Toggles display of status area", "View", 0);
            result.setTicked(showStatusArea);

            break;

        case CommandIDs::viewMediaClipboard:
            result.setInfo("Media Clipboard", "Toggles display of media clipboard", "View", 0);
            result.setTicked(showMediaClipboard);

            break;

        /* --Help-- */
        case CommandIDs::welcome:
            result.setInfo(
                "Welcome Page", "Provides helpful information for first-time users", "Help", 0);

            break;

        case CommandIDs::about:
            result.setInfo(
                "About HARP", "Provides helpful information and links for application", "Help", 0);

            break;
    }
}

bool MainComponent::perform(const InvocationInfo& info)
{
    switch (info.commandID)
    {
        /* --File-- */
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

        /* --Edit-- */
        case CommandIDs::undo:
            DBG("MainComponent::perform: \"undo\" command invoked.");
            undoCallback();

            break;

        case CommandIDs::redo:
            DBG("MainComponent::perform: \"redo\" command invoked.");
            redoCallback();

            break;

        /* --View-- */
        case CommandIDs::viewMediaClipboard:
            DBG("MainComponent::perform: \"viewMediaClipboard\" command invoked.");
            viewMediaClipboardCallback();

            break;

                    case CommandIDs::viewStatusArea:
            DBG("MainComponent::perform: \"viewStatusArea\" command invoked.");
            viewStatusAreaCallback();

            break;

        /* --Help-- */
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
