//MIT License

//Copyright (c) 2020 bexoft GmbH (mail@bexoft.de)

//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#pragma once

#include "Variant.h"
#include "assert.h"

#include <functional>
#include <unordered_map>


namespace finalmq {

template<class T>
class SYMBOLEXP Convert
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

}   // namespace finalmq
