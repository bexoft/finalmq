#pragma once

#include <cstdint>
#include <string>

#include "finalmq/helpers/FmqDefines.h"

template<class T>
void string2Number(const std::string& str, T& number)
{
}

template<>
void string2Number(const std::string& str, bool& number)
{
    static const std::string STR_TRUE = "true";
    if (str != STR_TRUE)
    {
        double value = strtof64(str.c_str(), nullptr);
#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
        if (value == 0)
#ifndef WIN32
#pragma GCC diagnostic pop
#endif
        {
            number = false;
        }
    }
    number = true;
}

template<>
void string2Number(const std::string& str, std::int8_t& number)
{
    number = static_cast<std::int8_t>(strtol(str.c_str(), nullptr, 10));
}

template<>
void string2Number(const std::string& str, std::uint8_t& number)
{
    number = static_cast<std::uint8_t>(strtoul(str.c_str(), nullptr, 10));
}

template<>
void string2Number(const std::string& str, std::int16_t& number)
{
    number = static_cast<std::int16_t>(strtol(str.c_str(), nullptr, 10));
}

template<>
void string2Number(const std::string& str, std::uint16_t& number)
{
    number = static_cast<std::uint16_t>(strtoul(str.c_str(), nullptr, 10));
}

template<>
void string2Number(const std::string& str, std::int32_t& number)
{
    number = static_cast<std::int32_t>(strtol(str.c_str(), nullptr, 10));
}

template<>
void string2Number(const std::string& str, std::uint32_t& number)
{
    number = static_cast<std::uint32_t>(strtoul(str.c_str(), nullptr, 10));
}

template<>
void string2Number(const std::string& str, std::int64_t& number)
{
    number = strtoll(str.c_str(), nullptr, 10);
}

template<>
void string2Number(const std::string& str, std::uint64_t& number)
{
    number = strtoull(str.c_str(), nullptr, 10);
}

template<>
void string2Number(const std::string& str, float& number)
{
    number = strtof32(str.c_str(), nullptr);
}

template<>
void string2Number(const std::string& str, double& number)
{
    number = strtof64(str.c_str(), nullptr);
}
