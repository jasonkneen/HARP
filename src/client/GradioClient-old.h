/**
 * @file
 * @brief A GradioClient class for making http requests to the Gradio API
 * @author  xribene
 */

#pragma once

#include "juce_core/juce_core.h"
#include <fstream>

#include "../utils/Logging.h"
#include "../utils/Errors.h"
//#include "../utils.h"
#include "Client.h"

using namespace juce;

class GradioClient : public Client
{
public:
    GradioClient();
    ~GradioClient() = default;

    // Requests
    OpResult uploadFileRequest(const File& fileToUpload,
                               String& uploadedFilePath,
                               const int timeoutMs = 10000) const override;
    OpResult processRequest(Error&, String&, std::vector<String>&, LabelList&) override;
    OpResult cancel() override;

    // Authorization
    OpResult validateToken(const String& newToken) const override;

private:
    OpResult downloadFileFromURL(const URL& fileURL,
                                 String& downloadedFilePath,
                                 const int timeoutMs = 10000) const;
};
