#pragma once

#include <juce_core/juce_core.h>

#include "../utils/Errors.h"
#include "../utils/Logging.h"

using namespace juce;

class Client
{
public:
    Client()
    {
        // TODO
    }

    //~Client() override {} // TODO

    virtual String inferHostSlashModel(String modelPath) = 0;
    virtual String inferEndpointURL(String modelPath) = 0;
    virtual String inferDocumentationURL(String modelPath) = 0;

    virtual OpResult queryControls(String modelPath, String& queryResponse) = 0;

private:
    String accessToken;
};
