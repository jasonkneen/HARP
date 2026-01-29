/**
 * @file
 * @brief A StabilityClient class for making http requests to the Stability AI's API
 * @author  xribene, huiranyu
 */

#pragma once

#include <fstream>
#include <BinaryData.h>

#include <juce_core/juce_core.h>

#include "../external/magic_enum.hpp"

#include "Client.h"

#include "../utils/Logging.h"
#include "../utils/Errors.h"

using namespace juce;

class StabilityClient : public Client
{
public:
    StabilityClient();
    ~StabilityClient() = default;

    OpResult cancel() override;

private:
    std::atomic<bool> shouldCancel { false };
};
