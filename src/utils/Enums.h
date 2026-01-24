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
