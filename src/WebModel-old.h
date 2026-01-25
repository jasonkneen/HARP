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
    // The input is a vector of String:File objects corresponding to
    // the files currently loaded in each inputMediaDisplay
    OpResult process(std::vector<std::tuple<Uuid, String, File>> localInputTrackFiles)
    {
        // Clear the outputFilePaths and the labels
        // They will be populated with the new processing results
        //outputFilePaths.clear();
        //labels.clear();

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

private:

    // A variable to store the latest labelList received during processing
    LabelList labels;
    // A vector to store the output file paths we get from gradio
    // after processing. We assume that the order of the output files
    // is the same as the order of the outputTracksInfo
    std::vector<juce::String> outputFilePaths;
};
