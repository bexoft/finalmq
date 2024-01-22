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

#include "finalmq/variant/VariantValueRegisterConversions.h"

#include "finalmq/conversions/Conversions.h"
#include "finalmq/variant/VariantValues.h"

namespace finalmq
{
static VariantValueRegisterConversions g_registerConversions;

template<class FROM, class TO>
class FunctionConvert
{
public:
    TO operator()(const Variant& variant)
    {
        assert(variant.getType() == MetaTypeInfo<FROM>::TypeId);
        const FROM* data = variant;
        assert(data);
#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
        return static_cast<TO>(*data);
#ifndef WIN32
#pragma GCC diagnostic pop
#endif
    }
};

template<class TO>
class FunctionConvertStringToNumber
{
public:
    TO operator()(const Variant& variant)
    {
        assert(variant.getType() == MetaTypeId::TYPE_STRING);
        const std::string* data = variant;
        TO number;
        string2Number(*data, number);
        return number;
    }
};

template<class FROM>
class FunctionConvertNumberToString
{
public:
    std::string operator()(const Variant& variant)
    {
        assert(variant.getType() == MetaTypeInfo<FROM>::TypeId);
        const FROM* data = variant;
        assert(data);
        return std::to_string(*data);
    }
};
template<>
class FunctionConvertNumberToString<bool>
{
public:
    std::string operator()(const Variant& variant)
    {
        assert(variant.getType() == MetaTypeInfo<bool>::TypeId);
        const bool* data = variant;
        assert(data);
        return (*data) ? "true" : "false";
    }
};

template<class TO>
void VariantValueRegisterConversions::registerConversionsTo()
{
    Convert<TO>::registerConversion(MetaTypeId::TYPE_BOOL, FunctionConvert<bool, TO>());
    Convert<TO>::registerConversion(MetaTypeId::TYPE_INT8, FunctionConvert<std::int8_t, TO>());
    Convert<TO>::registerConversion(MetaTypeId::TYPE_UINT8, FunctionConvert<std::uint8_t, TO>());
    Convert<TO>::registerConversion(MetaTypeId::TYPE_INT16, FunctionConvert<std::int16_t, TO>());
    Convert<TO>::registerConversion(MetaTypeId::TYPE_UINT16, FunctionConvert<std::uint16_t, TO>());
    Convert<TO>::registerConversion(MetaTypeId::TYPE_INT32, FunctionConvert<std::int32_t, TO>());
    Convert<TO>::registerConversion(MetaTypeId::TYPE_UINT32, FunctionConvert<std::uint32_t, TO>());
    Convert<TO>::registerConversion(MetaTypeId::TYPE_INT64, FunctionConvert<std::int64_t, TO>());
    Convert<TO>::registerConversion(MetaTypeId::TYPE_UINT64, FunctionConvert<std::uint64_t, TO>());
    Convert<TO>::registerConversion(MetaTypeId::TYPE_FLOAT, FunctionConvert<float, TO>());
    Convert<TO>::registerConversion(MetaTypeId::TYPE_DOUBLE, FunctionConvert<double, TO>());
}

VariantValueRegisterConversions::VariantValueRegisterConversions()
{
    registerConversionsTo<bool>();
    registerConversionsTo<std::int8_t>();
    registerConversionsTo<std::uint8_t>();
    registerConversionsTo<std::int16_t>();
    registerConversionsTo<std::uint16_t>();
    registerConversionsTo<std::int32_t>();
    registerConversionsTo<std::uint32_t>();
    registerConversionsTo<std::int64_t>();
    registerConversionsTo<std::uint64_t>();
    registerConversionsTo<float>();
    registerConversionsTo<double>();

    Convert<std::string>::registerConversion(MetaTypeId::TYPE_BOOL, FunctionConvertNumberToString<bool>());
    Convert<std::string>::registerConversion(MetaTypeId::TYPE_INT8, FunctionConvertNumberToString<std::int8_t>());
    Convert<std::string>::registerConversion(MetaTypeId::TYPE_UINT8, FunctionConvertNumberToString<std::uint8_t>());
    Convert<std::string>::registerConversion(MetaTypeId::TYPE_INT16, FunctionConvertNumberToString<std::int16_t>());
    Convert<std::string>::registerConversion(MetaTypeId::TYPE_UINT16, FunctionConvertNumberToString<std::uint16_t>());
    Convert<std::string>::registerConversion(MetaTypeId::TYPE_INT32, FunctionConvertNumberToString<std::int32_t>());
    Convert<std::string>::registerConversion(MetaTypeId::TYPE_UINT32, FunctionConvertNumberToString<std::uint32_t>());
    Convert<std::string>::registerConversion(MetaTypeId::TYPE_INT64, FunctionConvertNumberToString<std::int64_t>());
    Convert<std::string>::registerConversion(MetaTypeId::TYPE_UINT64, FunctionConvertNumberToString<std::uint64_t>());
    Convert<std::string>::registerConversion(MetaTypeId::TYPE_FLOAT, FunctionConvertNumberToString<float>());
    Convert<std::string>::registerConversion(MetaTypeId::TYPE_DOUBLE, FunctionConvertNumberToString<double>());

    Convert<bool>::registerConversion(MetaTypeId::TYPE_STRING, FunctionConvertStringToNumber<bool>());
    Convert<std::int8_t>::registerConversion(MetaTypeId::TYPE_STRING, FunctionConvertStringToNumber<std::int8_t>());
    Convert<std::uint8_t>::registerConversion(MetaTypeId::TYPE_STRING, FunctionConvertStringToNumber<std::uint8_t>());
    Convert<std::int16_t>::registerConversion(MetaTypeId::TYPE_STRING, FunctionConvertStringToNumber<std::int16_t>());
    Convert<std::uint16_t>::registerConversion(MetaTypeId::TYPE_STRING, FunctionConvertStringToNumber<std::uint16_t>());
    Convert<std::int32_t>::registerConversion(MetaTypeId::TYPE_STRING, FunctionConvertStringToNumber<std::int32_t>());
    Convert<std::uint32_t>::registerConversion(MetaTypeId::TYPE_STRING, FunctionConvertStringToNumber<std::uint32_t>());
    Convert<std::int64_t>::registerConversion(MetaTypeId::TYPE_STRING, FunctionConvertStringToNumber<std::int64_t>());
    Convert<std::uint64_t>::registerConversion(MetaTypeId::TYPE_STRING, FunctionConvertStringToNumber<std::uint64_t>());
    Convert<float>::registerConversion(MetaTypeId::TYPE_STRING, FunctionConvertStringToNumber<float>());
    Convert<double>::registerConversion(MetaTypeId::TYPE_STRING, FunctionConvertStringToNumber<double>());
}

} // namespace finalmq
