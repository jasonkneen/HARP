#pragma once

#include <juce_core/juce_core.h>

using namespace juce;

// TODO - the following patterns should be supported
// "xribene/midi_pitch_shifter",
// "http://localhost:7860",
// "https://xribene-midi-pitch-shifter.hf.space/",
// "https://huggingface.co/spaces/xribene/midi_pitch_shifter"

enum GradioEvents
{
    complete,
    error,
    heartbeat,
    generating
};

struct SpaceInfo
{
    enum Status
    {
        GRADIO,
        HUGGINGFACE,
        LOCALHOST,
        STABILITY,
        FAILED,
        EMPTY
    };

    String huggingface;
    String gradio;
    String stability;
    String userInput;
    String modelName;
    String userName;
    String error;
    Status status;

    std::optional<juce::String> stabilityServiceType;
    juce::String apiEndpointURL; // The primary API endpoint URL for the space

    SpaceInfo() : status(Status::EMPTY) {}

    String getStatusString() const
    {
        switch (status)
        {
            case GRADIO:
                return "Gradio";
            case HUGGINGFACE:
                return "HuggingFace";
            case LOCALHOST:
                return "Localhost";
            case STABILITY:
                if (stabilityServiceType.has_value())
                    return "Stability (" + stabilityServiceType.value() + ")";
                return "Stability";
            case FAILED:
                return "Error";
            case EMPTY:
                return "Empty";
            default:
                return "Unknown";
        }
    }
    String toString()
    {
        String str = "SpaceInfo: \n";
        str += "UserInput: " + userInput + "\n";
        str += "Status: " + getStatusString() + "\n";
        str += "API Endpoint: " + apiEndpointURL + "\n";
        if (status == STABILITY && stabilityServiceType.has_value())
        {
            str += "Service: " + stabilityServiceType.value() + "\n";
        }
        else
        {
            str += "Huggingface: " + huggingface + "\n";
            str += "Gradio: " + gradio + "\n";
            str += "ModelName: " + modelName + "\n";
            str += "UserName: " + userName + "\n";
        }
        str += "Error: " + error + "\n";
        return str;
    }

    String getModelSlashUser() const
    {
        if (status == LOCALHOST)
        {
            return "localhost";
        }
        else if (status == STABILITY)
        {
            if (stabilityServiceType.has_value())
                return "stability/" + stabilityServiceType.value();
            return "stability/unknown_service";
        }
        else if (userName.isNotEmpty() && modelName.isNotEmpty())
        {
            return userName + "/" + modelName;
        }
        return "Unknown/Unknown";
    }
};
