#pragma once

#include <string>
#include <vector>

#include <juce_core/juce_core.h>

#include "client/Client.h"

#include "utils/Clients.h"

using namespace juce;

enum ModelStatus
{
    EMPTY,

    QUERYING_CONTROLS,
    PROCESSING,
    CANCELING,

    READY,

    // TODO - are below statuses necessary?
    STARTING,
    SENDING,

    LOADING_ERROR, // TODO - synonymous with EMPTY?
    PROCESSING_ERROR // TODO - synonymous with READY?
};

struct ModelMetadata
{
    std::string name;
    std::string author;
    std::string description;

    std::vector<std::string> tags;

    bool isEmpty()
    {
        // TODO
    }
};

class Model
{
public:
    Model()
    {
        // TODO

        resetState();
    }

    //~Model() override {} // TODO

    bool isEmpty() { return metadata.isEmpty(); }
    bool isLoaded() { return ! isEmpty(); }

    void setStatus(ModelStatus newStatus)
    {
        status = newStatus;

        // TODO - update statusBox each time
    }

    ModelMetadata getMetadata() { return metadata; }

    void resetState()
    {
        metadata = ModelMetadata {};

        setStatus(ModelStatus::EMPTY);
    }

    void loadPath(String pathToLoad)
    {
        // Determine and initialize appropriate client for selected path
        std::unique_ptr<Client> tempClient = multiplexClients(pathToLoad);

        if (! tempClient)
        {
            // TODO - handle error case: unknown client
        }

        // TODO - link access token if available

        setStatus(ModelStatus::QUERYING_CONTROLS);

        String endpointURL = tempClient->inferEndpointURL(pathToLoad);

        if (endpointURL.isEmpty())
        {
            // TODO - handle error case: invalid path
        }

        String responseJSON = tempClient->queryControls(endpointURL);

        // TODO - extract / populate model based on JSON

        // TODO - going to need clearControls() function when loading a new model and model already loaded
        //        - actually notion of resetModel() would be better to clear everything (status / metadata / controls / etc.)

        // TODO - if successful replace existing client (and set path field)
    }

private:
    /*void extractControlsFromJSON(String responseJSON)
    {
        // TODO - parameter(s)

        // Create an Error object in case we need it for the next steps
        Error error;
        error.type = ErrorType::JsonParseError;
        // Parse the extracted JSON string
        var parsedData;
        JSON::parse(responseData, parsedData);

        if (! parsedData.isObject())
        {
            error.devMessage = "Failed to parse the data portion of the received controls JSON.";
            return OpResult::fail(error);
        }

        if (! parsedData.isArray())
        {
            error.devMessage = "Parsed JSON is not an array.";
            return OpResult::fail(error);
        }
        Array<var>* dataArray = parsedData.getArray();
        if (dataArray == nullptr)
        {
            error.devMessage = "Parsed JSON is not an array 2.";
            return OpResult::fail(error);
        }
        // Check if the first element in the array is a dict
        DynamicObject* obj = dataArray->getFirst().getDynamicObject();
        if (obj == nullptr)
        {
            error.devMessage = "First element in the array is not a dict.";
            return OpResult::fail(error);
        }

        // Get the card and controls objects from the parsed data
        DynamicObject* cardObj = obj->getProperty("card").getDynamicObject();

        if (cardObj == nullptr)
        {
            error.devMessage = "Couldn't load the modelCard dict from the controls response.";
            return OpResult::fail(error);
        }

        // Clear the existing properties in cardDict
        cardDict.clear();

        // Copy all properties from cardObj to cardDict
        for (auto& key : cardObj->getProperties())
        {
            cardDict.setProperty(key.name, key.value);
        }

        Array<var>* inputsArray = obj->getProperty("inputs").getArray();
        if (inputsArray == nullptr)
        {
            error.devMessage = "Couldn't load the controls array/list from the controls response.";
            return OpResult::fail(error);
        }
        inputComponents = *inputsArray;

        Array<var>* outputsArray = obj->getProperty("outputs").getArray();
        if (outputsArray == nullptr)
        {
            error.devMessage = "Couldn't load the controls array/list from the controls response.";
            return OpResult::fail(error);
        }
        outputComponents = *outputsArray;

        // TODO - return value
    }*/

    ModelStatus status; // TODO - control flow shouldn't depend on status

    String currentPath;

    ModelMetadata metadata;

    // TODO - subject to change
    //ComponentInfoList controlsInfo;

    //ComponentInfoList inputTracksInfo;
    //ComponentInfoList outputTracksInfo;
};
