#pragma once

#include <juce_core/juce_core.h>

using namespace juce;

struct OutputLabel
{
    // Required fields
    float t;
    String label;

    // Optional fields
    std::optional<String> description;
    std::optional<float> duration;
    std::optional<int> color;
    std::optional<String> link;

    virtual ~OutputLabel() = default; // Virtual destructor
};

struct AudioLabel : public OutputLabel
{
    // Optional on pyharp side
    std::optional<float> amplitude;
};

struct SpectrogramLabel : public OutputLabel
{
    // Optional on pyharp side
    std::optional<float> frequency;
};

struct MidiLabel : public OutputLabel
{
    // Optional on pyharp side
    std::optional<float> pitch;
};

using LabelList = std::vector<std::unique_ptr<OutputLabel>>;
