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
        // Create new client for querying
        std::unique_ptr<Client> tempClient;

        // Initialize appropriate client for selected path
        OpResult result = multiplexClients(pathToLoad, tempClient);

        if (result.failed())
        {
            return result;
        }

        // TODO - link access token if available

        setStatus(ModelStatus::QUERYING_CONTROLS);

        // Initialize empty dictionary to hold query response
        DynamicObject::Ptr controls;

        // Obtain JSON string corresponding to controls + track layout
        result = tempClient->queryControls(pathToLoad, controls);

        if (result.failed())
        {
            setStatus(ModelStatus::EMPTY);

            return result;
        }

        ModelMetadata newMetadata;

        // Attempt to extract metadata from JSON
        result = extractMetadata(controls, newMetadata);

        if (result.failed())
        {
            setStatus(ModelStatus::EMPTY);

            return result;
        }

        ModelComponentInfoList newInputs;
        ModelComponentInfoList newControls;

        // Attempt to extract inputs and controls from JSON
        result = extractInputs(controls, newInputs, newControls);

        if (result.failed())
        {
            setStatus(ModelStatus::EMPTY);

            return result;
        }

        ModelComponentInfoList newOutputs;

        // Attempt to extract outputs from JSON
        result = extractOutputs(controls, newOutputs);

        if (result.failed())
        {
            setStatus(ModelStatus::EMPTY);

            return result;
        }

        // Update model information if all loading operations are successful
        metadata = newMetadata;
        controlComponents = newControls;
        inputTrackComponents = newInputs;
        outputTrackComponents = newOutputs;

        // Replace existing client
        client = std::move(tempClient);
        // Keep track of loaded path
        currentlyLoadedPath = pathToLoad;

        setStatus(ModelStatus::READY);

        return result;
    }

private:
    OpResult extractMetadata(DynamicObject::Ptr& controls, ModelMetadata& newMetadata)
    {
        if (controls == nullptr)
        {
            return OpResult::fail(JSONError { JSONError::Type::NotADictionary, {} });
        }

        static const Identifier metadataKey { "card" };

        if (! controls->hasProperty(metadataKey))
        {
            return OpResult::fail(JSONError { JSONError::Type::MissingKey,
                                              JSON::toString(var(controls.get()), true),
                                              metadataKey.toString() });
        }

        const var& metadataValue = controls->getProperty(metadataKey);

        if (! metadataValue.isObject())
        {
            return OpResult::fail(
                JSONError { JSONError::Type::NotADictionary, metadataValue.toString() });
        }

        DynamicObject::Ptr metadataDict = metadataValue.getDynamicObject();

        jassert(metadataDict != nullptr);

        newMetadata = ModelMetadata(metadataDict);

        return OpResult::ok();
    }

    OpResult extractInputs(DynamicObject::Ptr& controls,
                           ModelComponentInfoList& newInputs,
                           ModelComponentInfoList& newControls)
    {
        if (controls == nullptr)
        {
            return OpResult::fail(JSONError { JSONError::Type::NotADictionary, {} });
        }

        static const Identifier inputsKey { "inputs" };

        if (! controls->hasProperty(inputsKey))
        {
            return OpResult::fail(JSONError { JSONError::Type::MissingKey,
                                              JSON::toString(var(controls.get()), true),
                                              inputsKey.toString() });
        }

        const var& inputsValue = controls->getProperty(inputsKey);

        if (! inputsValue.isArray())
        {
            return OpResult::fail(JSONError { JSONError::Type::NotAList, inputsValue.toString() });
        }

        Array<var>* inputComponents = inputsValue.getArray();

        for (int i = 0; i < inputComponents->size(); i++)
        {
            var controlsVar = inputComponents->getReference(i);

            if (! controlsVar.isObject())
            {
                return OpResult::fail(
                    JSONError { JSONError::Type::NotADictionary, controlsVar.toString() });
            }

            DynamicObject* controlsDict = controlsVar.getDynamicObject();

            static const Identifier typeKey { "type" };

            if (controlsDict->hasProperty(typeKey))
            {
                String type = controlsDict->getProperty("type").toString().toStdString();

                if (type == "audio_track")
                {
                    std::shared_ptr<ModelComponentInfo> audioTrack =
                        std::make_shared<AudioTrackComponentInfo>(controlsDict);

                    newInputs.push_back(audioTrack);

                    DBG_AND_LOG("Model::extractControlsFromJSON: Audio track input \""
                                + audioTrack->label + "\" extracted.");
                }
                else if (type == "midi_track")
                {
                    std::shared_ptr<ModelComponentInfo> midiTrack =
                        std::make_shared<MidiTrackComponentInfo>(controlsDict);

                    newInputs.push_back(midiTrack);

                    DBG_AND_LOG("Model::extractControlsFromJSON: MIDI track input \""
                                + midiTrack->label + "\" extracted.");
                }
                else if (type == "text_box")
                {
                    std::shared_ptr<ModelComponentInfo> textControl =
                        std::make_shared<TextBoxComponentInfo>(controlsDict);

                    newControls.push_back(textControl);

                    DBG_AND_LOG("Model::extractControlsFromJSON: Text control \""
                                + textControl->label + "\" extracted.");
                }
                else if (type == "number_box")
                {
                    std::shared_ptr<ModelComponentInfo> numberControl =
                        std::make_shared<NumberBoxComponentInfo>(controlsDict);

                    newControls.push_back(numberControl);

                    DBG_AND_LOG("Model::extractControlsFromJSON: Number box control \""
                                + numberControl->label + "\" extracted.");
                }
                else if (type == "toggle")
                {
                    std::shared_ptr<ModelComponentInfo> toggleControl =
                        std::make_shared<ToggleComponentInfo>(controlsDict);

                    newControls.push_back(toggleControl);

                    DBG_AND_LOG("Model::extractControlsFromJSON: Toggle control \""
                                + toggleControl->label + "\" extracted.");
                }
                else if (type == "slider")
                {
                    std::shared_ptr<ModelComponentInfo> sliderControl =
                        std::make_shared<SliderComponentInfo>(controlsDict);

                    newControls.push_back(sliderControl);

                    DBG_AND_LOG("Model::extractControlsFromJSON: Slider control \""
                                + sliderControl->label + "\" extracted.");
                }
                else if (type == "dropdown")
                {
                    std::shared_ptr<ModelComponentInfo> dropdownControl =
                        std::make_shared<ComboBoxComponentInfo>(controlsDict);

                    newControls.push_back(dropdownControl);

                    DBG_AND_LOG("Model::extractControlsFromJSON: Dropdown control \""
                                + dropdownControl->label + "\" extracted.");
                }
                else
                {
                    return OpResult::fail(
                        ControlError { ControlError::Type::UnsupportedControl, type });
                }
            }
            else
            {
                return OpResult::fail(JSONError { JSONError::Type::MissingKey,
                                                  JSON::toString(var(controlsDict), true),
                                                  typeKey.toString() });
            }
        }

        return OpResult::ok();
    }

    OpResult extractOutputs(DynamicObject::Ptr& controls, ModelComponentInfoList& newOutputs)
    {
        if (controls == nullptr)
        {
            return OpResult::fail(JSONError { JSONError::Type::NotADictionary, {} });
        }

        static const Identifier outputsKey { "outputs" };

        if (! controls->hasProperty(outputsKey))
        {
            return OpResult::fail(JSONError { JSONError::Type::MissingKey,
                                              JSON::toString(var(controls.get()), true),
                                              outputsKey.toString() });
        }

        const var& outputsValue = controls->getProperty(outputsKey);

        if (! outputsValue.isArray())
        {
            return OpResult::fail(JSONError { JSONError::Type::NotAList, outputsValue.toString() });
        }

        Array<var>* outputComponents = outputsValue.getArray();

        for (int i = 0; i < outputComponents->size(); i++)
        {
            var controlsVar = outputComponents->getReference(i);

            if (! controlsVar.isObject())
            {
                return OpResult::fail(
                    JSONError { JSONError::Type::NotADictionary, controlsVar.toString() });
            }

            DynamicObject* controlsDict = controlsVar.getDynamicObject();

            static const Identifier typeKey { "type" };

            if (controlsDict->hasProperty("type"))
            {
                String type = controlsDict->getProperty("type").toString().toStdString();

                if (type == "audio_track")
                {
                    std::shared_ptr<ModelComponentInfo> audioTrack =
                        std::make_shared<AudioTrackComponentInfo>(controlsDict);

                    newOutputs.push_back(audioTrack);

                    DBG_AND_LOG("Model::extractControlsFromJSON: Audio track output \""
                                + audioTrack->label + "\" extracted.");
                }
                else if (type == "midi_track")
                {
                    std::shared_ptr<ModelComponentInfo> midiTrack =
                        std::make_shared<MidiTrackComponentInfo>(controlsDict);

                    newOutputs.push_back(midiTrack);

                    DBG_AND_LOG("Model::extractControlsFromJSON: MIDI track output \""
                                + midiTrack->label + "\" extracted.");
                }
                else
                {
                    return OpResult::fail(
                        ControlError { ControlError::Type::UnsupportedControl, type });
                }
            }
            else
            {
                return OpResult::fail(JSONError { JSONError::Type::MissingKey,
                                                  JSON::toString(var(controlsDict), true),
                                                  typeKey.toString() });
            }
        }

        return OpResult::ok();
    }

    ModelStatus status; // TODO - control flow shouldn't depend on status

    std::unique_ptr<Client> client;

    String currentlyLoadedPath;

    ModelMetadata metadata;

    ModelComponentInfoList controlComponents;

    ModelComponentInfoList inputTrackComponents;
    ModelComponentInfoList outputTrackComponents;
};
