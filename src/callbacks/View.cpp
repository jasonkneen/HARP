#include "../MainComponent.h"

void MainComponent::viewStatusAreaCallback()
{
    // Toggle status Area visibility state
    showStatusArea = ! showStatusArea;

    // Find top-level window for resizing
    if (auto* window = findParentComponentOfClass<juce::DocumentWindow>())
    {
        // Determine which display contains HARP
        auto* currentDisplay =
            juce::Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds());

        int currentDisplayHeight;

        if (currentDisplay != nullptr)
        {
            if (window->isFullScreen())
            {
                currentDisplayHeight = currentDisplay->totalArea.getHeight();
            }
            else
            {
                currentDisplayHeight = currentDisplay->userArea.getHeight();
            }
        }

        // Get current bounds of top-level window
        Rectangle<int> windowBounds = window->getBounds();

        if (showStatusArea)
        {
            // Scale bounds to extend window by height of status area
            windowBounds.setHeight(jmin(currentDisplayHeight, windowBounds.getHeight() + 100));
        }
        else
        {
            if (! window->isFullScreen())
            {
                // Scale bounds to reduce window to main height
                windowBounds.setHeight(windowBounds.getHeight() - 100);
            }
        }

        // Set extended or reduced bounds
        window->setBounds(windowBounds);
    }

    // Add view preference to persistent settings
    AppSettings::setValue("showStatusArea", showStatusArea ? "1" : "0");
    AppSettings::saveIfNeeded();

    // Send status message to add check to file menu
    commandManager.commandStatusChanged();

    resized();
}

void MainComponent::viewMediaClipboardCallback()
{
    // Toggle media clipboard visibility state
    showMediaClipboard = ! showMediaClipboard;

    // Find top-level window for resizing
    if (auto* window = findParentComponentOfClass<juce::DocumentWindow>())
    {
        // Determine which display contains HARP
        auto* currentDisplay =
            juce::Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds());

        int currentDisplayWidth;

        if (currentDisplay != nullptr)
        {
            if (window->isFullScreen())
            {
                currentDisplayWidth = currentDisplay->totalArea.getWidth();
            }
            else
            {
                currentDisplayWidth = currentDisplay->userArea.getWidth();
            }
        }

        //int totalDesktopWidth = juce::Desktop::getInstance().getDisplays().getDisplayForRect(getBounds())->totalArea.getWidth();

        // Get current bounds of top-level window
        Rectangle<int> windowBounds = window->getBounds();

        if (showMediaClipboard)
        {
            // Scale bounds to extend window by 40% of main width
            windowBounds.setWidth(
                jmin(currentDisplayWidth, static_cast<int>(1.4 * windowBounds.getWidth())));
        }
        else
        {
            if (! window->isFullScreen())
            {
                // Scale bounds to reduce window to main width
                windowBounds.setWidth(static_cast<int>(windowBounds.getWidth() / 1.4));
            }
        }

        // Set extended or reduced bounds
        window->setBounds(windowBounds);
    }

    // Add view preference to persistent settings
    AppSettings::setValue("showMediaClipboard", showMediaClipboard ? "1" : "0");
    AppSettings::saveIfNeeded();

    // Send status message to add check to file menu
    commandManager.commandStatusChanged();

    resized();
}
