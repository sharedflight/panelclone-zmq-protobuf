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

int PubFloatValue_getvf32(const PubFloatValue *dr, float *ff, unsigned off, unsigned num);

float PubFloatValue_getf(const PubFloatValue *dr);

#ifdef __cplusplus
}
#endif

#endif // #ifndefine _PUBFLOATVALUE_HPP_