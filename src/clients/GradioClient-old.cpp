#include "GradioClient.h"

#include "../utils/Enums.h"
#include "../utils/Errors.h"

/*
// Requests
OpResult GradioClient::processRequest(Error& error,
                                      String& processingPayload,
                                      std::vector<String>& outputFilePaths,
                                      LabelList& labels)
{
    // Iterate through the array elements
    for (int i = 0; i < dataArray->size(); i++)
    {

        else if (procObjType == "pyharp.LabelList")
        {
            Array<var>* labelsPyharp = procObj.getDynamicObject()->getProperty("labels").getArray();

            for (int j = 0; j < labelsPyharp->size(); j++)
            {
                DynamicObject* labelPyharp = labelsPyharp->getReference(j).getDynamicObject();
                String labelType = labelPyharp->getProperty("label_type").toString();
                std::unique_ptr<OutputLabel> label;

                if (labelType == "AudioLabel")
                {
                    auto audioLabel = std::make_unique<AudioLabel>();
                    if (labelPyharp->hasProperty("amplitude"))
                    {
                        if (labelPyharp->getProperty("amplitude").isDouble()
                            || labelPyharp->getProperty("amplitude").isInt())
                        {
                            audioLabel->amplitude =
                                static_cast<float>(labelPyharp->getProperty("amplitude"));
                        }
                    }
                    label = std::move(audioLabel);
                }
                else if (labelType == "SpectrogramLabel")
                {
                    auto spectrogramLabel = std::make_unique<SpectrogramLabel>();
                    if (labelPyharp->hasProperty("frequency"))
                    {
                        if (labelPyharp->getProperty("frequency").isDouble()
                            || labelPyharp->getProperty("frequency").isInt())
                        {
                            spectrogramLabel->frequency =
                                static_cast<float>(labelPyharp->getProperty("frequency"));
                        }
                    }
                    label = std::move(spectrogramLabel);
                }
                else if (labelType == "MidiLabel")
                {
                    auto midiLabel = std::make_unique<MidiLabel>();
                    if (labelPyharp->hasProperty("pitch"))
                    {
                        if (labelPyharp->getProperty("pitch").isDouble()
                            || labelPyharp->getProperty("pitch").isInt())
                        {
                            midiLabel->pitch =
                                static_cast<float>(labelPyharp->getProperty("pitch"));
                        }
                    }
                    label = std::move(midiLabel);
                }
                else if (labelType == "OutputLabel")
                {
                    auto outputLabel = std::make_unique<OutputLabel>();
                    label = std::move(outputLabel);
                }
                else
                {
                    error.type = ErrorType::UnknownLabelType;
                    error.devMessage = "Unknown label type: " + labelType;
                    return OpResult::fail(error);
                }
                // All the labels, no matter their type, have some common properties
                // t: float
                // label: str
                // duration: float = 0.0
                // description: str = None
                // color: int = 0
                // first we'll check which of those exist and are not void or null
                // for those that exist, we fill the struct properties
                // the rest will be ignored
                if (labelPyharp->hasProperty("t"))
                {
                    // now check if it's a float
                    if (labelPyharp->getProperty("t").isDouble()
                        || labelPyharp->getProperty("t").isInt())
                    {
                        label->t = static_cast<float>(labelPyharp->getProperty("t"));
                    }
                }
                if (labelPyharp->hasProperty("label"))
                {
                    // now check if it's a string
                    if (labelPyharp->getProperty("label").isString())
                    {
                        label->label = labelPyharp->getProperty("label").toString();
                        DBG_AND_LOG("label: " + label->label);
                    }
                }
                if (labelPyharp->hasProperty("duration"))
                {
                    // now check if it's a float
                    if (labelPyharp->getProperty("duration").isDouble()
                        || labelPyharp->getProperty("duration").isInt())
                    {
                        label->duration = static_cast<float>(labelPyharp->getProperty("duration"));
                    }
                }
                if (labelPyharp->hasProperty("description"))
                {
                    // now check if it's a string
                    if (labelPyharp->getProperty("description").isString())
                    {
                        label->description = labelPyharp->getProperty("description").toString();
                    }
                }
                if (labelPyharp->hasProperty("color"))
                {
                    // now check if it's an int
                    if ((labelPyharp->getProperty("color").isInt64()
                         || labelPyharp->getProperty("color").isInt()))
                    {
                        int color_val = static_cast<int>(labelPyharp->getProperty("color"));

                        if (color_val != 0)
                        {
                            label->color = color_val;
                        }
                    }
                }
                if (labelPyharp->hasProperty("link"))
                {
                    // now check if it's a string
                    if (labelPyharp->getProperty("link").isString())
                    {
                        label->link = labelPyharp->getProperty("link").toString();
                    }
                }
                labels.push_back(std::move(label));
            }
        }
        else
        {
            DBG_AND_LOG("The pyharp Gradio app returned a " + procObjType
                        + " object, that we don't yet support in HARP.");
        }
    }
    return result;
}
*/

/*
OpResult GradioClient::cancel()
{
    OpResult result = OpResult::ok();
    String eventId;
    String endpoint = "cancel";

    // Perform a POST request to the cancel endpoint to get the event ID
    String jsonBody = R"({"data": []})"; // The body is empty in this case

    result = makePostRequestForEventID(endpoint, eventId, jsonBody);
    if (result.failed())
    {
        return result;
    }

    // Use the event ID to make a GET request for the cancel response
    String response;
    result = getResponseFromEventID(endpoint, eventId, response);
    return result;
}
*/

/*
OpResult GradioClient::downloadFileFromURL(const URL& fileURL,
                                           String& downloadedFilePath,
                                           const int timeoutMs) const
{
    // Create the error here, in case we need it
    Error error;
    error.type = ErrorType::FileDownloadError;

    // Determine the local temporary directory for storing the downloaded file
    File tempDir = File::getSpecialLocation(File::tempDirectory);
    String fileName = fileURL.getFileName();
    // // Add a timestamp to the file name to avoid overwriting
    // // Insert timestamp before the file extension using File operations
    // String baseName = File::createFileWithoutCheckingPath(fileName).getFileNameWithoutExtension();
    // String extension = File::createFileWithoutCheckingPath(fileName).getFileExtension();
    // String timestamp = "_" + String(Time::getCurrentTime().formatted("%Y%m%d%H%M%S"));
    // fileName = baseName + timestamp + extension;
    String baseName = File::createFileWithoutCheckingPath(fileName).getFileNameWithoutExtension();
    String extension = File::createFileWithoutCheckingPath(fileName).getFileExtension();
    File downloadedFile = tempDir.getChildFile(baseName + "_" + Uuid().toString() + extension);

    // Create input stream to download the file
    StringPairArray responseHeaders;
    int statusCode = 0;
    auto options = URL::InputStreamOptions(URL::ParameterHandling::inAddress)
                       .withExtraHeaders(createCommonHeaders())
                       .withConnectionTimeoutMs(timeoutMs)
                       .withResponseHeaders(&responseHeaders)
                       .withStatusCode(&statusCode)
                       .withNumRedirectsToFollow(5);

    std::unique_ptr<InputStream> stream(fileURL.createInputStream(options));

    if (stream == nullptr)
    {
        error.devMessage = "Failed to create input stream for file download request.";
        return OpResult::fail(error);
    }

    // Check if the request was successful
    if (statusCode != 200)
    {
        error.devMessage = "Request failed with status code: " + String(statusCode);
        return OpResult::fail(error);
    }

    // Remove file at target path if one already exists
    // Before adding this, the new file did not replace the old file
    // TODO - make file path unique (timestamp or Uuid)
    downloadedFile.deleteFile();

    // Create output stream to save the file locally
    std::unique_ptr<FileOutputStream> fileOutput(downloadedFile.createOutputStream());

    if (fileOutput == nullptr || ! fileOutput->openedOk())
    {
        error.devMessage =
            "Failed to create output stream for file: " + downloadedFile.getFullPathName();
        return OpResult::fail(error);
    }

    // Copy data from the input stream to the output stream
    fileOutput->writeFromInputStream(*stream, stream->getTotalLength());

    // Store the file path where the file was downloaded
    downloadedFilePath = downloadedFile.getFullPathName();

    return OpResult::ok();
}
*/
