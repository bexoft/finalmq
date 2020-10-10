#pragma once

#include "VariantValuePrimitive.h"
#include "Variant.h"
#include "metadata/MetaType.h"

#include <unordered_map>
#include <functional>




/////////////////////////////////////////
typedef VariantValuePrimitive<TYPE_BOOL>   VariantValueBool;
template<>
class VariantValueTypeInfo<bool>
{
public:
    typedef bool T;
    typedef VariantValueBool VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValuePrimitive<TYPE_INT32>   VariantValueInt32;
template<>
class VariantValueTypeInfo<std::int32_t>
{
public:
    typedef std::int32_t T;
    typedef VariantValueBool VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValuePrimitive<TYPE_UINT32>  VariantValueUInt32;
template<>
class VariantValueTypeInfo<std::uint32_t>
{
public:
    typedef std::uint32_t T;
    typedef VariantValueBool VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValuePrimitive<TYPE_INT64>   VariantValueInt64;
template<>
class VariantValueTypeInfo<std::int64_t>
{
public:
    typedef std::int64_t T;
    typedef VariantValueBool VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValuePrimitive<TYPE_UINT64>  VariantValueUInt64;
template<>
class VariantValueTypeInfo<std::uint64_t>
{
public:
    typedef std::uint64_t T;
    typedef VariantValueBool VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValuePrimitive<TYPE_FLOAT>  VariantValueFloat;
template<>
class VariantValueTypeInfo<float>
{
public:
    typedef float T;
    typedef VariantValueBool VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValuePrimitive<TYPE_DOUBLE>  VariantValueDouble;
template<>
class VariantValueTypeInfo<double>
{
public:
    typedef double T;
    typedef VariantValueBool VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValuePrimitive<TYPE_STRING>  VariantValueString;
template<>
class VariantValueTypeInfo<std::string>
{
public:
    typedef std::string T;
    typedef VariantValueBool VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValuePrimitive<TYPE_BYTES>  VariantValueBytes;
template<>
class VariantValueTypeInfo<Bytes>
{
public:
    typedef Bytes T;
    typedef VariantValueBool VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};

