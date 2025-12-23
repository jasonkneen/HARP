#include "../MainComponent.h"

void MainComponent::undoCallback()
{
    // DBG_AND_LOG("Undoing last edit");

    // // check if the audio file is loaded
    // if (! mediaDisplay->isFileLoaded())
    // {
    //     // TODO - gray out undo option in this case?
    //     // Fail with beep, we should just ignore this if it doesn't make sense
    //     DBG_AND_LOG("No file loaded to perform operation on");
    //     juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
    //     return;
    // }

    /*if (isProcessing)
    {
        DBG_AND_LOG("Can't undo while processing occurring!");
        juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
        return;
    }*/

    // Iterate over all inputMediaDisplays and call the iteratePreviousTempFile()
    //auto& inputMediaDisplays = inputTrackAreaWidget.getMediaDisplays();

    /*for (auto& inputMediaDisplay : inputMediaDisplays)
        {
            if (! inputMediaDisplay->iteratePreviousTempFile())
            {
                DBG_AND_LOG("Nothing to undo!");
                // juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
            }
            else
            {
                saveEnabled = true;
                DBG_AND_LOG("Undo callback completed successfully");
            }
        }*/
}

void MainComponent::redoCallback()
{
    // DBG_AND_LOG("Redoing last edit");

    // // check if the audio file is loaded
    // if (! mediaDisplay->isFileLoaded())
    // {
    //     // TODO - gray out undo option in this case?
    //     // Fail with beep, we should just ignore this if it doesn't make sense
    //     DBG_AND_LOG("No file loaded to perform operation on");
    //     juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
    //     return;
    // }

    /*if (isProcessing)
    {
        DBG_AND_LOG("Can't redo while processing occurring!");
        juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
        return;
    }*/

    // Iterate over all inputMediaDisplays and call the iterateNextTempFile()
    //auto& inputMediaDisplays = inputTrackAreaWidget.getMediaDisplays();

    /*for (auto& inputMediaDisplay : inputMediaDisplays)
        {
            if (! inputMediaDisplay->iterateNextTempFile())
            {
                DBG_AND_LOG("Nothing to redo!");
                // juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
            }
            else
            {
                saveEnabled = true;
                DBG_AND_LOG("Redo callback completed successfully");
            }
        }*/
}
