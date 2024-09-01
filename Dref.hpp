//
//  DatarefPublisher.hpp
//
//  Created by Justin Snapp on 8/31/24.
//  Copyright © 2024 Justin Snapp. All rights reserved.
//

#ifndef Dref_hpp
#define Dref_hpp

#include <string>

extern "C" {
    #include <XPLMDataAccess.h>
}

class Dref {
    std::string _name;
    XPLMDataRef dr;
    XPLMDataTypeID _type;
    bool writable;

    // For datarefs we create...
    void *value;
    size_t count;
    size_t stride;

public:

    bool find(std::string drefName);

    std::string name();
    XPLMDataTypeID type();

    int geti();
    int geti(size_t index);
    float getf();
    float getf(size_t index);
    double getd();
    int gets(std::string &str, size_t len);

    void seti(int val);
    void setf(float val);
    void setf(float val, size_t index);
    void setf(float *vals, size_t start, size_t len);
    void setd(double val);
    
};

#endif /* Dref_hpp */