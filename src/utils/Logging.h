#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

using namespace juce;

/**
 * Helper function to simplify logging calls.
 */
inline void LogAndDBG(const String& message)
{
    Logger::getInstance()->LogAndDBG(message);
}

class Logger : private DeletedAtShutdown
{
   public:
    JUCE_DECLARE_SINGLETON(Logger, false)

    ~Logger()
    {
        logger.reset(); // Explicitly reset pointer to release FileLogger

        clearSingletonInstance();
    }

    // Disable copy constructor
    Logger(const Logger&) = delete;
    // Disable assignment operator
    Logger& operator=(const Logger&) = delete;

    void initializeLogger()
    {
        // MacOS: ~/Library/Logs/HARP/main.log
        // Windows: C:\Users\<username>\AppData\Roaming\HARP\main.log
        // Linux: ~/.config/HARP/main.log
        logger.reset(juce::FileLogger::createDefaultAppLogger("HARP", "main.log", ""));
    }

    void LogAndDBG(const String& message) const
    {
        DBG(message); // Write to console

        if (logger)
        {
            logger->logMessage(message); // Write to file
        }
    }

    File getLogFile() const
    {
        return logger->getLogFile();
    }

   private:
    Logger() = default; // Prevents instantiation from outside

    std::unique_ptr<FileLogger> logger{nullptr};
};
