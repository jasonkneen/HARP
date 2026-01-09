#pragma once

#include "Client.h"

#include "../utils/Enums.h"
#include "../utils/Errors.h"

using namespace juce;

class GradioClient : public Client
{
public:
    enum GradioEvents
    {
        Complete,
        Heartbeat,
        Error,
        //Generating
    };

    GradioClient()
    {
        // TODO
    }

    //~GradioClient() override {} // TODO

    static bool matchesPathSpec(String modelPath)
    {
        return isValidLocalPath(modelPath) || isValidGradioPath(modelPath)
               || isValidHuggingFacePath(modelPath);
    }

    String inferHostSlashModel(String modelPath) override
    {
        String hostSlashModel;

        if (isValidLocalPath(modelPath) || isValidGradioPath(modelPath))
        {
            hostSlashModel = "localhost";
        }
        else if (isValidHuggingFacePath(modelPath))
        {
            if (isValidShortHuggingFacePath(modelPath))
            {
                StringArray array = StringArray::fromTokens(
                    modelPath.fromFirstOccurrenceOf("https://", false, false)
                        .upToFirstOccurrenceOf(".hf.space", false, false),
                    "-",
                    "");

                hostSlashModel = array[0] + "/" + array[1];
            }
            else if (isValidLongHuggingFacePath(modelPath))
            {
                StringArray array = StringArray::fromTokens(
                    modelPath.fromFirstOccurrenceOf("https://huggingface.co/spaces/", false, false),
                    "/",
                    "");

                hostSlashModel = array[0] + "/" + array[1];
            }
            else if (isValidAbbrevHuggingFacePath(modelPath))
            {
                hostSlashModel = modelPath;
            }
        }
        else
        {
            DBG_AND_LOG("GradioClient::inferHostSlashModel: Path \""
                        << modelPath << "\" does not match valid specification for Gradio.");
        }

        return hostSlashModel;
    }

    String inferEndpointPath(String modelPath) override
    {
        String endpointPath;

        if (isValidLocalPath(modelPath))
        {
            endpointPath = modelPath;

            if (! endpointPath.startsWith("http://"))
            {
                endpointPath = "http://" + endpointPath;
            }
        }
        else if (isValidGradioPath(modelPath))
        {
            endpointPath = modelPath;
        }
        else if (isValidHuggingFacePath(modelPath))
        {
            if (isValidShortHuggingFacePath(modelPath))
            {
                endpointPath = modelPath;
            }
            else if (isValidLongHuggingFacePath(modelPath)
                     || isValidAbbrevHuggingFacePath(modelPath))
            {
                String hostSlashModel = inferHostSlashModel(modelPath);

                auto array = StringArray::fromTokens(hostSlashModel, "/", "");

                String host = array[0];
                String model = array[1];

                endpointPath = "https://" + host + "-" + model.replace("_", "-") + ".hf.space/";
            }
        }
        else
        {
            DBG_AND_LOG("GradioClient::inferEndpointURL: Path \""
                        << modelPath << "\" does not match valid specification for Gradio.");
        }

        return endpointPath;
    }

    String inferDocumentationPath(String modelPath) override
    {
        String documentationPath;

        if (isValidLocalPath(modelPath) || isValidGradioPath(modelPath))
        {
            documentationPath = inferEndpointPath(documentationPath);
        }
        else if (isValidHuggingFacePath(modelPath))
        {
            if (isValidShortHuggingFacePath(modelPath) || isValidAbbrevHuggingFacePath(modelPath))
            {
                documentationPath = "https://huggingface.co/spaces/" + inferHostSlashModel(modelPath);
            }
            else if (isValidLongHuggingFacePath(modelPath))
            {
                documentationPath = modelPath;
            }
        }
        else
        {
            DBG_AND_LOG("GradioClient::inferDocumentationPath: Path \""
                        << modelPath << "\" does not match valid specification for Gradio.");
        }

        return documentationPath;
    }

    OpResult queryControls(String modelPath, DynamicObject::Ptr& controls)
    {
        String responseJSON;

        OpResult result = makePOSTRequest(modelPath, "controls", emptyJSONBody, responseJSON);

        if (result.failed())
        {
            return result;
        }

        DynamicObject::Ptr responseDict;

        result = stringJSONToDict(responseJSON, responseDict);

        if (result.failed())
        {
            return result;
        }

        static const Identifier eventKey { "event_id" };

        if (! responseDict->hasProperty(eventKey))
        {
            return OpResult::fail(JsonError { JsonError::Type::MissingKey,
                                              JSON::toString(var(responseDict.get()), true),
                                              eventKey.toString() });
        }

        String eventID = responseDict->getProperty(eventKey);

        DBG_AND_LOG("GradioClient::queryControls: Process created with ID \"" << eventID << "\".");

        responseJSON.clear();

        result = makeGETRequest(modelPath, "controls", eventID, responseJSON);

        if (result.failed())
        {
            return result;
        }

        Array<var> dataList;

        result = stringJSONToList(responseJSON, dataList);

        if (result.failed())
        {
            return result;
        }

        if (dataList.isEmpty())
        {
            return OpResult::fail(JsonError { JsonError::Type::Empty, {} });
        }

        var first = dataList.getFirst();

        if (! first.isObject())
        {
            return OpResult::fail(JsonError { JsonError::Type::NotADictionary, first.toString() });
        }

        // Extract model metadata, inputs, controls, and outputs
        controls = first.getDynamicObject();

        if (controls == nullptr)
        {
            return OpResult::fail(JsonError { JsonError::Type::InvalidJSON, first.toString() });
        }

        return OpResult::ok();
    }

private:
    static bool isValidLocalPath(String modelPath)
    {
        /*
          e.g., "http://localhost:7860" or "http://127.0.0.1:7860"
        */

        return modelPath.contains("localhost") || modelPath.matchesWildcard("*.*.*.*:*", true);
    }

    static bool isValidGradioPath(String modelPath)
    {
        /*
          e.g., "https://<RANDOM_STRING>.gradio.live"
        */

        return modelPath.startsWith("https://") && modelPath.endsWith(".gradio.live");
    }

    static bool isValidHuggingFacePath(String modelPath)
    {
        return isValidShortHuggingFacePath(modelPath) || isValidLongHuggingFacePath(modelPath)
               || isValidAbbrevHuggingFacePath(modelPath);
    }

    static bool isValidShortHuggingFacePath(String modelPath)
    {
        /*
          e.g., "https://xribene-midi-pitch-shifter.hf.space/"
        */

        StringArray array =
            StringArray::fromTokens(modelPath.fromFirstOccurrenceOf("https://", false, false)
                                        .upToFirstOccurrenceOf(".hf.space", false, false),
                                    "-",
                                    "");

        if (array.size() == 2)
        {
            return true;
        }
        else if (array.size() != 0)
        {
            /*
              This is ambiguous. There's no way to tell where the delimeter
              belongs and which hypens were converted from underscores.
            */

            DBG_AND_LOG(
                "GradioClient::isValidShortHuggingFacePath: Path \""
                << modelPath
                << "\" is ambiguous. Please use the abbreviated or long-form path instead.");

            return false;
        }
        else
        {
            return false;
        }
    }

    static bool isValidLongHuggingFacePath(String modelPath)
    {
        /*
          e.g., "https://huggingface.co/spaces/xribene/midi_pitch_shifter"
        */

        return isValidAbbrevHuggingFacePath(
            modelPath.fromFirstOccurrenceOf("https://huggingface.co/spaces/", false, false));
    }

    static bool isValidAbbrevHuggingFacePath(String modelPath)
    {
        /*
          e.g., "xribene/midi_pitch_shifter"
        */

        StringArray array = StringArray::fromTokens(modelPath, "/", "");

        return array.size() == 2;
    }

    URL getEndpoint(String endpointPath, String functionName)
    {
        URL endpointURL = URL(endpointPath)
                              .getChildURL("gradio_api")
                              .getChildURL("call")
                              .getChildURL(functionName);

        return endpointURL;
    }

    OpResult makePOSTRequest(const String modelPath,
                             const String functionName,
                             const String body,
                             String& response,
                             const int timeoutMs = 10000)
    {
        URL endpoint = getEndpoint(inferEndpointPath(modelPath), functionName);

        String requestHeaders = getJSONHeaders();

        DBG_AND_LOG("GradioClient::makePOSTRequest: Attempting to make POST request for endpoint \""
                    << endpoint.toString(true) << "\" with JSON body \"" << body
                    << "\" and headers \"" << toPrintableHeaders(requestHeaders) << "\".");

        endpoint = endpoint.withPOSTData(body);

        if (! endpoint.isWellFormed())
        {
            return OpResult::fail(HttpError {
                HttpError::Type::InvalidURL, HttpError::Request::POST, inferDocumentationPath(modelPath) });
        }

        int statusCode = 0;
        StringPairArray responseHeaders;

        auto options = URL::InputStreamOptions(URL::ParameterHandling::inPostData)
                           .withExtraHeaders(requestHeaders)
                           .withConnectionTimeoutMs(timeoutMs)
                           .withResponseHeaders(&responseHeaders)
                           .withStatusCode(&statusCode)
                           .withNumRedirectsToFollow(5)
                           .withHttpRequestCmd("POST");

        std::unique_ptr<InputStream> stream(endpoint.createInputStream(options));

        if (stream == nullptr)
        {
            return OpResult::fail(HttpError { HttpError::Type::ConnectionFailed,
                                              HttpError::Request::POST,
                                              inferDocumentationPath(modelPath) });
        }

        DBG_AND_LOG("GradioClient::makePOSTRequest: Received status code \""
                    << String(statusCode) << "\" and response \""
                    << toPrintableHeaders(responseHeaders.getDescription()) << "\".");

        if (statusCode != 200)
        {
            return OpResult::fail(HttpError { HttpError::Type::BadStatusCode,
                                              HttpError::Request::POST,
                                              inferDocumentationPath(modelPath),
                                              statusCode });
        }

        response = stream->readEntireStreamAsString();

        return OpResult::ok();
    }

    OpResult makeGETRequest(const String modelPath,
                            const String functionName,
                            const String eventID,
                            String& response,
                            const int timeoutMs = 10000)
    {
        URL endpoint = getEndpoint(inferEndpointPath(modelPath), functionName).getChildURL(eventID);

        String requestHeaders = getCommonHeaders();

        DBG_AND_LOG("GradioClient::makeGETRequest: Attempting to make GET request for endpoint \""
                    << endpoint.toString(true) << "\" with headers \""
                    << toPrintableHeaders(requestHeaders) << "\".");

        if (! endpoint.isWellFormed())
        {
            return OpResult::fail(HttpError {
                HttpError::Type::InvalidURL, HttpError::Request::GET, inferDocumentationPath(modelPath) });
        }

        int statusCode = 0;
        StringPairArray responseHeaders;

        auto options = URL::InputStreamOptions(URL::ParameterHandling::inAddress)
                           .withExtraHeaders(requestHeaders)
                           .withConnectionTimeoutMs(timeoutMs)
                           .withResponseHeaders(&responseHeaders)
                           .withStatusCode(&statusCode)
                           .withNumRedirectsToFollow(5);

        std::unique_ptr<InputStream> stream(endpoint.createInputStream(options));

        if (stream == nullptr)
        {
            return OpResult::fail(HttpError { HttpError::Type::ConnectionFailed,
                                              HttpError::Request::GET,
                                              inferDocumentationPath(modelPath) });
        }

        DBG_AND_LOG("GradioClient::makeGETRequest: Received status code \""
                    << String(statusCode) << "\" and response \""
                    << toPrintableHeaders(responseHeaders.getDescription()) << "\".");

        if (statusCode != 200)
        {
            return OpResult::fail(HttpError { HttpError::Type::BadStatusCode,
                                              HttpError::Request::GET,
                                              inferDocumentationPath(modelPath),
                                              statusCode });
        }

        while (! stream->isExhausted())
        {
            response = stream->readNextLine();

            DBG_AND_LOG("GradioClient::makeGETRequest: Streamed response \"" << response << "\".");

            if (response.containsIgnoreCase(enumToString(GradioEvents::Complete)))
            {
                response = extractPayLoad(stream->readNextLine());

                DBG_AND_LOG("GradioClient::makeGETRequest: Final response \"" << response << "\".");

                break;
            }
            else if (response.containsIgnoreCase(enumToString(GradioEvents::Error)))
            {
                response = stream->readNextLine();

                DBG_AND_LOG("GradioClient::makeGETRequest: Error response \"" << response << "\".");

                return OpResult::fail(
                    GradioError { GradioError::Type::RuntimeError, endpoint.toString(true) });
            }
            else
            {
                // TODO - what other information is available?
                // Informational or progress events
                // Examples:
                // - event: heartbeat
                // - event: log
                // - event: progress
            }
        }

        return OpResult::ok();
    }

    String extractPayLoad(String response)
    {
        String payload = response.trim();

        if (payload.startsWith("data:"))
        {
            payload = payload.fromFirstOccurrenceOf("data:", false, false).trim();
        }

        return payload;
    }
};
