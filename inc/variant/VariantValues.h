#pragma once

#include "VariantValueTemplate.h"
#include "metadata/MetaType.h"



/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_BOOL>   VariantValueBool;
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
typedef VariantValueTemplate<TYPE_INT32>   VariantValueInt32;
template<>
class VariantValueTypeInfo<std::int32_t>
{
public:
    typedef std::int32_t T;
    typedef VariantValueInt32 VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_UINT32>  VariantValueUInt32;
template<>
class VariantValueTypeInfo<std::uint32_t>
{
public:
    typedef std::uint32_t T;
    typedef VariantValueUInt32 VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_INT64>   VariantValueInt64;
template<>
class VariantValueTypeInfo<std::int64_t>
{
public:
    typedef std::int64_t T;
    typedef VariantValueInt64 VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_UINT64>  VariantValueUInt64;
template<>
class VariantValueTypeInfo<std::uint64_t>
{
public:
    typedef std::uint64_t T;
    typedef VariantValueUInt64 VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_FLOAT>  VariantValueFloat;
template<>
class VariantValueTypeInfo<float>
{
public:
    typedef float T;
    typedef VariantValueFloat VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_DOUBLE>  VariantValueDouble;
template<>
class VariantValueTypeInfo<double>
{
public:
    typedef double T;
    typedef VariantValueDouble VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_STRING>  VariantValueString;
template<>
class VariantValueTypeInfo<std::string>
{
public:
    typedef std::string T;
    typedef VariantValueString VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_BYTES>  VariantValueBytes;
template<>
class VariantValueTypeInfo<Bytes>
{
public:
    typedef Bytes T;
    typedef VariantValueBytes VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};

/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_ARRAY_BOOL>   VariantValueArrayBool;
template<>
class VariantValueTypeInfo<std::vector<bool>>
{
public:
    typedef std::vector<bool> T;
    typedef VariantValueArrayBool VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_ARRAY_INT32>   VariantValueArrayInt32;
template<>
class VariantValueTypeInfo<std::vector<std::int32_t>>
{
public:
    typedef std::vector<std::int32_t> T;
    typedef VariantValueArrayInt32 VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_ARRAY_UINT32>  VariantValueArrayUInt32;
template<>
class VariantValueTypeInfo<std::vector<std::uint32_t>>
{
public:
    typedef std::vector<std::uint32_t> T;
    typedef VariantValueArrayUInt32 VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_ARRAY_INT64>   VariantValueArrayInt64;
template<>
class VariantValueTypeInfo<std::vector<std::int64_t>>
{
public:
    typedef std::vector<std::int64_t> T;
    typedef VariantValueArrayInt64 VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_ARRAY_UINT64>  VariantValueArrayUInt64;
template<>
class VariantValueTypeInfo<std::vector<std::uint64_t>>
{
public:
    typedef std::vector<std::uint64_t> T;
    typedef VariantValueArrayUInt64 VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_ARRAY_FLOAT>  VariantValueArrayFloat;
template<>
class VariantValueTypeInfo<std::vector<float>>
{
public:
    typedef std::vector<float> T;
    typedef VariantValueArrayFloat VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_ARRAY_DOUBLE>  VariantValueArrayDouble;
template<>
class VariantValueTypeInfo<std::vector<double>>
{
public:
    typedef std::vector<double> T;
    typedef VariantValueArrayDouble VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_ARRAY_STRING>  VariantValueArrayString;
template<>
class VariantValueTypeInfo<std::vector<std::string>>
{
public:
    typedef std::vector<std::string> T;
    typedef VariantValueArrayString VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};
/////////////////////////////////////////
typedef VariantValueTemplate<TYPE_ARRAY_BYTES>  VariantValueArrayBytes;
template<>
class VariantValueTypeInfo<std::vector<Bytes>>
{
public:
    typedef std::vector<Bytes> T;
    typedef VariantValueArrayBytes VariantValueType;
    const static int VARTYPE = MetaTypeInfo<T>::TypeId;
    typedef Convert<T> ConvertType;
};

