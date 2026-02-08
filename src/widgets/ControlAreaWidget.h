/**
 * @file ControlAreaWidget.h
 * @brief Component comprising all model controls.
 * @author hugofloresgarcia, xribene, cwitkowitz
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../gui/HoverableLabel.h"
#include "../widgets/StatusAreaWidget.h"

#include "../gui/ComboBoxWithLabel.h"
#include "../gui/HoverHandler.h"
#include "../gui/SliderWithLabel.h"
#include "../gui/TextBoxWithLabel.h"
#include "../gui/ToggleWithLabel.h"

#include "../utils/Controls.h"
#include "../utils/Logging.h"

using namespace juce;

class ControlAreaWidget : public Component
{
public:
    ControlAreaWidget() { resetState(); }
    ~ControlAreaWidget() { resetState(); }

    void resized() override
    {
        auto area = getLocalBounds().reduced((int) marginSize);

        auto rows = buildRowsForWidth(area.getWidth());

        if (area.isEmpty() || rows.empty())
        {
            return;
        }

        int y = area.getY();

        for (const auto& row : rows)
        {
            if (row.empty())
            {
                continue;
            }

            int totalComponentWidth = 0;
            int rowHeight = 0;
            for (const auto& entry : row)
            {
                totalComponentWidth += entry.width;
                rowHeight = jmax(rowHeight, entry.height);
            }

            int slotGap = minInterItemGap;
            int edgeGap = minEdgeGap;

            int totalMinSpacing = minInterItemGap * jmax(0, (int) row.size() - 1);
            int remaining = area.getWidth() - totalComponentWidth - totalMinSpacing;

            if (remaining >= 0)
            {
                int slots = (int) row.size() + 1;
                int distributed = remaining / slots;
                edgeGap += distributed;
                slotGap += distributed;
            }

            int x = area.getX() + edgeGap;
            for (const auto& entry : row)
            {
                int componentY = y + (rowHeight - entry.height) / 2;
                entry.component->setBounds(x, componentY, entry.width, entry.height);
                x += entry.width + slotGap;
            }

            y += rowHeight + minRowGap;
        }
    }

    int getMinimumRequiredWidth() const
    {
        auto items = getOrderedLayoutItems();
        if (items.empty())
            return 0;

        int requiredWidth = 0;
        for (const auto& item : items)
        {
            requiredWidth =
                jmax(requiredWidth, item.component->getMinimumRequiredWidth());
        }

        return requiredWidth + 2 * ((int) marginSize + minEdgeGap);
    }

    int getRequiredHeightForWidth(int width) const
    {
        auto rows = buildRowsForWidth(width - 2 * (int) marginSize);
        if (rows.empty())
            return 0;

        int totalHeight = 2 * (int) marginSize;

        for (size_t i = 0; i < rows.size(); ++i)
        {
            int rowHeight = 0;
            for (const auto& entry : rows[i])
            {
                rowHeight = jmax(rowHeight, entry.height);
            }

            totalHeight += rowHeight;
            if (i + 1 < rows.size())
            {
                totalHeight += minRowGap;
            }
        }

        return totalHeight;
    }

    int getNumControls()
    {
        return textComponents.size() + toggleComponents.size() + sliderComponents.size()
               + dropdownComponents.size();
    }

    void resetState()
    {
        for (auto& c : textComponents)
        {
            removeChildComponent(c.get());
        }
        textComponents.clear();

        // TODO - numberComponents

        for (auto& c : toggleComponents)
        {
            removeChildComponent(c.get());
        }
        toggleComponents.clear();

        for (auto& c : sliderComponents)
        {
            removeChildComponent(c.get());
        }
        sliderComponents.clear();

        for (auto& c : dropdownComponents)
        {
            removeChildComponent(c.get());
        }
        dropdownComponents.clear();

        handlers.clear();
    }

    void updateControls(const ModelComponentInfoList& controlsInfo)
    {
        resetState();

        for (const auto& info : controlsInfo)
        {
            if (auto* textInfo = dynamic_cast<TextBoxComponentInfo*>(info.get()))
            {
                addTextBox(textInfo);
            }
            //else if (const auto* numberInfo = dynamic_cast<NumberBoxComponentInfo*>(info.get())) { addNumberBox(numberInfo); }
            else if (auto* toggleInfo = dynamic_cast<ToggleComponentInfo*>(info.get()))
            {
                addToggle(toggleInfo);
            }
            else if (auto* sliderInfo = dynamic_cast<SliderComponentInfo*>(info.get()))
            {
                addSlider(sliderInfo);
            }
            else if (auto* dropdownInfo = dynamic_cast<ComboBoxComponentInfo*>(info.get()))
            {
                addDropdown(dropdownInfo);
            }
            else
            {
                // Unsupported control detected
                jassertfalse;
            }
        }

        resized();
    }

private:
    void addTextBox(TextBoxComponentInfo* info)
    {
        std::unique_ptr<TextBoxWithLabel> textComponent =
            std::make_unique<TextBoxWithLabel>(info->label);

        auto& textBox = textComponent->getTextBox();

        textComponent->setText(info->value);

        addHandler(&textBox, info);
        textBox.addListener(info);

        addAndMakeVisible(*textComponent);

        textComponents.push_back(std::move(textComponent));
    }

    // TODO - void addNumberBox() {}

    void addToggle(ToggleComponentInfo* info)
    {
        std::unique_ptr<ToggleWithLabel> toggleComponent =
            std::make_unique<ToggleWithLabel>(info->label);

        auto& toggle = toggleComponent->getToggleButton();

        toggleComponent->setToggleState(info->value, dontSendNotification);

        addHandler(&toggle, info);
        toggle.addListener(info);

        addAndMakeVisible(*toggleComponent);

        toggleComponents.push_back(std::move(toggleComponent));
    }

    void addSlider(SliderComponentInfo* info)
    {
        std::unique_ptr<SliderWithLabel> sliderComponent =
            std::make_unique<SliderWithLabel>(info->label, Slider::RotaryHorizontalVerticalDrag);

        auto& slider = sliderComponent->getSlider();

        slider.setRange(info->minimum, info->maximum, info->step);
        slider.setValue(info->value);
        slider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);

        addHandler(&slider, info);
        slider.addListener(info);

        addAndMakeVisible(*sliderComponent);

        sliderComponents.push_back(std::move(sliderComponent));
    }

    void addDropdown(ComboBoxComponentInfo* info)
    {
        std::unique_ptr<ComboBoxWithLabel> dropdownComponent =
            std::make_unique<ComboBoxWithLabel>(info->label);

        auto& dropdown = dropdownComponent->getComboBox();
        auto font = dropdown.getLookAndFeel().getComboBoxFont(dropdown);
        int widestOptionText = 0;

        for (const auto& option : info->options)
        {
            dropdown.addItem(option, dropdown.getNumItems() + 1);
            widestOptionText = jmax(widestOptionText, font.getStringWidth(option));
        }

        if (! info->value.empty())
        {
            // Set initial selection if "value" was provided
            dropdown.setSelectedItemIndex(
                std::distance(info->options.begin(),
                              std::find(info->options.begin(), info->options.end(), info->value)),
                dontSendNotification);
        }
        else
        {
            // Fallback to first item
            dropdown.setSelectedItemIndex(0, dontSendNotification);
        }
        dropdown.setTextWhenNoChoicesAvailable("Empty");
        dropdownComponent->setMinimumContentWidth(widestOptionText);

        addHandler(&dropdown, info);
        dropdown.addListener(info);

        addAndMakeVisible(*dropdownComponent);

        dropdownComponents.push_back(std::move(dropdownComponent));
    }

    void addHandler(Component* comp, ModelComponentInfo* info)
    {
        std::unique_ptr<HoverHandler> handler = std::make_unique<HoverHandler>(*comp);

        handler->onMouseEnter = [this, info]() { setInstructions(info->info); };
        handler->onMouseExit = [this]() { clearInstructions(); };
        handler->attach();

        handlers.push_back(std::move(handler));
    }

    void setInstructions(const String& text)
    {
        if (text.isNotEmpty() && instructionsMessage != nullptr)
        {
            instructionsMessage->setMessage(text);
        }
    }

    void clearInstructions()
    {
        if (instructionsMessage != nullptr)
        {
            instructionsMessage->clearMessage();
        }
    }

    struct LayoutItem
    {
        enum class Type
        {
            Slider,
            Toggle,
            Dropdown,
            TextBox
        };

        ControlComponent* component;
        Type type;
    };

    struct RowEntry
    {
        ControlComponent* component = nullptr;
        int width = 0;
        int height = 0;
    };

    std::vector<LayoutItem> getOrderedLayoutItems() const
    {
        std::vector<LayoutItem> items;
        items.reserve(sliderComponents.size() + toggleComponents.size()
                      + dropdownComponents.size() + textComponents.size());

        for (const auto& slider : sliderComponents)
            items.push_back({ slider.get(), LayoutItem::Type::Slider });

        for (const auto& toggle : toggleComponents)
            items.push_back({ toggle.get(), LayoutItem::Type::Toggle });

        for (const auto& dropdown : dropdownComponents)
            items.push_back({ dropdown.get(), LayoutItem::Type::Dropdown });

        for (const auto& text : textComponents)
            items.push_back({ text.get(), LayoutItem::Type::TextBox });

        return items;
    }

    std::vector<std::vector<RowEntry>> buildRowsForWidth(int width) const
    {
        std::vector<std::vector<RowEntry>> rows;

        auto items = getOrderedLayoutItems();

        if (items.empty() || width <= 0)
            return rows;

        rows.emplace_back();
        int currentRowWidth = 0;

        for (const auto& item : items)
        {
            int minWidth = item.component->getMinimumRequiredWidth();
            int itemWidth = getPreferredWidth(item.type);
            int itemHeight = getMinimumRequiredHeight(item.type);

            itemWidth = jmax(minWidth, itemWidth);
            itemWidth = jmin(itemWidth, width);

            auto& row = rows.back();
            int gapContribution = row.empty() ? 0 : minInterItemGap;
            int candidateWidth = currentRowWidth + gapContribution + itemWidth;

            if (! row.empty() && candidateWidth > width)
            {
                rows.emplace_back();
                currentRowWidth = 0;
            }

            auto& activeRow = rows.back();
            if (! activeRow.empty())
                currentRowWidth += minInterItemGap;

            activeRow.push_back(RowEntry { item.component, itemWidth, itemHeight });
            currentRowWidth += itemWidth;
        }

        return rows;
    }

    int getMinimumRequiredHeight(LayoutItem::Type type) const
    {
        switch (type)
        {
            case LayoutItem::Type::Slider:
                return minSliderHeight;
            case LayoutItem::Type::Toggle:
                return minToggleHeight;
            case LayoutItem::Type::Dropdown:
                return minDropdownHeight;
            case LayoutItem::Type::TextBox:
                return minTextBoxHeight;
        }

        return minDropdownHeight;
    }

    int getPreferredWidth(LayoutItem::Type type) const
    {
        switch (type)
        {
            case LayoutItem::Type::Slider:
                return preferredSliderWidth;
            case LayoutItem::Type::Toggle:
                return preferredToggleWidth;
            case LayoutItem::Type::Dropdown:
                return preferredDropdownWidth;
            case LayoutItem::Type::TextBox:
                return preferredTextBoxWidth;
        }

        return preferredDropdownWidth;
    }

    // Layout constants
    static constexpr float marginSize = 4;

    // Minimum heights for each control type to prevent overlapping
    static constexpr int minSliderHeight = 108;
    static constexpr int minToggleHeight = 34;
    static constexpr int minDropdownHeight = 44;
    static constexpr int minTextBoxHeight = 84;

    static constexpr int preferredSliderWidth = 108;
    static constexpr int preferredToggleWidth = 112;
    static constexpr int preferredDropdownWidth = 140;
    static constexpr int preferredTextBoxWidth = 200;
    static constexpr int minInterItemGap = 6;
    static constexpr int minEdgeGap = 4;
    static constexpr int minRowGap = 6;

    std::vector<std::unique_ptr<TextBoxWithLabel>> textComponents;
    // TODO - numberComponents
    std::vector<std::unique_ptr<ToggleWithLabel>> toggleComponents;
    std::vector<std::unique_ptr<SliderWithLabel>> sliderComponents;
    std::vector<std::unique_ptr<ComboBoxWithLabel>> dropdownComponents;

    std::vector<std::unique_ptr<HoverHandler>> handlers;

    SharedResourcePointer<InstructionsMessage> instructionsMessage;
};
