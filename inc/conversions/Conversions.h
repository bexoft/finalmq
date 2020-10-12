#pragma once

#include <cstdint>


template <class T>
void string2Number(const std::string& str, T& number)
{
}


template <>
void string2Number(const std::string& str, bool& number)
{
    static const std::string STR_TRUE = "true";
    if (str != STR_TRUE)
    {
        double number = strtof64(str.c_str(), nullptr);
        if (number == 0)
        {
            number = false;
        }
    }
    number = true;
}

template <>
void string2Number(const std::string& str, std::int32_t& number)
{
    number = strtol(str.c_str(), nullptr, 10);
}

template <>
void string2Number(const std::string& str, std::uint32_t& number)
{
    number = strtoul(str.c_str(), nullptr, 10);
}

template <>
void string2Number(const std::string& str, std::int64_t& number)
{
    number = strtoll(str.c_str(), nullptr, 10);
}

template <>
void string2Number(const std::string& str, std::uint64_t& number)
{
    number = strtoull(str.c_str(), nullptr, 10);
}

template <>
void string2Number(const std::string& str, float& number)
{
    number = strtof32(str.c_str(), nullptr);
}

template <>
void string2Number(const std::string& str, double& number)
{
    number = strtof64(str.c_str(), nullptr);
}

