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
