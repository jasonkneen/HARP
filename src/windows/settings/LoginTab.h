#pragma once

#include <JuceHeader.h>

#include "../../client/Client.h"

#include "../../utils/Clients.h"
#include "../../utils/Logging.h"
#include "../../utils/Settings.h"

using namespace juce;

class ProviderPage : public Component
{
public:
    ProviderPage(Provider p, String n) : provider(p), displayName(n)
    {
        // Load appropriate client
        client = multiplexClients(provider);

        String titleText = "Update or remove your API key for " + displayName + ".";

        titleLabel.setText(titleText, dontSendNotification);
        titleLabel.setJustificationType(Justification::centred);

        getTokenLink.setButtonText("Click here to register a new API key.");
        getTokenLink.setURL(client->tokenRegistrationURL);
        getTokenLink.setColour(HyperlinkButton::textColourId, Colours::blue);

        addAndMakeVisible(titleLabel);
        addAndMakeVisible(getTokenLink);

        String emptyText = "Enter API key here";

        tokenEditor.setTextToShowWhenEmpty(emptyText, Colours::grey);
        tokenEditor.setReturnKeyStartsNewLine(false);
        tokenEditor.setSelectAllWhenFocused(true);
        tokenEditor.onTextChange = [this]()
        {
            bool hasText = tokenEditor.getText().trim().isNotEmpty();

            updateButton.setEnabled(hasText);
        };
        tokenEditor.onReturnKey = [this] { updateButton.triggerClick(); };

        addAndMakeVisible(tokenEditor);

        updateButton.addShortcut(KeyPress(KeyPress::returnKey));
        updateButton.onClick = [this] { updateTokenCallback(); };
        removeButton.onClick = [this] { removeTokenCallback(); };

        if (sharedTokens->savedTokens.contains(p))
        {
            String savedToken = sharedTokens->savedTokens[p];

            OpResult result = client->validateToken(savedToken);

            if (result.failed())
            {
                statusLabel.setText(std::move(tokenInvalidMessage), dontSendNotification);
            }
            else
            {
                statusLabel.setText(std::move(tokenLoadedMessage), dontSendNotification);
            }

            tokenEditor.setText(savedToken);
            removeButton.setEnabled(true);
        }
        else
        {
            statusLabel.setText(std::move(tokenMissingMessage), dontSendNotification);
            updateButton.setEnabled(false);
            removeButton.setEnabled(false);
        }

        addAndMakeVisible(updateButton);
        addAndMakeVisible(removeButton);

        statusLabel.setJustificationType(Justification::centred);

        addAndMakeVisible(statusLabel);
    }

    void resized() override
    {
        FlexBox pageArea;
        pageArea.flexDirection = FlexBox::Direction::column;

        pageArea.items.add(FlexItem(titleLabel).withHeight(rowHeight).withMargin(marginSize));
        pageArea.items.add(FlexItem().withHeight(gapSize)); // Filler space
        pageArea.items.add(FlexItem(tokenEditor).withHeight(rowHeight).withMargin(marginSize));

        FlexBox buttonsArea;
        buttonsArea.flexDirection = FlexBox::Direction::row;

        buttonsArea.items.add(FlexItem().withFlex(1.0)); // Filler space
        buttonsArea.items.add(FlexItem(updateButton).withWidth(buttonWidth));
        buttonsArea.items.add(FlexItem().withWidth(gapSize)); // Filler space
        buttonsArea.items.add(FlexItem(removeButton).withWidth(buttonWidth));
        buttonsArea.items.add(FlexItem().withFlex(1.0)); // Filler space

        pageArea.items.add(FlexItem().withHeight(gapSize)); // Filler space
        pageArea.items.add(FlexItem(buttonsArea).withHeight(rowHeight));
        pageArea.items.add(FlexItem().withFlex(1.0)); // Filler space

        pageArea.items.add(FlexItem(statusLabel).withHeight(rowHeight));
        pageArea.items.add(FlexItem().withFlex(1.0)); // Filler space

        pageArea.items.add(FlexItem(getTokenLink).withHeight(rowHeight).withMargin(marginSize));

        pageArea.performLayout(getLocalBounds());
    }

    String getDisplayName() { return displayName; }

private:
    void updateTokenCallback()
    {
        // TODO
    }

    void removeTokenCallback()
    {
        // TODO
    }

    const float marginSize = 2;
    const float gapSize = 10;
    const float rowHeight = 26;
    const float buttonWidth = 120;

    Provider provider;
    String displayName;

    Label titleLabel;
    HyperlinkButton getTokenLink;

    TextEditor tokenEditor;

    TextButton updateButton { "Update" };
    TextButton removeButton { "Remove" };

    std::unique_ptr<Client> client;

    String tokenMissingMessage = "No token has been added for this provider.";
    String tokenInvalidMessage = "This token is invalid.";
    String tokenLoadedMessage = "This token has been loaded.";
    Label statusLabel;

    SharedResourcePointer<SharedAPIKeys> sharedTokens;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProviderPage)
};

class PageSwitcher : public Component
{
public:
    struct Entry
    {
        String displayName;
        std::unique_ptr<ProviderPage> page;
    };

    void resized() override
    {
        for (auto& e : entries)
        {
            e.page->setBounds(getLocalBounds());
        }
    }

    int getNumPages() { return entries.size(); }

    const String& getNameForIndex(int idx) const { return entries[(size_t) idx].displayName; }

    void showPageByIndex(int idx)
    {
        for (int i = 0; i < (int) entries.size(); ++i)
        {
            entries[i].page->setVisible(i == idx);
        }
    }

    void addPage(std::unique_ptr<ProviderPage> page)
    {
        addAndMakeVisible(page.get());
        page->setVisible(entries.empty());

        entries.push_back({ std::move(page->getDisplayName()), std::move(page) });
    }

private:
    std::vector<Entry> entries;
};

class LoginTab : public Component, private ListBoxModel
{
public:
    LoginTab()
    {
        sidebar.setModel(this);
        sidebar.setRowHeight(36);
        sidebar.setOutlineThickness(0);
        addAndMakeVisible(sidebar);

        loginPages.addPage(std::make_unique<ProviderPage>(Provider::HuggingFace, "Hugging Face"));
        loginPages.addPage(std::make_unique<ProviderPage>(Provider::Stability, "Stability AI"));
        addAndMakeVisible(loginPages);

        sidebar.updateContent();
        sidebar.selectRow(0);
    }

    void resized() override
    {
        Rectangle<int> area = getLocalBounds();

        sidebar.setBounds(area.removeFromLeft(100));

        loginPages.setBounds(area);
    }

private:
    int getNumRows() override { return loginPages.getNumPages(); }

    void paintListBoxItem(int rowNumber,
                          Graphics& g,
                          int width,
                          int height,
                          bool rowIsSelected) override
    {
        if (rowIsSelected)
        {
            g.fillAll(Colours::darkgrey.withAlpha(0.4f));
        }

        g.setColour(Colours::white);
        g.setFont(14.0f);
        g.drawText(
            loginPages.getNameForIndex(rowNumber), 0, 0, width, height, Justification::centred);
    }

    void selectedRowsChanged(int row) override { loginPages.showPageByIndex(row); }

    ListBox sidebar;
    PageSwitcher loginPages;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoginTab)
};
