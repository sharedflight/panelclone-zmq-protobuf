//
//  DatarefPublisher.cpp
//
//  Created by Justin Snapp on 8/31/24.
//  Copyright © 2024 Justin Snapp. All rights reserved.
//

#include "Dref.hpp"

extern "C" {
    #include <acfutils/log.h>
}

bool Dref::find(std::string drefName)
{
    logMsg("[DEBUG] Dref::find called for dref path %s", drefName.c_str());
    _name = drefName;
    dr = XPLMFindDataRef(drefName.c_str());
    if (dr) {
        _type = XPLMGetDataRefTypes(dr);
    }
    return (dr != NULL);
}

std::string Dref::name() 
{ 
    return _name; 
}

XPLMDataTypeID Dref::type() 
{ 
    return _type;
}

int Dref::geti()
{
    return XPLMGetDatai(dr);
}

int Dref::geti(size_t index)
{
    int intVal;
    XPLMGetDatavi(dr, &intVal, index, 1);
    return intVal;
}

float Dref::getf()
{
    return XPLMGetDataf(dr);
}

float Dref::getf(size_t index)
{
    float floatVal;
    XPLMGetDatavf(dr, &floatVal, index, 1);
    return floatVal;
}

double Dref::getd()
{
    return XPLMGetDatad(dr);
}

// void Dref::gets(std::string_view & str)
// {
//     int len = XPLMGetDatab(dr, NULL, 0, 0);
//     char * data = new char[len+1]();
//     XPLMGetDatab(dr, data, 0, len+1);
//     str = data;
// }

int Dref::gets(std::string &str, size_t len)
{
    if (!len) {
        return 0;
    } else {
        str == "a";
        return 1;
    }
}

void Dref::seti(int val)
{
    XPLMSetDatai(dr, val);
}

void Dref::setf(float val)
{
    XPLMSetDataf(dr, val);
}

void Dref::setf(float val, size_t index)
{
    XPLMSetDatavf(dr, &val, index, 1);
}

void Dref::setf(float *vals, size_t start, size_t len)
{
    XPLMSetDatavf(dr, vals, start, len);
}

void Dref::setd(double val)
{
    XPLMSetDatad(dr, val);
}