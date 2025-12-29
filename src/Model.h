#pragma once

#include <string>
#include <vector>

enum ModelStatus
{
    EMPTY,

    QUERYING_CLIENT,
    GETTING_CONTROLS,
    LOADED, // TODO - rename to IDLE?

    LOADING_ERROR, // TODO - synonymous with EMPTY?

    // TODO - proccessing stuff below
    STARTING, // TODO - synonymous with QUERYING_CLIENT?
    SENDING,
    PROCESSING,
    FINISHED, // TODO - synonymous with LOADED?
    CANCELLING,
    CANCELLED, // TODO - synonymous with LOADED?

    PROCESSING_ERROR // TODO - synonymous with LOADED?
};

struct ModelMetadata
{
    std::string name;
    std::string description;
    std::string author;
    std::vector<std::string> tags;
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

    // TODO - these are not necessarily the correct ways to check this
    bool isEmpty() { return status == ModelStatus::EMPTY; }
    bool isLoaded() { return status == ModelStatus::LOADED; }

    // TODO - update statusBox
    void setStatus(ModelStatus newStatus) { status = newStatus; }

    ModelMetadata getMetadata() { return metadata; }

    void resetState()
    {
        metadata = ModelMetadata {};

        setStatus(ModelStatus::EMPTY);
    }

private:
    ModelStatus status;

    ModelMetadata metadata;

    // TODO - subject to change
    //ComponentInfoList controlsInfo;
    //ComponentInfoList inputTracksInfo;
    //ComponentInfoList outputTracksInfo;
};
