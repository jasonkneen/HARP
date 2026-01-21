/**
 * @file
 * @brief Parent class for making HTTP requests to an API
 * @author huiranyu
 */

#pragma once

#include "juce_core/juce_core.h"
#include <fstream>

#include "../utils/Clients.h"
#include "../utils/Errors.h"
#include "../utils/Labels.h"
#include "../utils/Logging.h"

using namespace juce;

class Client
{
public:
    Client() = default;
    virtual ~Client() {};

    // Requests
    virtual OpResult uploadFileRequest(const File&, String&, const int timeoutMs = 10000) const = 0;
    virtual OpResult processRequest(Error&, String&, std::vector<String>&, LabelList&) = 0;
    virtual OpResult cancel() = 0;

    // Authorization
    void setToken(const String& t) { accessToken = t; }
    String getToken() const { return accessToken; }

protected:

    String accessToken;
    URL tokenValidationURL;

    SpaceInfo spaceInfo;
};
