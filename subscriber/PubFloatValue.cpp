#include "PubFloatValue.hpp"

#include "DatarefSubscriber.hpp"

extern "C" 
int PubFloatValue_find(PubFloatValue *dr, const char *dr_name) {
    bool success = DatarefSubscriber::getInstance().FindFloatValue(dr, std::string(dr_name));

    return (success);
}

extern "C" 
int PubFloatValue_getvf32(const PubFloatValue *dr, float *ff, unsigned off, unsigned num) {
    assert(dr);
    assert(ff);
    off;
    num;
    return 0;
}

extern "C" 
float PubFloatValue_getf(const PubFloatValue *dr) {
    assert(dr);
    return DatarefSubscriber::getInstance().GetFloatValue(dr->index);
}