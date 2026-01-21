#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "ModelTab.h"

#include "client/Client.h"

#include "widgets/MediaClipboardWidget.h"
#include "widgets/StatusAreaWidget.h"

#include "windows/AboutWindow.h"
#include "windows/settings/SettingsWindow.h"

#include "utils/Interface.h"
#include "utils/Logging.h"
#include "utils/Settings.h"

using namespace juce;

/*// this only calls the callback ONCE
class TimedCallback : public Timer
{
   public:
    TimedCallback(std::function<void()> callback, int interval)
        : mCallback(callback), mInterval(interval)
    {
        startTimer(mInterval);
    }

    ~TimedCallback() override { stopTimer(); }

    void timerCallback() override
    {
        mCallback();
        stopTimer();
    }

   private:
    std::function<void()> mCallback;
    int mInterval;
};*/

/*inline std::unique_ptr<OutputStream> makeOutputStream(const URL& url)
{
    if (const auto doc = AndroidDocument::fromDocument(url))
        return doc.createOutputStream();

#if ! JUCE_IOS
    if (url.isLocalFile())
        return url.getLocalFile().createOutputStream();
#endif

    return url.createOutputStream();
}*/

class MainComponent : public Component, public MenuBarModel, public ApplicationCommandTarget

{
public:
    MainComponent();
    ~MainComponent() override;

    /* File Menu */

    StringArray getMenuBarNames() override;

    std::unique_ptr<PopupMenu> getMacExtraMenu();

    PopupMenu getMenuForIndex([[maybe_unused]] int menuIndex, const String& menuName) override;

    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

    /* Application */

    ApplicationCommandTarget* getNextCommandTarget() override { return nullptr; }

    void getAllCommands(Array<CommandID>& commands) override;

    void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override;

    bool perform(const InvocationInfo& info) override;

    /* Callbacks */

    // File
    void importNewFile(File mediaFile, bool fromDAW = false);
    void openSettingsWindow();

    // Edit
    //void undoCallback();
    //void redoCallback();

    // View
    void viewStatusAreaCallback();
    void viewMediaClipboardCallback();

    // Help
    void openAboutWindow();
    //void openWelcomeWindow();

    /* Component */

    void paint(Graphics& g) override;
    void resized() override;

private:
    /* File Menu */

    void initializeMenuBar();
    void deinitializeMenuBar();

    std::unique_ptr<MenuBarComponent> menuBar;
    std::unique_ptr<PopupMenu> macExtraMenu; // TODO - is this actually used?

    /* Application */

    ApplicationCommandManager commandManager;

    /* Callbacks */

    // Miscellaneous
    //void focusCallback();

    /* Interface */

    bool showStatusArea;
    bool showMediaClipboard;

    ModelTab mainModelTab;
    StatusAreaWidget statusAreaWidget;
    MediaClipboardWidget mediaClipboardWidget;

    SharedResourcePointer<SharedAPIKeys> sharedTokens;
    SharedResourcePointer<StatusMessage> statusMessage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
