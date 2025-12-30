#pragma once

#include <juce_core/juce_core.h>

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

    virtual String queryControls(String modelPath) = 0;

private:
    String accessToken;
};
