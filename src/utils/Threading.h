#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

#include "Logging.h"

using namespace juce;

// TODO - is this really even needed?

class CustomThreadPoolJob : public ThreadPoolJob
{
public:
    CustomThreadPoolJob(std::function<void(String)> _jobFunction, String jobID)
        : ThreadPoolJob(jobID), jobFunction(_jobFunction)
    {
    }

    ~CustomThreadPoolJob() override { DBG_AND_LOG("Job Stopped"); }

    JobStatus runJob() override
    {
        if (jobFunction)
        {
            jobFunction(getJobName());
            return jobHasFinished;
        }
        else
        {
            return jobHasFinished;
        }
    }

private:
    std::function<void(String)> jobFunction;
};
