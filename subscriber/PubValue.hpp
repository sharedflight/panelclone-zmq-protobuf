#ifndef _PUBVALUE_HPP_
#define _PUBVALUE_HPP_

#include <string>

#include <event.pb.h>

struct PubValue
{
    size_t index;
    std::string dataref;
    int dref_index;
    std::variant<int, float, double, std::pair<std::string_view, size_t>> value;
    panelclone::DrefValue::ValueCase chosenType;
    int last_frame_updated;

    float floatValue() {
        switch (chosenType) {
            case panelclone::DrefValue::ValueCase::kIntVal:
                return static_cast<float>(std::get<int>(value));
                break;
            case panelclone::DrefValue::ValueCase::kFloatVal:
                return std::get<float>(value);
                break;
            case panelclone::DrefValue::ValueCase::kDoubleVal:
                return static_cast<float>(std::get<double>(value));
                break;
            case panelclone::DrefValue::ValueCase::kByteVal:
            default:
                return 0;
                break;
        }
    }
};

#endif // #ifndefine _PUBVALUE_HPP_