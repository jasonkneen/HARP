#pragma once

#include "Client.h"

#include "../utils/Enums.h"

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
        provider = Provider::HuggingFace;

        tokenValidationURL = URL("https://huggingface.co/api/whoami-v2");
        tokenRegistrationURL = URL("https://huggingface.co/settings/tokens");
    }

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

                // TODO - this can load paths that were incorrectly added with "-" instead of "_" resulting in a broken documentation link
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
                documentationPath =
                    "https://huggingface.co/spaces/" + inferHostSlashModel(modelPath);
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

    virtual OpResult validateToken(const String& tokenToValidate) override
    {
        String responseJSON;

        OpResult result = queryToken(tokenToValidate, responseJSON);

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

        auto* tokenJSON = responseDict->getProperty("auth")
                              .getDynamicObject()
                              ->getProperty("accessToken")
                              .getDynamicObject();

        String role = tokenJSON->getProperty("role").toString();

        if (! (role == "write" || role == "read"))
        {
            bool hasAllPermissions = false;

            auto* scopedArray = tokenJSON->getProperty("fineGrained")
                                    .getDynamicObject()
                                    ->getProperty("scoped")
                                    .getArray();

            for (const auto& scopeEntry : *scopedArray)
            {
                if (! scopeEntry.isObject())
                    continue;

                var permissionsVar = scopeEntry.getDynamicObject()->getProperty("permissions");

                if (! permissionsVar.isArray())
                    continue;

                auto* permissionsArray = permissionsVar.getArray();
                bool hasAll = permissionsArray->contains("repo.content.read")
                              && permissionsArray->contains("repo.write")
                              && permissionsArray->contains("inference.serverless.write")
                              && permissionsArray->contains("inference.endpoints.infer.write");

                if (hasAll)
                {
                    hasAllPermissions = true;
                    break;
                }
            }

            if (! hasAllPermissions)
            {
                return OpResult::fail(ClientError { ClientError::Type::InsufficientPermissions,
                                                    "",
                                                    "Hugging Face",
                                                    tokenToValidate });
            }
        }

        return OpResult::ok();
    }

    OpResult makeRequest(const String modelPath,
                         const String requestType,
                         const String body,
                         Array<var>& output)
    {
        URL endpoint = URL(inferEndpointPath(modelPath))
                           .getChildURL("gradio_api")
                           .getChildURL("call")
                           .getChildURL(requestType);

        String responseJSON;

        OpResult result = makePOSTRequest(
            endpoint, getJSONHeaders(), body, responseJSON, inferDocumentationPath(modelPath));

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

        endpoint = URL(inferEndpointPath(modelPath))
                       .getChildURL("gradio_api")
                       .getChildURL("call")
                       .getChildURL(requestType)
                       .getChildURL(eventID);

        responseJSON.clear();

        result = makeGETRequest(endpoint, responseJSON, inferDocumentationPath(modelPath));

        if (result.failed())
        {
            return result;
        }

        result = stringJSONToList(responseJSON, output);

        if (result.failed())
        {
            return result;
        }

        if (output.isEmpty())
        {
            return OpResult::fail(JsonError { JsonError::Type::Empty, {} });
        }

        return OpResult::ok();
    }

    OpResult queryControls(String modelPath, DynamicObject::Ptr& controls)
    {
        Array<var> dataList;

        OpResult result = makeRequest(modelPath, "controls", emptyJSONBody, dataList);

        if (result.failed())
        {
            return result;
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

    OpResult uploadFile(String modelPath, const File& fileToUpload, String& remoteFilePath) override
    {
        URL endpoint =
            URL(inferEndpointPath(modelPath)).getChildURL("gradio_api").getChildURL("upload");

        String requestBody; // Empty
        String responseJSON;

        OpResult result = makePOSTRequest(endpoint,
                                          getCommonHeaders(),
                                          requestBody,
                                          responseJSON,
                                          inferDocumentationPath(modelPath),
                                          fileToUpload);

        if (result.failed())
        {
            return result;
        }

        Array<var> responseArray;

        result = stringJSONToList(responseJSON, responseArray);

        if (result.failed())
        {
            return result;
        }

        remoteFilePath = responseArray.getFirst().toString();

        if (remoteFilePath.isEmpty())
        {
            return OpResult::fail(
                FileError { FileError::Type::UploadFailed, fileToUpload.getFullPathName() });
        }

        return OpResult::ok();
    }

    OpResult downloadFile(String downloadPath, File& fileToDownload) override
    {
        // Obtain local temporary directory for downloaded file
        File tempDir = File::getSpecialLocation(File::tempDirectory);

        URL endpoint = URL(downloadPath);
        String fileName = endpoint.getFileName();
        String baseName =
            File::createFileWithoutCheckingPath(fileName).getFileNameWithoutExtension();

        String extension = File::createFileWithoutCheckingPath(fileName).getFileExtension();

        // Create file at a unique path
        fileToDownload = tempDir.getChildFile(baseName + "_" + Uuid().toString() + extension);

        OpResult result = makeGETRequest(endpoint, fileToDownload, downloadPath);

        if (result.failed())
        {
            return result;
        }

        return OpResult::ok();
    }

    var wrapPayloadElement(var payloadElement, bool isFile = false) override
    {
        if (isFile and ! payloadElement.isVoid())
        {
            DynamicObject::Ptr wrappedPayloadElement = payloadElement.getDynamicObject();

            DynamicObject::Ptr meta = new DynamicObject();

            meta->setProperty("_type", var("gradio.FileData"));
            wrappedPayloadElement->setProperty("meta", var(meta));

            return var(wrappedPayloadElement);
        }
        else
        {
            return payloadElement;
        }
    }

    OpResult process(String modelPath,
                     String& payloadJSON,
                     std::vector<File>& outputFiles,
                     LabelList& labels)
    {
        Array<var> dataList;

        OpResult result = makeRequest(modelPath, "process", payloadJSON, dataList);

        if (result.failed())
        {
            return result;
        }

        for (int i = 0; i < dataList.size(); i++)
        {
            var outputVar = dataList.getReference(i);

            if (! outputVar.isObject())
            {
                return OpResult::fail(
                    JsonError { JsonError::Type::NotADictionary, outputVar.toString() });
            }

            DynamicObject* outputDict = outputVar.getDynamicObject();

            static const Identifier metadataKey { "meta" };

            if (outputDict->hasProperty(metadataKey))
            {
                var metadata = outputDict->getProperty(metadataKey);

                if (! metadata.isObject())
                {
                    return OpResult::fail(
                        JsonError { JsonError::Type::NotADictionary, metadata.toString() });
                }

                static const Identifier typeKey { "_type" };

                String outputType = metadata.getDynamicObject()->getProperty(typeKey).toString();

                if (outputType == "gradio.FileData")
                {
                    File outputFile;

                    String remotePath = outputDict->getProperty("url").toString();

                    result = downloadFile(remotePath, outputFile);

                    if (result.failed())
                    {
                        return result;
                    }

                    outputFiles.push_back(outputFile);
                }
                else if (outputType == "pyharp.LabelList")
                {
                    // TODO
                }
                else
                {
                    // TODO - handle error (unknown output type)
                }
            }
            else
            {
                return OpResult::fail(JsonError { JsonError::Type::MissingKey,
                                                  JSON::toString(var(outputDict), true),
                                                  metadataKey.toString() });
            }
        }

        ignoreUnused(labels);

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

    OpResult makePOSTRequest(URL endpoint,
                             const String headers,
                             const String body,
                             String& response,
                             const String errorPath = "",
                             const File& fileToUpload = File(),
                             const int timeoutMs = 10000)
    {
        String debugMessage =
            "GradioClient::makePOSTRequest: Attempting to make POST request for endpoint \""
            + endpoint.toString(true) + "\" with headers \"" + toPrintableHeaders(headers);

        if (body.isNotEmpty())
        {
            endpoint = endpoint.withPOSTData(body);

            debugMessage += "\" and body \"" + body;
        }

        if (fileToUpload.existsAsFile())
        {
            endpoint = endpoint.withFileToUpload("files", fileToUpload, "audio/midi");

            debugMessage += "\" and file \"" + fileToUpload.getFullPathName();
        }

        debugMessage += "\".";

        DBG_AND_LOG(debugMessage);

        if (! endpoint.isWellFormed())
        {
            return OpResult::fail(
                HttpError { HttpError::Type::InvalidURL, HttpError::Request::POST, errorPath });
        }

        int statusCode = 0;
        StringPairArray responseHeaders;

        auto options = URL::InputStreamOptions(URL::ParameterHandling::inPostData)
                           .withExtraHeaders(headers)
                           .withConnectionTimeoutMs(timeoutMs)
                           .withResponseHeaders(&responseHeaders)
                           .withStatusCode(&statusCode)
                           .withNumRedirectsToFollow(5)
                           .withHttpRequestCmd("POST");

        std::unique_ptr<InputStream> stream(endpoint.createInputStream(options));

        if (stream == nullptr)
        {
            return OpResult::fail(HttpError {
                HttpError::Type::ConnectionFailed, HttpError::Request::POST, errorPath });
        }

        DBG_AND_LOG("GradioClient::makePOSTRequest: Received status code \""
                    << String(statusCode) << "\" and response \""
                    << toPrintableHeaders(responseHeaders.getDescription()) << "\".");

        if (statusCode != 200)
        {
            return OpResult::fail(HttpError {
                HttpError::Type::BadStatusCode, HttpError::Request::POST, errorPath, statusCode });
        }

        response = stream->readEntireStreamAsString();

        return OpResult::ok();
    }

    OpResult makeGETRequestStream(const URL endpoint,
                                  std::unique_ptr<InputStream>& stream,
                                  const String errorPath = "",
                                  const int timeoutMs = 10000)
    {
        String requestHeaders = getCommonHeaders();

        DBG_AND_LOG(
            "GradioClient::makeGETRequestStream: Attempting to make GET request for endpoint \""
            << endpoint.toString(true) << "\" with headers \"" << toPrintableHeaders(requestHeaders)
            << "\".");

        if (! endpoint.isWellFormed())
        {
            return OpResult::fail(
                HttpError { HttpError::Type::InvalidURL, HttpError::Request::GET, errorPath });
        }

        int statusCode = 0;
        StringPairArray responseHeaders;

        auto options = URL::InputStreamOptions(URL::ParameterHandling::inAddress)
                           .withExtraHeaders(requestHeaders)
                           .withConnectionTimeoutMs(timeoutMs)
                           .withResponseHeaders(&responseHeaders)
                           .withStatusCode(&statusCode)
                           .withNumRedirectsToFollow(5);

        stream = endpoint.createInputStream(options);

        if (stream == nullptr)
        {
            return OpResult::fail(HttpError {
                HttpError::Type::ConnectionFailed, HttpError::Request::GET, errorPath });
        }

        DBG_AND_LOG("GradioClient::makeGETRequestStream: Received status code \""
                    << String(statusCode) << "\" and response \""
                    << toPrintableHeaders(responseHeaders.getDescription()) << "\".");

        if (statusCode != 200)
        {
            return OpResult::fail(HttpError {
                HttpError::Type::BadStatusCode, HttpError::Request::GET, errorPath, statusCode });
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

    OpResult makeGETRequest(const URL endpoint,
                            String& response,
                            const String errorPath = "",
                            const int timeoutMs = 10000)
    {
        std::unique_ptr<InputStream> stream;

        OpResult result = makeGETRequestStream(endpoint, stream, errorPath, timeoutMs);

        if (result.failed())
        {
            return result;
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

    OpResult makeGETRequest(const URL endpoint,
                            File& fileToDownload,
                            const String errorPath = "",
                            const int timeoutMs = 10000)
    {
        std::unique_ptr<InputStream> stream;

        OpResult result = makeGETRequestStream(endpoint, stream, errorPath, timeoutMs);

        // Remove file at target path if one already exists
        fileToDownload.deleteFile();

        // Create output stream to save file locally
        std::unique_ptr<FileOutputStream> outputFileStream(fileToDownload.createOutputStream());

        if (outputFileStream == nullptr || ! outputFileStream->openedOk())
        {
            return OpResult::fail(
                FileError { FileError::Type::DownloadFailed, endpoint.toString(true) });
        }

        // Copy data from GET request stream to stream for output file
        outputFileStream->writeFromInputStream(*stream, stream->getTotalLength());

        return OpResult::ok();
    }
};
