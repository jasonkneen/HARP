#include "GradioClient.h"

#include "../utils/Enums.h"
#include "../utils/Errors.h"

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
