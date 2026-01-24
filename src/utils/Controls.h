#pragma once

#include <juce_core/juce_core.h>

using namespace juce;

/**
 * Helper function to convert a boolean string to a C++ boolean
 * value. JUCE doesn't have a built-in function for this purpose.
 */
inline bool stringToBool(const String& str)
{
    String lowerStr = str.toLowerCase();

    if (lowerStr == "true" || lowerStr == "1" || lowerStr == "yes" || lowerStr == "y")
    {
        return true;
    }

    return false;
}

struct ModelComponentInfo
{
    Uuid id { "" };

    std::string label { "" };
    std::string info { "" };

    ModelComponentInfo() = default;
    virtual ~ModelComponentInfo() = default;

    ModelComponentInfo(DynamicObject* input)
    {
        id = Uuid();

        // TODO - check that the following properties are of the correct type

        if (input->hasProperty("label"))
        {
            label = input->getProperty("label").toString().toStdString();
        }
        if (input->hasProperty("info"))
        {
            info = input->getProperty("info").toString().toStdString();
        }
    }
};

struct TrackComponentInfo : public ModelComponentInfo
{
    bool required = true;

    std::string remotePath { "" }; // Used when uploading files

    TrackComponentInfo() = default;
    virtual ~TrackComponentInfo() = default; // Make child-tree polymorphic

    TrackComponentInfo(DynamicObject* input) : ModelComponentInfo(input)
    {
        // TODO - check that the following properties are of the correct type

        if (input->hasProperty("required"))
        {
            required = stringToBool(input->getProperty("required").toString());
        }
    }
};

struct AudioTrackComponentInfo : public TrackComponentInfo
{
    using TrackComponentInfo::TrackComponentInfo;
};

struct MidiTrackComponentInfo : public TrackComponentInfo
{
    using TrackComponentInfo::TrackComponentInfo;
};

struct TextBoxComponentInfo : public ModelComponentInfo
{
    std::string value { "" };

    TextBoxComponentInfo(DynamicObject* input) : ModelComponentInfo(input)
    {
        // TODO - check that the following properties are of the correct type

        if (input->hasProperty("value"))
        {
            value = input->getProperty("value").toString().toStdString();
        }
    }
};

struct NumberBoxComponentInfo : public ModelComponentInfo
{
    // TODO - consistency with SliderComponent
    double min;
    double max;
    double value;

    NumberBoxComponentInfo(DynamicObject* input) : ModelComponentInfo(input)
    {
        // TODO - check that the following properties are of the correct type

        if (input->hasProperty("min"))
        {
            min = input->getProperty("min").toString().getFloatValue();
        }
        if (input->hasProperty("max"))
        {
            max = input->getProperty("max").toString().getFloatValue();
        }
        if (input->hasProperty("value"))
        {
            value = input->getProperty("value").toString().getFloatValue();
        }
    }
};

struct ToggleComponentInfo : public ModelComponentInfo
{
    bool value = false;

    ToggleComponentInfo(DynamicObject* input) : ModelComponentInfo(input)
    {
        // TODO - check that the following properties are of the correct type

        if (input->hasProperty("value"))
        {
            value = stringToBool(input->getProperty("value").toString());
        }
    }
};

struct SliderComponentInfo : public ModelComponentInfo
{
    double minimum;
    double maximum;
    double step;
    double value;

    SliderComponentInfo(DynamicObject* input) : ModelComponentInfo(input)
    {
        // TODO - check that the following properties are of the correct type

        if (input->hasProperty("minimum"))
        {
            minimum = input->getProperty("minimum").toString().getFloatValue();
        }
        if (input->hasProperty("maximum"))
        {
            maximum = input->getProperty("maximum").toString().getFloatValue();
        }
        if (input->hasProperty("step"))
        {
            step = input->getProperty("step").toString().getFloatValue();
        }
        if (input->hasProperty("value"))
        {
            value = input->getProperty("value").toString().getFloatValue();
        }
    }
};

struct ComboBoxComponentInfo : public ModelComponentInfo
{
    std::vector<std::string> options;

    std::string value;

    ComboBoxComponentInfo(DynamicObject* input) : ModelComponentInfo(input)
    {
        // TODO - check that the following properties are of the correct type

        if (input->hasProperty("choices"))
        {
            Array<var>* choices = input->getProperty("choices").getArray();

            if (choices == nullptr)
            {
                // TODO - handle error case: couldn't load choices
            }

            int numChoices = choices->size();

            if (numChoices > 0)
            {
                for (int j = 0; j < numChoices; j++)
                {
                    options.push_back(
                        choices->getReference(j).getArray()->getFirst().toString().toStdString());
                }

                if (! input->hasProperty("value"))
                {
                    // Set selection to first option
                    value = options[0];
                }
                else
                {
                    // Set selection to chosen value
                    value = input->getProperty("value").toString().toStdString();
                }
            }
            else
            {
                // TODO - handle error case: no choices
            }
        }
        else
        {
            // TODO - handle error case: no choises provided
        }
    }
};

using ModelComponentInfoList = std::vector<std::shared_ptr<ModelComponentInfo>>;
