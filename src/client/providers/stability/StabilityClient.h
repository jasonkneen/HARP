#pragma once

#include <BinaryData.h>

#include "../../Client.h"

#include "../../../utils/Errors.h"

using namespace juce;

class StabilityClient : public Client
{
public:
    StabilityClient()
    {
        // TODO
    }

    //~StabilityClient() override {} // TODO

    static bool matchesPathSpec(String modelPath)
    {
        return isValidTextToAudioPath(modelPath) || isValidAudioToAudioPath(modelPath);
    }

    String inferHostSlashModel(String modelPath) override
    {
        String hostSlashModel;

        if (isValidTextToAudioPath(modelPath))
        {
            hostSlashModel = "stability/text-to-audio";
        }
        else if (isValidAudioToAudioPath(modelPath))
        {
            hostSlashModel = "stability/audio-to-audio";
        }
        else
        {
            DBG_AND_LOG("StabilityClient::inferHostSlashModel: Path \""
                        << modelPath << "\" does not match valid specification for Stability AI.");
        }

        return hostSlashModel;
    }

    String inferEndpointURL(String modelPath) override
    {
        String endpointURL;

        if (isValidTextToAudioPath(modelPath))
        {
            endpointURL = "https://api.stability.ai/v2beta/audio/stable-audio-2/text-to-audio";
        }
        else if (isValidAudioToAudioPath(modelPath))
        {
            endpointURL = "https://api.stability.ai/v2beta/audio/stable-audio-2/audio-to-audio";
        }
        else
        {
            DBG_AND_LOG("StabilityClient::inferEndpointURL: Path \""
                        << modelPath << "\" does not match valid specification for Stability AI.");
        }

        return endpointURL;
    }

    String inferDocumentationURL(String modelPath) override
    {
        String documentationURL;

        if (isValidTextToAudioPath(modelPath))
        {
            documentationURL =
                "https://platform.stability.ai/docs/api-reference#tag/Stable-Audio-2/"
                "paths/~1v2beta~1audio~1stable-audio-2~1text-to-audio/post";
        }
        else if (isValidAudioToAudioPath(modelPath))
        {
            documentationURL =
                "https://platform.stability.ai/docs/api-reference#tag/Stable-Audio-2/"
                "paths/~1v2beta~1audio~1stable-audio-2~1audio-to-audio/post";
        }
        else
        {
            DBG_AND_LOG("StabilityClient::inferDocumentationURL: Path \""
                        << modelPath << "\" does not match valid specification for Stability AI.");
        }

        return documentationURL;
    }

    OpResult queryControls(String modelPath, String& queryResponse)
    {
        const char* jsonData;
        int jsonDataSize = 0;

        if (isValidTextToAudioPath(modelPath))
        {
            DBG_AND_LOG("StabilityClient::queryControls: Reading text-to-audio.json.");

            // Access binarized JSON for text-to-audio controls
            jsonData = BinaryData::texttoaudio_json;
            jsonDataSize = BinaryData::texttoaudio_jsonSize;
        }
        else if (isValidAudioToAudioPath(modelPath))
        {
            DBG_AND_LOG("StabilityClient::queryControls: Reading audio-to-audio.json.");

            // Access binarized JSON for audio-to-audio controls
            jsonData = BinaryData::audiotoaudio_json;
            jsonDataSize = BinaryData::audiotoaudio_jsonSize;
        }
        else
        {
            ClientError error { ClientError::Type::InvalidModelPath, modelPath, "Stability AI" };

            DBG_AND_LOG("StabilityClient::queryControls: " << toUserMessage(error));

            return OpResult::fail(error);
        }

        queryResponse = String::fromUTF8(jsonData, jsonDataSize);

        if (queryResponse.isEmpty())
        {
            DBG_AND_LOG("StabilityClient::queryControls: JSON response is empty.");

            // TODO - should this be a failure? does this even ever happen?
        }

        return OpResult::ok();
    }

private:
    static bool isValidTextToAudioPath(String modelPath)
    {
        return isValidShortTextToAudioPath(modelPath) || isValidLongTextToAudioPath(modelPath);
    }

    static bool isValidShortTextToAudioPath(String modelPath)
    {
        /*
          i.e., "stability/text-to-audio"
        */

        StringArray array = StringArray::fromTokens(modelPath, "/", "");

        return array.size() == 2 && array[0].equalsIgnoreCase("stability")
               && array[1].equalsIgnoreCase("text-to-audio");
    }

    static bool isValidLongTextToAudioPath(String modelPath)
    {
        /*
          i.e., "https://api.stability.ai/v2beta/audio/stable-audio-2/text-to-audio"
        */

        return modelPath
            .fromFirstOccurrenceOf(
                "https://api.stability.ai/v2beta/audio/stable-audio-2/", false, false)
            .equalsIgnoreCase("text-to-audio");
    }

    static bool isValidAudioToAudioPath(String modelPath)
    {
        return isValidShortAudioToAudioPath(modelPath) || isValidLongAudioToAudioPath(modelPath);
    }

    static bool isValidShortAudioToAudioPath(String modelPath)
    {
        /*
          i.e., "stability/audio-to-audio"
        */

        StringArray array = StringArray::fromTokens(modelPath, "/", "");

        return array.size() == 2 && array[0].equalsIgnoreCase("stability")
               && array[1].equalsIgnoreCase("audio-to-audio");
    }

    static bool isValidLongAudioToAudioPath(String modelPath)
    {
        /*
          i.e., "https://api.stability.ai/v2beta/audio/stable-audio-2/audio-to-audio"
        */

        return modelPath
            .fromFirstOccurrenceOf(
                "https://api.stability.ai/v2beta/audio/stable-audio-2/", false, false)
            .equalsIgnoreCase("audio-to-audio");
    }
};
