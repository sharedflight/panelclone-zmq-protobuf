#include "PubFloatValue.hpp"

#include "DatarefSubscriber.hpp"

extern "C" 
int PubFloatValue_find(PubFloatValue *dr, const char *dr_name) {
    bool success = DatarefSubscriber::getInstance().FindFloatValue(dr, std::string(dr_name));

    return (success);
}

extern "C" 
float PubFloatValue_getf(const PubFloatValue *dr) {
    assert(dr);
    float val = DatarefSubscriber::getInstance().GetFloatValue(dr->index);
    return val;
}