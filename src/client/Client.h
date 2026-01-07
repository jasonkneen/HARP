#pragma once

#include <juce_core/juce_core.h>

#include "../utils/Errors.h"
#include "../utils/Logging.h"

using namespace juce;

inline OpResult parseJSONString(const String& stringJSON, var& outData)
{
    const Result result = JSON::parse(stringJSON, outData);

    if (result.failed())
    {
        return OpResult::fail(JSONError { JSONError::Type::InvalidJSON, stringJSON });
    }

    return OpResult::ok();
}

inline OpResult stringJSONToDict(const String& stringJSON, DynamicObject::Ptr& dict)
{
    var data;

    OpResult result = parseJSONString(stringJSON, data);

    if (result.failed())
    {
        return result;
    }

    if (! data.isObject())
    {
        return OpResult::fail(JSONError { JSONError::Type::NotADictionary, stringJSON });
    }

    dict = data.getDynamicObject();

    jassert(dict != nullptr);

    return OpResult::ok();
}

inline OpResult stringJSONToList(const String& stringJSON, Array<var>& list)
{
    var data;

    OpResult result = parseJSONString(stringJSON, data);

    if (result.failed())
    {
        return result;
    }

    if (! data.isArray())
    {
        return OpResult::fail(JSONError { JSONError::Type::NotAList, stringJSON });
    }

    list = *data.getArray();

    return OpResult::ok();
}

class Client
{
public:
    Client()
    {
        // TODO
    }

    //~Client() override {} // TODO

    virtual String inferHostSlashModel(String modelPath) = 0;
    virtual String inferEndpointPath(String modelPath) = 0;
    virtual String inferDocumentationPath(String modelPath) = 0;

    virtual OpResult queryControls(String modelPath, DynamicObject::Ptr& controls) = 0;

    const String emptyJSONBody = R"({"data": []})";

    const String acceptHeader = "Accept: */*\r\n";
    const String contentTypeJSONHeader = "Content-Type: application/json\r\n";

    String toPrintableHeaders(String headers)
    {
        return headers.replace("\r", "\\r").replace("\n", "\\n");
    }

protected:
    String getCommonHeaders() const { return getAuthorizationHeader() + acceptHeader; }
    String getJSONHeaders() const { return getCommonHeaders() + contentTypeJSONHeader; }

private:
    String getAuthorizationHeader() const
    {
        String authorizationHeader;

        if (! accessToken.isEmpty())
        {
            authorizationHeader = "Authorization: Bearer " + accessToken + "\r\n";
        }

        return authorizationHeader;
    }

    String accessToken;
};
