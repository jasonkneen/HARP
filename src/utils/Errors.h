/**
 * @file
 * @brief Classes and helper functions for error handling.
 * @author xribene, cwitkowitz
 */

#pragma once

#include <juce_core/juce_core.h>

using namespace juce;

struct ClientError
{
    enum class Type
    {
        UnknownClient,
        InvalidModelPath
    };

    Type type;

    String path;
    String client;
};

inline String toUserMessage(const ClientError& e)
{
    String userMessage = "A client error occurred.";

    switch (e.type)
    {
        case ClientError::Type::UnknownClient:

            userMessage = "Path ";

            if (e.path.isNotEmpty())
            {
                "\"" + e.path + "\" ";
            }

            userMessage += "does not match valid specification for any supported clients.";

            return userMessage;

        case ClientError::Type::InvalidModelPath:

            userMessage = "Path ";

            if (e.path.isNotEmpty())
            {
                "\"" + e.path + "\" ";
            }

            userMessage += "does not match valid specification for client";

            if (e.client.isNotEmpty())
            {
                " \"" + e.client + "\"";
            }

            userMessage += ".";

            return userMessage;
    }

    return userMessage;
}

/*
struct ControlError
{
    enum class Type
    {
        UnsupportedControlType,
        InvalidValue
    };

    Type type;
};
*/

/*
struct FileError
{
    enum class Type
    {
        UploadFailed,
        DownloadFailed,
        PermissionDenied
    };

    Type type;
};
*/

/*
struct HuggingFaceError
{
    enum class Type
    {
        Paused,
        Sleeping
    };

    Type type;
};
*/

using Error = std::variant<ClientError>;

inline String toUserMessage(const Error& error)
{
    return std::visit([](const auto& e) { return toUserMessage(e); }, error);
}

class OpResult
{
public:
    /**
     * Create a result indicating success.
     */
    static OpResult ok() noexcept { return OpResult(Result::ok(), std::nullopt); }

    /**
     * Create a result indicating failure with a specific error.
     */
    static OpResult fail(Error error)
    {
        return OpResult(Result::fail(toUserMessage(error)), std::move(error));
    }

    /**
     * Check if this result indicates success.
     */
    bool wasOk() const noexcept { return result.wasOk(); }

    /**
     * Check if this result indicates failure.
     */
    bool failed() const noexcept { return result.failed(); }

    /**
     * Obtain the error associated with this result if one exists.
     */
    const Error* getError() const noexcept { return error ? &*error : nullptr; }

    explicit operator bool() const noexcept { return wasOk(); }

private:
    OpResult(Result r, std::optional<Error> e) : result(std::move(r)), error(std::move(e)) {}

    Result result;

    std::optional<Error> error;
};
