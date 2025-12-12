#include "../MainComponent.h"

void MainComponent::undoCallback()
{
    // DBG("Undoing last edit");

    // // check if the audio file is loaded
    // if (! mediaDisplay->isFileLoaded())
    // {
    //     // TODO - gray out undo option in this case?
    //     // Fail with beep, we should just ignore this if it doesn't make sense
    //     DBG("No file loaded to perform operation on");
    //     juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
    //     return;
    // }

    /*if (isProcessing)
    {
        DBG("Can't undo while processing occurring!");
        juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
        return;
    }*/

    // Iterate over all inputMediaDisplays and call the iteratePreviousTempFile()
    //auto& inputMediaDisplays = inputTrackAreaWidget.getMediaDisplays();

    /*for (auto& inputMediaDisplay : inputMediaDisplays)
        {
            if (! inputMediaDisplay->iteratePreviousTempFile())
            {
                DBG("Nothing to undo!");
                // juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
            }
            else
            {
                saveEnabled = true;
                DBG("Undo callback completed successfully");
            }
        }*/
}

void MainComponent::redoCallback()
{
    // DBG("Redoing last edit");

    // // check if the audio file is loaded
    // if (! mediaDisplay->isFileLoaded())
    // {
    //     // TODO - gray out undo option in this case?
    //     // Fail with beep, we should just ignore this if it doesn't make sense
    //     DBG("No file loaded to perform operation on");
    //     juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
    //     return;
    // }

    /*if (isProcessing)
    {
        DBG("Can't redo while processing occurring!");
        juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
        return;
    }*/

    // Iterate over all inputMediaDisplays and call the iterateNextTempFile()
    //auto& inputMediaDisplays = inputTrackAreaWidget.getMediaDisplays();

    /*for (auto& inputMediaDisplay : inputMediaDisplays)
        {
            if (! inputMediaDisplay->iterateNextTempFile())
            {
                DBG("Nothing to redo!");
                // juce::LookAndFeel::getDefaultLookAndFeel().playAlertSound();
            }
            else
            {
                saveEnabled = true;
                DBG("Redo callback completed successfully");
            }
        }*/
}
