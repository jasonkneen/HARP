#include "MainComponent.h"

StringArray MainComponent::getMenuBarNames()
{
    StringArray menuBarNames;

    menuBarNames.add("File");
    //menuBarNames.add("Edit");
    menuBarNames.add("View");
    menuBarNames.add("Help");

    return menuBarNames;
}

/**
 * Place certain commands in the application menu ("HARP" tab) for MacOS.
 */
std::unique_ptr<PopupMenu> MainComponent::getMacExtraMenu()
{
    auto menu = std::make_unique<PopupMenu>();

    menu->addCommandItem(&commandManager, CommandIDs::about);
    menu->addCommandItem(&commandManager, CommandIDs::settings);

    return menu;
}

PopupMenu MainComponent::getMenuForIndex([[maybe_unused]] int menuIndex, const String& menuName)
{
    PopupMenu menu;

    if (menuName == "File")
    {
        menu.addCommandItem(&commandManager, CommandIDs::open);
        //menu.addCommandItem(&commandManager, CommandIDs::save);
        menu.addCommandItem(&commandManager, CommandIDs::saveAs);

        menu.addSeparator();

        menu.addCommandItem(&commandManager, CommandIDs::settings);
    }
    else if (menuName == "Edit")
    {
        //menu.addCommandItem(&commandManager, CommandIDs::undo);
        //menu.addCommandItem(&commandManager, CommandIDs::redo);
    }
    else if (menuName == "View")
    {
        menu.addCommandItem(&commandManager, CommandIDs::viewMediaClipboard);
    }
    else if (menuName == "Help")
    {
        menu.addCommandItem(&commandManager, CommandIDs::about);
        menu.addCommandItem(&commandManager, CommandIDs::welcome);
    }
    else
    {
        DBG("MainComponent::getMenuForIndex: Unknown menu name: " << menuName << ".");
    }

    return menu;
}

void MainComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    DBG("MainComponent::menuItemSelected: Selected ID " << menuItemID << " and index "
                                                        << topLevelMenuIndex << ".");
}

// TODO - move to Interface.cpp?
void MainComponent::initializeMenuBar()
{
    // init the menu bar
    menuBar.reset(new MenuBarComponent(this));
    addAndMakeVisible(menuBar.get());
    setApplicationCommandManagerToWatch(&commandManager);

    // Register commands
    commandManager.registerAllCommandsForTarget(this);
    commandManager.setFirstCommandTarget(this);

    // commandManager.setFirstCommandTarget(this);
    addKeyListener(commandManager.getKeyMappings());

#if JUCE_MAC
    // TODO - is this actually used?
    macExtraMenu = getMacExtraMenu();

    MenuBarModel::setMacMainMenu(this, macExtraMenu.get());
#endif

    menuBar->setVisible(true);
    menuItemsChanged();
}

// TODO - move to Interface.cpp?
void MainComponent::deinitializeMenuBar()
{
#if JUCE_MAC
    MenuBarModel::setMacMainMenu(nullptr);
#endif
}
