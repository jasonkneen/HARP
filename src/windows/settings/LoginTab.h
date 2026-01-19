#pragma once

#include <JuceHeader.h>

#include "../../external/magic_enum.hpp"

#include "../../client/Client.h"

#include "../../utils/Logging.h"
#include "../../utils/Settings.h"

using namespace juce;

class ProviderPage : public Component
{
public:
    explicit ProviderPage(Provider p) : provider(p)
    {
        String providerName = std::string(magic_enum::enum_name(p)).c_str();

        titleLabel.setText(providerName, dontSendNotification);

        addAndMakeVisible(titleLabel);
        addAndMakeVisible(infoLabel);
        addAndMakeVisible(registerLabel);
        addAndMakeVisible(linkLabel);

        addAndMakeVisible(tokenEditor);

        addAndMakeVisible(updateButton);
        addAndMakeVisible(removeButton);

        addAndMakeVisible(statusLabel);
    }

    void resized() override
    {
        //DBG_AND_LOG("LoginTab::resized()");
        auto area = getLocalBounds().reduced(10);
        const int rowH = 26;
        const int gap = 10;
        const int btnW = 120;
        // Messages
        titleLabel.setBounds(area.removeFromTop(rowH));
        area.removeFromTop(gap);
        infoLabel.setBounds(area.removeFromTop(rowH));
        area.removeFromTop(gap);
        auto registerRow = area.removeFromTop(rowH);
        registerLabel.setBounds(registerRow.removeFromLeft(100));
        linkLabel.setBounds(registerRow.removeFromLeft(btnW));

        // Token and Toggle
        tokenEditor.setBounds(area.removeFromTop(rowH));
        // rememberTokenToggle.setBounds(area.removeFromTop(rowH));
        area.removeFromTop(gap);
        // Buttons
        auto buttonsRow = area.removeFromTop(rowH);
        const int totalBtnW = btnW * 2 + gap;
        auto right = buttonsRow.removeFromRight(totalBtnW);
        updateButton.setBounds(right.removeFromLeft(btnW));
        right.removeFromLeft(gap);
        removeButton.setBounds(right.removeFromLeft(btnW));
        // right.removeFromLeft(gap);
        // tokenButton.setBounds(right); // remaining btnW
        // Status
        statusLabel.setBounds(area.removeFromBottom(rowH));
    }

private:
    Provider provider;

    Label titleLabel;
    Label infoLabel;
    Label registerLabel;
    HyperlinkButton linkLabel;

    TextEditor tokenEditor;

    TextButton updateButton { "Update" };
    TextButton removeButton { "Remove" };

    Label statusLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProviderPage)
};

class ComponentSwitcher : public Component
{
public:
    void resized() override
    {
        for (auto* c : components)
        {
            c->setBounds(getLocalBounds());
        }
    }

    int getNumComponents() { return components.size(); }

    void addComponent(std::unique_ptr<Component> c)
    {
        Component* c_ptr = c.get();
        components.add(std::move(c));
        addAndMakeVisible(c_ptr);

        // Make visible if this is only component
        c_ptr->setVisible(components.size() == 1);
    }

    void showComponent(int idx)
    {
        for (int i = 0; i < components.size(); ++i)
        {
            components[i]->setVisible(i == idx);
        }
    }

private:
    OwnedArray<Component> components;
};

class LoginTab : public Component, private ListBoxModel
{
public:
    LoginTab()
    {
        names = { "Hugging Face", "Stability AI" };

        sidebar.setModel(this);
        sidebar.setRowHeight(36);
        sidebar.setOutlineThickness(0);
        addAndMakeVisible(sidebar);

        loginPages.addComponent(
            std::make_unique<ProviderPage>(Provider::HuggingFace));
        loginPages.addComponent(std::make_unique<ProviderPage>(Provider::Stability));
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
    int getNumRows() override { return names.size(); }

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
        g.drawText(names[rowNumber], 0, 0, width, height, Justification::centred);
    }

    void selectedRowsChanged(int row) override { loginPages.showComponent(row); }

    ListBox sidebar;
    ComponentSwitcher loginPages;

    StringArray names;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoginTab)
};
