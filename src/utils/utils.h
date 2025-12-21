/**
 * @file
 * @brief Utility structs and enums used in the application
 * @author xribene
 */

#pragma once

#include "juce_core/juce_core.h"
#include "juce_gui_basics/juce_gui_basics.h"

using namespace juce;

// A function to convert a boolean string to a c++ boolean value
// JUCE doesn't have a built-in function to do this
inline bool stringToBool(const String& str)
{
    String lowerStr = str.toLowerCase();
    if (lowerStr == "true" || lowerStr == "1" || lowerStr == "yes" || lowerStr == "y")
        return true;
    return false;
}
