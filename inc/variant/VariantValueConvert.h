#pragma once

#include "Variant.h"
#include "assert.h"

#include <functional>
#include <unordered_map>

template<class T>
class Convert
{
public:
    typedef std::function<T(const Variant& variant)> FuncConvert;

    static void registerConversion(int varType, FuncConvert funcConvert)
    {
        std::unordered_map<int, FuncConvert>& registry = getRegistry();
        registry[varType] = funcConvert;
    }
    static T convert(const Variant& variant)
    {
        if (MetaTypeInfo<T>::TypeId == variant.getType())
        {
            const T* data = variant;
            assert(data);
            return *data;
        }
        const std::unordered_map<int, FuncConvert>& registry = getRegistry();
        auto it = registry.find(variant.getType());
        if (it != registry.end())
        {
            auto& funcConvert = it->second;
            if (funcConvert)
            {
                return funcConvert(variant);
            }
        }
        return T();
    }

private:
    static std::unordered_map<int, FuncConvert>& getRegistry()
    {
        static std::unordered_map<int, FuncConvert> conversions;
        return conversions;
    }
};


