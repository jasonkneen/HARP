#pragma once

#include <BinaryData.h>

#include "../../Client.h"

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
        else if (isValidTextToAudioPath(modelPath))
        {
            hostSlashModel = "stability/audio-to-audio";
        }
        else
        {
            // TODO - handle error case (invalid Stability AI path)
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
        else if (isValidTextToAudioPath(modelPath))
        {
            endpointURL = "https://api.stability.ai/v2beta/audio/stable-audio-2/audio-to-audio";
        }
        else
        {
            // TODO - handle error case (invalid Stability AI path)
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
            // TODO - handle error case (invalid Stability AI path)
        }

        return documentationURL;
    }

    String queryControls(String modelPath)
    {
        const char* jsonData;
        int jsonDataSize = 0;

        if (isValidTextToAudioPath(modelPath))
        {
            // Access binarized JSON for text-to-audio controls
            jsonData = BinaryData::texttoaudio_json;
            jsonDataSize = BinaryData::texttoaudio_jsonSize;
        }
        else if (isValidAudioToAudioPath(modelPath))
        {
            // Access binarized JSON for audio-to-audio controls
            jsonData = BinaryData::audiotoaudio_json;
            jsonDataSize = BinaryData::audiotoaudio_jsonSize;
        }
        else
        {
            // TODO - handle error case (invalid Stability AI path)
        }

        String responseJSON = String::fromUTF8(jsonData, jsonDataSize);

        if (responseJSON.isEmpty())
        {
            // TODO - handle error case (failed to read controls JSON from resource file)
        }

        return responseJSON;
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

        return array.size() == 2 && array[0] == "stability" && array[1] == "text-to-audio";
    }

    static bool isValidLongTextToAudioPath(String modelPath)
    {
        /*
          i.e., "https://api.stability.ai/v2beta/audio/stable-audio-2/text-to-audio"
        */

        return modelPath.fromFirstOccurrenceOf(
                   "https://api.stability.ai/v2beta/audio/stable-audio-2/", false, false)
               == "text-to-audio";
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

        return array.size() == 2 && array[0] == "stability" && array[1] == "audio-to-audio";
    }

    static bool isValidLongAudioToAudioPath(String modelPath)
    {
        /*
          i.e., "https://api.stability.ai/v2beta/audio/stable-audio-2/audio-to-audio"
        */

        return modelPath.fromFirstOccurrenceOf(
                   "https://api.stability.ai/v2beta/audio/stable-audio-2/", false, false)
               == "text-to-audio";
    }
};
