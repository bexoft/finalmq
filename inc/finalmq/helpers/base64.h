

#pragma once

#include "BexDefines.h"

#include <vector>
#include <string>


namespace finalmq
{

class Base64
{
public:
    inline static ssize_t encodeSize(ssize_t size)
    { 
        return ((((size + 2) / 3) * 4) + 1); 
    }

    inline static ssize_t decodeSize(ssize_t size)
    { 
        return (((size / 4) * 3) + 2); 
    }

    static void encode(const char* src, ssize_t len, std::string& dest);
    static void encode(const std::vector<char>& src, std::string& dest);
    static ssize_t decode(const char* src, ssize_t len, std::vector<char>& dest);
    static ssize_t decode(const std::string& src, std::vector<char>& dest);
};


}