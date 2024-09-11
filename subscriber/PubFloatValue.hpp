#ifndef _PUBFLOATVALUE_HPP_
#define _PUBFLOATVALUE_HPP_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PubFloatValue_s
{
    int index;
    int dref_index;
    float value;
} PubFloatValue;

int PubFloatValue_find(PubFloatValue *dr, const char *dr_name);

float PubFloatValue_getf(const PubFloatValue *dr);

#ifdef __cplusplus
}
#endif

#endif // #ifndefine _PUBFLOATVALUE_HPP_