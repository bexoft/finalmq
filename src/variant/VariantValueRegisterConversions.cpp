

#include "variant/VariantValueRegisterConversions.h"

#include "variant/VariantValues.h"
#include "conversions/Conversions.h"



static VariantValueRegisterConversions g_registerConversions;


template<class FROM, class TO>
class FunctionConvert
{
public:
    TO operator ()(const Variant& variant)
    {
        assert(variant.getType() == MetaTypeInfo<FROM>::TypeId);
        const FROM* data = variant;
        assert(data);
        return static_cast<TO>(*data);
    }
};


template<class TO>
class FunctionConvertStringToNumber
{
public:
    TO operator ()(const Variant& variant)
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
    std::string operator ()(const Variant& variant)
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
    std::string operator ()(const Variant& variant)
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
    registerConversionsTo<std::int32_t>();
    registerConversionsTo<std::uint32_t>();
    registerConversionsTo<std::int64_t>();
    registerConversionsTo<std::uint64_t>();
    registerConversionsTo<float>();
    registerConversionsTo<double>();

    FunctionConvertNumberToString<bool>();
    FunctionConvertNumberToString<std::int32_t>();
    FunctionConvertNumberToString<std::uint32_t>();
    FunctionConvertNumberToString<std::int64_t>();
    FunctionConvertNumberToString<std::uint64_t>();
    FunctionConvertNumberToString<float>();
    FunctionConvertNumberToString<double>();

    FunctionConvertStringToNumber<bool>();
    FunctionConvertStringToNumber<std::int32_t>();
    FunctionConvertStringToNumber<std::uint32_t>();
    FunctionConvertStringToNumber<std::int64_t>();
    FunctionConvertStringToNumber<std::uint64_t>();
    FunctionConvertStringToNumber<float>();
    FunctionConvertStringToNumber<double>();
}

