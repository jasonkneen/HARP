#pragma once

#include <juce_core/juce_core.h>

#include "../external/magic_enum.hpp"

using namespace juce;

// TODO - is this or magic_enum even used / required?

template <typename EnumType>
inline String enumToString(EnumType enumValue)
{
    return String(magic_enum::enum_name(enumValue).data());
}

// A function to convert a boolean string to a c++ boolean value
// JUCE doesn't have a built-in function to do this
/*inline bool stringToBool(const String& str)
{
    String lowerStr = str.toLowerCase();
    if (lowerStr == "true" || lowerStr == "1" || lowerStr == "yes" || lowerStr == "y")
        return true;
    return false;
}*/
