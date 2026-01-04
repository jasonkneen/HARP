#pragma once

#include <string>
#include <vector>

#include <juce_core/juce_core.h>

#include "client/Client.h"

#include "utils/Clients.h"
#include "utils/Controls.h"
#include "utils/Errors.h"
#include "utils/Logging.h"

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

    ModelMetadata() {}

    ModelMetadata(DynamicObject* card)
    {
        // TODO - check that the following properties are of the correct type

        if (card->hasProperty("name"))
        {
            name = card->getProperty("name").toString().toStdString();
        }
        if (card->hasProperty("author"))
        {
            author = card->getProperty("author").toString().toStdString();
        }
        if (card->hasProperty("description"))
        {
            description = card->getProperty("description").toString().toStdString();
        }
        if (card->hasProperty("tags"))
        {
            Array<var>* newTags = card->getProperty("tags").getArray();

            if (newTags == nullptr)
            {
                // TODO - handle error case: failed to parse tags
            }
            else
            {
                for (int i = 0; i < newTags->size(); i++)
                {
                    tags.push_back(newTags->getReference(i).toString().toStdString());
                }
            }
        }
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

    //bool isEmpty() { return metadata.isEmpty(); }
    //bool isLoaded() { return ! isEmpty(); }

    void setStatus(ModelStatus newStatus)
    {
        status = newStatus;

        // TODO - update statusBox each time
    }

    ModelMetadata getMetadata() { return metadata; }

    void resetState()
    {
        metadata = ModelMetadata {};

        // TODO - clear controls / tracks

        // TODO - clear client

        currentlyLoadedPath.clear();

        setStatus(ModelStatus::EMPTY);
    }

    OpResult loadPath(String pathToLoad)
    {
        // Initialize result for error handling
        OpResult result = OpResult::ok();

        // Create new client for querying
        std::unique_ptr<Client> tempClient;

        // Initialize appropriate client for selected path
        result = multiplexClients(pathToLoad, tempClient);

        if (result.failed())
        {
            return result;
        }

        // TODO - link access token if available

        setStatus(ModelStatus::QUERYING_CONTROLS);

        // Initialize empty string to hold query response
        String queryResponse;

        // Obtain JSON string corresponding to controls + track layout
        result = tempClient->queryControls(pathToLoad, queryResponse);

        if (result.failed())
        {
            setStatus(ModelStatus::EMPTY);

            return result;
        }

        // Attempt to parse JSON response and update controls + track layout
        result = populateControlsFromJSON(queryResponse);

        if (result.failed())
        {
            setStatus(ModelStatus::EMPTY);

            return result;
        }

        // Replace existing client
        client = std::move(tempClient);
        // Keep track of loaded path
        currentlyLoadedPath = pathToLoad;

        setStatus(ModelStatus::READY);

        return result;
    }

private:
    // TODO - going to need clearControls() function when loading a new model and model already loaded
    //        - actually notion of resetModel() would be better to clear everything (status / metadata / controls / etc.)
    // TODO - does anything special need to be done to free memory?
    void setModelMetadata(ModelMetadata newMetadata) { metadata = newMetadata; }

    void setControls(ModelComponentInfoList newControls) { controlComponents = newControls; }

    void setInputs(ModelComponentInfoList newInputs) { inputTrackComponents = newInputs; }

    void setOutputs(ModelComponentInfoList newOutputs) { outputTrackComponents = newOutputs; }

    OpResult populateControlsFromJSON(String responseJSON)
    {
        // Initialize result for error handling
        OpResult result = OpResult::ok();

        var parsedData;

        Result parseResult = JSON::parse(responseJSON, parsedData);

        if (parseResult.failed() || ! parsedData.isObject())
        {
            // TODO - handle error case: failed to parse controls
        }

        if (! parsedData.isArray())
        {
            // TODO - handle error case: parsed data is not an array
        }

        //Array<var>* dataArray = parsedData.getArray();

        /*
        if (dataArray == nullptr)
        {
            // TODO - handle error case: failed to parse controls
        }
        */

        // Verify first element of array is a dict
        DynamicObject* dict = parsedData.getDynamicObject();

        if (dict == nullptr)
        {
            // TODO - handle error case: failed to parse controls
        }

        /*
        // Verify first element of array is a dict
        DynamicObject* firstElement = dataArray->getFirst().getDynamicObject();

        if (firstElement == nullptr)
        {
            // TODO - handle error case: first element is not a dict
        }
        */

        // Extract metadata from parsed JSON
        DynamicObject* card = dict->getProperty("card").getDynamicObject();

        if (card == nullptr)
        {
            // TODO - handle error case: couldn't load metadata
        }

        ModelMetadata newMetadata = ModelMetadata(card);

        Array<var>* inputComponents = dict->getProperty("inputs").getArray();

        if (inputComponents == nullptr)
        {
            // TODO - handle error case: couldn't load inputs
        }

        ModelComponentInfoList newInputs;
        ModelComponentInfoList newControls;

        for (int i = 0; i < inputComponents->size(); i++)
        {
            DynamicObject* input = inputComponents->getReference(i).getDynamicObject();

            if (dict == nullptr)
            {
                // TODO - handle error case: failed to read control
            }
            else if (input->hasProperty("type"))
            {
                String type = input->getProperty("type").toString().toStdString();

                if (type == "audio_track")
                {
                    std::shared_ptr<ModelComponentInfo> audioTrack =
                        std::make_shared<AudioTrackComponentInfo>(input);

                    newInputs.push_back(audioTrack);

                    DBG_AND_LOG("Model::extractControlsFromJSON: Audio track input \""
                                + audioTrack->label + "\" extracted.");
                }
                else if (type == "midi_track")
                {
                    std::shared_ptr<ModelComponentInfo> midiTrack =
                        std::make_shared<MidiTrackComponentInfo>(input);

                    newInputs.push_back(midiTrack);

                    DBG_AND_LOG("Model::extractControlsFromJSON: MIDI track input \""
                                + midiTrack->label + "\" extracted.");
                }
                else if (type == "text_box")
                {
                    std::shared_ptr<ModelComponentInfo> textControl =
                        std::make_shared<TextBoxComponentInfo>(input);

                    newControls.push_back(textControl);

                    DBG_AND_LOG("Model::extractControlsFromJSON: Text control \""
                                + textControl->label + "\" extracted.");
                }
                else if (type == "number_box")
                {
                    std::shared_ptr<ModelComponentInfo> numberControl =
                        std::make_shared<NumberBoxComponentInfo>(input);

                    newControls.push_back(numberControl);

                    DBG_AND_LOG("Model::extractControlsFromJSON: Number box control \""
                                + numberControl->label + "\" extracted.");
                }
                else if (type == "toggle")
                {
                    std::shared_ptr<ModelComponentInfo> toggleControl =
                        std::make_shared<ToggleComponentInfo>(input);

                    newControls.push_back(toggleControl);

                    DBG_AND_LOG("Model::extractControlsFromJSON: Toggle control \""
                                + toggleControl->label + "\" extracted.");
                }
                else if (type == "slider")
                {
                    std::shared_ptr<ModelComponentInfo> sliderControl =
                        std::make_shared<SliderComponentInfo>(input);

                    newControls.push_back(sliderControl);

                    DBG_AND_LOG("Model::extractControlsFromJSON: Slider control \""
                                + sliderControl->label + "\" extracted.");
                }
                else if (type == "dropdown")
                {
                    std::shared_ptr<ModelComponentInfo> dropdownControl =
                        std::make_shared<ComboBoxComponentInfo>(input);

                    newControls.push_back(dropdownControl);

                    DBG_AND_LOG("Model::extractControlsFromJSON: Dropdown control \""
                                + dropdownControl->label + "\" extracted.");
                }
                else
                {
                    // TODO - handle error case: unknown control
                }
            }
            else
            {
                // TODO - handle error case: ambiguous input
            }
        }

        Array<var>* outputComponents = dict->getProperty("outputs").getArray();

        if (outputComponents == nullptr)
        {
            // TODO - handle error case: couldn't load outputs
        }

        ModelComponentInfoList newOutputs;

        for (int i = 0; i < outputComponents->size(); i++)
        {
            DynamicObject* output = outputComponents->getReference(i).getDynamicObject();

            if (dict == nullptr)
            {
                // TODO - handle error case: failed to read control
            }
            else if (output->hasProperty("type"))
            {
                String type = output->getProperty("type").toString().toStdString();

                if (type == "audio_track")
                {
                    std::shared_ptr<ModelComponentInfo> audioTrack =
                        std::make_shared<AudioTrackComponentInfo>(output);

                    newOutputs.push_back(audioTrack);

                    DBG_AND_LOG("Model::extractControlsFromJSON: Audio track output \""
                                + audioTrack->label + "\" extracted.");
                }
                else if (type == "midi_track")
                {
                    std::shared_ptr<ModelComponentInfo> midiTrack =
                        std::make_shared<MidiTrackComponentInfo>(output);

                    newOutputs.push_back(midiTrack);

                    DBG_AND_LOG("Model::extractControlsFromJSON: MIDI track output \""
                                + midiTrack->label + "\" extracted.");
                }
                else
                {
                    // TODO - handle error case: unknown control
                }
            }
            else
            {
                // TODO - handle error case: ambiguous output
            }
        }

        // Update model information if all loading operations are successful
        setModelMetadata(newMetadata);
        setControls(newControls);
        setInputs(newInputs);
        setOutputs(newOutputs);

        return result;
    }

    ModelStatus status; // TODO - control flow shouldn't depend on status

    std::unique_ptr<Client> client;

    String currentlyLoadedPath;

    ModelMetadata metadata;

    ModelComponentInfoList controlComponents;

    ModelComponentInfoList inputTrackComponents;
    ModelComponentInfoList outputTrackComponents;
};
