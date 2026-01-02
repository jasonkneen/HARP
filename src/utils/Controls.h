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

struct ModelComponent
{
    //Uuid id { "" }; // TODO - is this field necessary?

    std::string label { "" };
    std::string info { "" };

    ModelComponent(DynamicObject* input)
    {
        // TODO - check that the following properties are of the correct type

        //id = juce::Uuid();

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

struct AudioTrackComponent : public ModelComponent
{
    bool required = true;

    AudioTrackComponent(DynamicObject* input) : ModelComponent(input)
    {
        // TODO - check that the following properties are of the correct type

        if (input->hasProperty("required"))
        {
            required = stringToBool(input->getProperty("required").toString());
        }
    }
};

struct MidiTrackComponent : public ModelComponent
{
    bool required = true;

    MidiTrackComponent(DynamicObject* input) : ModelComponent(input)
    {
        // TODO - check that the following properties are of the correct type

        if (input->hasProperty("required"))
        {
            required = stringToBool(input->getProperty("required").toString());
        }
    }
};

struct TextBoxComponent : public ModelComponent
{
    std::string value { "" };

    TextBoxComponent(DynamicObject* input) : ModelComponent(input)
    {
        // TODO - check that the following properties are of the correct type

        if (input->hasProperty("value"))
        {
            value = input->getProperty("value").toString().toStdString();
        }
    }
};

struct NumberBoxComponent : public ModelComponent
{
    // TODO - consistency with SliderComponent
    double min;
    double max;
    double value;

    NumberBoxComponent(DynamicObject* input) : ModelComponent(input)
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

struct ToggleComponent : public ModelComponent
{
    bool value = false;

    ToggleComponent(DynamicObject* input) : ModelComponent(input)
    {
        // TODO - check that the following properties are of the correct type

        if (input->hasProperty("value"))
        {
            value = stringToBool(input->getProperty("value").toString());
        }
    }
};

struct SliderComponent : public ModelComponent
{
    double minimum;
    double maximum;
    double step;
    double value;

    SliderComponent(DynamicObject* input) : ModelComponent(input)
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

struct ComboBoxComponent : public ModelComponent
{
    std::vector<std::string> options;

    std::string value;

    ComboBoxComponent(DynamicObject* input) : ModelComponent(input)
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

// TODO - are all of these necessary?
using ModelComponentPair = std::pair<Uuid, std::shared_ptr<ModelComponent>>;
using ModelComponentMap = std::map<Uuid, std::shared_ptr<ModelComponent>>;
using ModelComponentPairList = std::vector<ModelComponentPair>;
using ModelComponentList = std::vector<std::shared_ptr<ModelComponent>>;
