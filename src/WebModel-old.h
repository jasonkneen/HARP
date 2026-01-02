/**
 * @file
 * @brief Base class for any models that utilize a web api to process
 * information. Currently we provide an implmentation of a wave 2 wave web based
 * model. We use the GradioClient class to communicate with a gradio server.
 * @author hugo flores garcia, aldo aguilar, xribene
 */

#pragma once

#include <fstream>

#include <juce_core/juce_core.h>

#include "Model.h"
#include "client/Client.h"
#include "client/GradioClient.h"
#include "client/StabilityClient.h"

#include "utils/Logging.h"

class WebModel : public Model
{
public:
    WebModel() { status2 = ModelStatus::INITIALIZED; }

    ~WebModel() {}

    bool ready() const override { return m_loaded; }

    ComponentInfoList& getControlsInfo() { return controlsInfo; }
    ComponentInfoList& getInputTracksInfo() { return inputTracksInfo; }
    ComponentInfoList& getOutputTracksInfo() { return outputTracksInfo; }

    // A getter function that gets a Uuid and returns the corresponding info object
    // from any of the three maps
    // This is when using ComponentInfoList
    // Not as efficient as the map version, but it's easier to use vector<pair<>> in the GUI
    std::shared_ptr<PyHarpComponentInfo> findComponentInfoByUuid(const juce::Uuid& id) const
    {
        for (const auto& pair : controlsInfo)
        {
            if (pair.first == id)
                return pair.second;
        }
        for (const auto& pair : inputTracksInfo)
        {
            if (pair.first == id)
                return pair.second;
        }
        for (const auto& pair : outputTracksInfo)
        {
            if (pair.first == id)
                return pair.second;
        }
        return nullptr;
    }

    /*OpResult load(const map<string, any>& params) override
    {
        controlsInfo.clear();
        inputTracksInfo.clear();
        outputTracksInfo.clear();
        uuidsInOrder.clear();

        status2 = ModelStatus::LOADED;
        m_loaded = true;
        return OpResult::ok();
    }*/

    // The input is a vector of String:File objects corresponding to
    // the files currently loaded in each inputMediaDisplay
    OpResult process(std::vector<std::tuple<Uuid, String, File>> localInputTrackFiles)
    {
        status2 = ModelStatus::STARTING;
        // Create an Error object in case we need it
        // and a successful result
        Error error;
        error.type = ErrorType::JsonParseError;
        OpResult result = OpResult::ok();

        status2 = ModelStatus::SENDING;

        // Clear the outputFilePaths and the labels
        // They will be populated with the new processing results
        outputFilePaths.clear();
        labels.clear();

        // We need to upload all the localInputTrackFiles to the gradio server
        // and get the vector of the remote (uploaded) file paths
        // iterate over the localInputTrackFiles map
        // and upload each file
        // and get the corresponding remote file path
        // juce::StringArray remoteTrackFilePaths;
        // std::map<juce::Uuid, std::string> remoteTrackFilePaths;
        for (auto& tuple : localInputTrackFiles)
        {
            juce::String remoteTrackFilePath;
            result = loadedClient->uploadFileRequest(std::get<2>(tuple), remoteTrackFilePath);
            if (result.failed())
            {
                result.getError().userMessage = "Failed to upload file for track "
                                                + std::get<1>(tuple) + ": "
                                                + std::get<2>(tuple).getFileName();
                status2 = ModelStatus::ERROR;
                return result;
            }
            // remoteTrackFilePaths[std::get<0>(tuple)] = remoteTrackFilePath.toStdString();
            // The following line would be a better way to do it, instead of using the remoteTrackFilePaths dict
            // it won't work though because the pyharpCOmponentInfo in the inputTracksInfo
            // needs to dynamically casted to the correct type AudioTrackInfo or MidiTrackInfo
            // inputTracksInfo[std::get<0>(tuple)]->value = remoteTrackFilePath.toStdString();
            // Here is how to do it:
            auto trackInfo = findComponentInfoByUuid(std::get<0>(tuple));
            if (trackInfo == nullptr)
            {
                status2 = ModelStatus::ERROR;
                error.devMessage = "Failed to upload file for track " + std::get<1>(tuple) + ": "
                                   + std::get<2>(tuple).getFileName()
                                   + ". The track is not an audio or midi track.";
                return OpResult::fail(error);
            }
            if (auto audioTrackInfo = dynamic_cast<AudioTrackInfo*>(trackInfo.get()))
            {
                audioTrackInfo->value = remoteTrackFilePath.toStdString();
            }
            else if (auto midiTrackInfo = dynamic_cast<MidiTrackInfo*>(trackInfo.get()))
            {
                midiTrackInfo->value = remoteTrackFilePath.toStdString();
            }
        }

        // the jsonBody is created by controlsToJson
        juce::String processingPayload;
        result = prepareProcessingPayload(processingPayload);
        if (result.failed())
        {
            result.getError().devMessage = "Failed to upload file";
            status2 = ModelStatus::ERROR;
            return result;
        }

        status2 = ModelStatus::PROCESSING;
        result = loadedClient->processRequest(error, processingPayload, outputFilePaths, labels);
        if (result.failed())
        {
            status2 = ModelStatus::ERROR;
        }
        // Finished status will be set by the MainComponent.h
        // status2 = ModelStatus::FINISHED;
        return result;
    }

    OpResult cancel()
    {
        // Create a successful result.
        // we'll update it to a failure result if something goes wrong
        status2 = ModelStatus::CANCELLING;
        OpResult result = loadedClient->cancel();
        if (result.failed())
        {
            status2 = ModelStatus::ERROR;
            return result;
        }
        status2 = ModelStatus::CANCELLED;
        return result;
    }

    ModelStatus getStatus() { return status2; }

    void setStatus(ModelStatus status) { status2 = status; }

    ModelStatus getLastStatus() { return lastStatus; }
    void setLastStatus(ModelStatus status) { lastStatus = status; }

    Client& getClient() { return *loadedClient; }
    Client& getTempClient() { return *tempClient; }
    // StabilityClient& getStabilityClient() { return stabilityClient; }

    LabelList& getLabels() { return labels; }

    std::vector<juce::String>& getOutputFilePaths() { return outputFilePaths; }

    void clearOutputFilePaths() { outputFilePaths.clear(); }

private:
    OpResult prepareProcessingPayload(juce::String& payloadJson)
    {
        // Create a JSON array to hold each control's value
        juce::Array<juce::var> jsonControlsArray;

        // Iterate through each control in controlsInfo
        // for (const auto& controlPair : controlsInfo)
        for (const auto& currentUuid : uuidsInOrder)
        {
            auto element = findComponentInfoByUuid(currentUuid);
            if (! element)
            {
                // Control not found, handle the error
                Error error;
                // error.type = ErrorType::MissingJsonKey;
                error.devMessage =
                    "Control with ID: " + currentUuid.toString() + " not found in controlsInfo.";
                return OpResult::fail(error);
            }

            juce::var controlValue;
            bool isFile = false;

            // Check the type of control and extract its value
            if (auto sliderControl = dynamic_cast<SliderInfo*>(element.get()))
            {
                controlValue = juce::var(sliderControl->value);
            }
            else if (auto textBoxControl = dynamic_cast<TextBoxInfo*>(element.get()))
            {
                controlValue = juce::var(textBoxControl->value);
            }
            else if (auto numberBoxControl = dynamic_cast<NumberBoxInfo*>(element.get()))
            {
                controlValue = juce::var(numberBoxControl->value);
            }
            else if (auto toggleControl = dynamic_cast<ToggleInfo*>(element.get()))
            {
                controlValue = juce::var(toggleControl->value);
            }
            else if (auto comboBoxControl = dynamic_cast<ComboBoxInfo*>(element.get()))
            {
                controlValue = juce::var(comboBoxControl->value);
            }

            // Audio Input
            else if (auto audioInTrackInfo = dynamic_cast<AudioTrackInfo*>(element.get()))
            {
                if (audioInTrackInfo->value.empty())
                {
                    controlValue = juce::var(); // null
                }
                else
                {
                    juce::DynamicObject::Ptr fileObj = new juce::DynamicObject();
                    fileObj->setProperty("path", juce::var(audioInTrackInfo->value));

                    juce::DynamicObject::Ptr meta = new juce::DynamicObject();
                    meta->setProperty("_type", juce::var("gradio.FileData"));
                    fileObj->setProperty("meta", juce::var(meta));

                    controlValue = juce::var(fileObj);
                }

                isFile = true;
            }

            // MIDI Input
            else if (auto midiInTrackInfo = dynamic_cast<MidiTrackInfo*>(element.get()))
            {
                // skip MIDI input for Stability models
                if (isStabilityModel)
                    continue;

                if (midiInTrackInfo->value.empty())
                {
                    controlValue = juce::var(); // null
                }
                else
                {
                    juce::DynamicObject::Ptr fileObj = new juce::DynamicObject();
                    fileObj->setProperty("path", juce::var(midiInTrackInfo->value));

                    juce::DynamicObject::Ptr meta = new juce::DynamicObject();
                    meta->setProperty("_type", juce::var("gradio.FileData"));
                    fileObj->setProperty("meta", juce::var(meta));

                    controlValue = juce::var(fileObj);
                }

                isFile = true;
            }

            else
            {
                Error error;
                error.type = ErrorType::UnsupportedControlType;
                error.devMessage =
                    "Unsupported control type for control with UUID: " + currentUuid.toString();
                return OpResult::fail(error);
            }

            // wrapping for Stability
            if (isStabilityModel)
            {
                juce::DynamicObject::Ptr wrapped = new juce::DynamicObject();

                // Important: use "input" label for audio file — required for Stability AI
                const juce::String label =
                    isFile ? juce::String("input") : juce::String(element->label);

                wrapped->setProperty("label", label);
                wrapped->setProperty("value", controlValue);

                jsonControlsArray.add(juce::var(wrapped));
            }
            else
            {
                // For Gradio: just add raw values
                jsonControlsArray.add(controlValue);
            }
        }

        juce::DynamicObject::Ptr dataObject = new juce::DynamicObject();
        dataObject->setProperty("data", jsonControlsArray);

        payloadJson = juce::JSON::toString(juce::var(dataObject), true);
        DBG_AND_LOG("prepareProcessingPayload: " + payloadJson);

        return OpResult::ok();
    }

    bool isStabilityModel =
        false; // A flag to indicate if the current model is a Stability AI model

    // A vector that stores the Uuid of the input and control components
    // in the order they are received from the server
    // We need to keep track the order to be able to send the data
    // for processing in the same order.
    // We wouldn't have to do that if
    //    1. c++ had an ordered map (like python)
    //    2. the gradio server would accept key:value pairs instead of list
    std::vector<juce::Uuid> uuidsInOrder;
    std::unique_ptr<Client> loadedClient;
    // GradioClient gradioClient;
    // StabilityClient stabilityClient;

    // A helper variable to store the status of the model
    // before loading a new model. If the new model fails to load,
    // we want to go back to the status we had before the failed attempt
    ModelStatus lastStatus;

    // A variable to store the latest labelList received during processing
    LabelList labels;
    // A vector to store the output file paths we get from gradio
    // after processing. We assume that the order of the output files
    // is the same as the order of the outputTracksInfo
    std::vector<juce::String> outputFilePaths;
};

// a timer that checks the status of the model and broadcasts a change if if there is one
class ModelStatusTimer : public juce::Timer, public juce::ChangeBroadcaster
{
public:
    ModelStatusTimer(std::shared_ptr<WebModel> model) : m_model(model) {}

    void timerCallback() override
    {
        // get the status of the model
        ModelStatus status = m_model->getStatus();
        // DBG_AND_LOG("ModelStatusTimer::timerCallback status: " + std::to_string(status)
        //     + " lastStatus: " + std::to_string(lastStatus));

        // if the status has changed, broadcast a change
        if (status != lastStatus)
        {
            lastStatus = status;
            sendChangeMessage();
        }
    }

    void setModel(std::shared_ptr<WebModel> model)
    {
        // stopTimer();
        m_model = model;
        // lastStatus = ModelStatus::INITIALIZED;
        // startTimer(50);
    }

private:
    std::shared_ptr<WebModel> m_model;
    ModelStatus lastStatus;
};
