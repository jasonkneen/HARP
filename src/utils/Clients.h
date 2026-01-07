#pragma once

#include <juce_core/juce_core.h>

#include "../client/Client.h"
#include "../client/GradioClient.h"
#include "../client/providers/stability/StabilityClient.h"

#include "Errors.h"
#include "Logging.h"

using namespace juce;

inline OpResult multiplexClients(String modelPath, std::unique_ptr<Client>& client)
{
    if (StabilityClient::matchesPathSpec(modelPath))
    {
        DBG_AND_LOG(
            "utils::multiplexClients: Stability AI path detected. Initializing Stability client.");

        client = std::make_unique<StabilityClient>();
    }
    // Check Gradio last to serve as a catch-all if no third-party provider path specifications match
    else if (GradioClient::matchesPathSpec(modelPath))
    {
        DBG_AND_LOG("utils::multiplexClients: Gradio path detected. Initializing Gradio client.");

        client = std::make_unique<GradioClient>();
    }
    else
    {
        ClientError error { ClientError::Type::UnknownClient, modelPath, {} };

        DBG_AND_LOG("utils::multiplexClients: " << toUserMessage(error));

        return OpResult::fail(error);
    }

    return OpResult::ok();
}

/*
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
*/
