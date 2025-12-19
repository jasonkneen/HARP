#include "juce_core/juce_core.h"

using namespace juce;

struct PyHarpComponentInfo
{
    Uuid id{""};
    std::string label{""};
    std::string info{""};

    virtual ~PyHarpComponentInfo() = default; // Virtual destructor
};

struct SliderInfo : public PyHarpComponentInfo
{
    double minimum;
    double maximum;
    double step;
    double value;
};

struct TextBoxInfo : public PyHarpComponentInfo
{
    std::string value;
};

struct AudioTrackInfo : public PyHarpComponentInfo
{
    std::string value;
    bool required;
};

struct MidiTrackInfo : public PyHarpComponentInfo
{
    std::string value;
    bool required;
};

struct NumberBoxInfo : public PyHarpComponentInfo
{
    double min;
    double max;
    double value;
};

struct ToggleInfo : public PyHarpComponentInfo
{
    bool value;
};

struct ComboBoxInfo : public PyHarpComponentInfo
{
    std::vector<std::string> options;
    std::string value;
};

using ComponentInfo = std::pair<Uuid, std::shared_ptr<PyHarpComponentInfo>>;
using ComponentInfoMap = std::map<Uuid, std::shared_ptr<PyHarpComponentInfo>>;
using ComponentInfoList = std::vector<ComponentInfo>;
