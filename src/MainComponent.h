#pragma once

// #include <juce_audio_basics/juce_audio_basics.h>
// #include <juce_audio_devices/juce_audio_devices.h>
// #include <juce_audio_formats/juce_audio_formats.h>
// #include <juce_audio_processors/juce_audio_processors.h>
// #include <juce_audio_utils/juce_audio_utils.h>
// #include <juce_core/juce_core.h>
// #include <juce_data_structures/juce_data_structures.h>
// #include <juce_dsp/juce_dsp.h>
// #include <juce_events/juce_events.h>
// #include <juce_gui_basics/juce_gui_basics.h>
// #include <juce_gui_extra/juce_gui_extra.h>

#include "windows/AboutWindow.h"
#include "windows/settings/SettingsWindow.h"

//#include "ModelTab.h"
#include "widgets/MediaClipboardWidget.h"
#include "widgets/StatusAreaWidget.h"

//#include "ThreadPoolJob.h"
//#include "WebModel.h"

// #include "gui/CustomPathDialog.h"
// #include "gui/HoverHandler.h"
// #include "gui/ModelAuthorLabel.h"
// #include "gui/MultiButton.h"
// #include "gui/StatusComponent.h"
// #include "gui/TitledTextBox.h"

// #include "client/Client.h"

//#include "external/magic_enum.hpp"
// #include "media/AudioDisplayComponent.h"
// #include "media/MediaDisplayComponent.h"
// #include "media/MidiDisplayComponent.h"

#include "utils/Logging.h"
#include "utils/Settings.h"
#include "utils/Interface.h"

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


class MainComponent : public Component,
                      public MenuBarModel,
                      public ApplicationCommandTarget

{
   public:
    MainComponent();
    ~MainComponent() override;

    enum CommandIDs
    {
        // invalid = 0x0000,

        // File
        open = 0x0001,
        save = 0x0002,
        saveAs = 0x0003,
        settings = 0x0004,

        // Edit
        undo = 0x1000,
        redo = 0x1001,

        // View
        viewMediaClipboard = 0x2000,

        // Help
        welcome = 0x3000,
        about = 0x3001
    };

    /* File Menu */

    StringArray getMenuBarNames() override;

    std::unique_ptr<PopupMenu> getMacExtraMenu();

    PopupMenu getMenuForIndex([[maybe_unused]] int menuIndex, const String& menuName) override;

    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

    void initializeMenuBar();
    void deinitializeMenuBar();

    /* Application */

    ApplicationCommandTarget* getNextCommandTarget() override { return nullptr; }

    void getAllCommands(Array<CommandID>& commands) override;

    void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override;

    bool perform(const InvocationInfo& info) override;

    /* Callbacks */

    // File
    void importNewFile(File mediaFile, bool fromDAW = false);
    void showSettingsDialog();

    // Edit
    void undoCallback();
    void redoCallback();

    // View
    void viewMediaClipboardCallback();

    // Help
    void showAboutDialog();
    // void showWelcomeDialog();

    // Authenticate
    void tryLoadSavedToken();

    // Load
    void openCustomPathDialog(const std::string& prefillPath = "");
    void loadModelCallback();

    // Process
    void processCallback();
    void cancelCallback();

    // Miscellaneous
    void focusCallback();

    /* Component */

    void paint(Graphics& g) override;
    void resized() override;

   private:
    /* File Menu */

    std::unique_ptr<MenuBarComponent> menuBar;
    // std::unique_ptr<PopupMenu> macExtraMenu;

    /* Application */

    ApplicationCommandManager commandManager;

    /* Interface */

    //ModelTab mainModelTab;
    Component mainModelTab;
    StatusAreaWidget statusAreaWidget;
    MediaClipboardWidget mediaClipboardWidget;

    bool showMediaClipboard;

    /* Authentication */

    String savedStabilityToken;

    /* Loading */

    ChangeBroadcaster loadBroadcaster;

    ThreadPool threadPool{1};

    /* Processing */

    String currentProcessID;

    std::mutex processMutex;

    ChangeBroadcaster processBroadcaster;

    ThreadPool jobProcessorThread{10};

    // bool isProcessing = false;

    /* Miscellaneous */

    // std::shared_ptr<fontawesome::IconHelper> fontawesomeHelper;
    // std::shared_ptr<fontaudio::IconHelper> fontaudioHelper;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
