#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum MetaTypeId
{
    TYPE_NONE       = 0,

    TYPE_BOOL       = 1,
    TYPE_INT32      = 2,
    TYPE_UINT32     = 3,
    TYPE_INT64      = 4,
    TYPE_UINT64     = 5,
    TYPE_FLOAT      = 6,
    TYPE_DOUBLE     = 7,
    TYPE_STRING     = 8,
    TYPE_BYTES      = 9,
    TYPE_STRUCT     = 10,
    TYPE_ENUM       = 11,

    TYPE_ARRAY_FLAG       = 32,

    TYPE_ARRAY_BOOL       = 32 + 1,
    TYPE_ARRAY_INT32      = 32 + 2,
    TYPE_ARRAY_UINT32     = 32 + 3,
    TYPE_ARRAY_INT64      = 32 + 4,
    TYPE_ARRAY_UINT64     = 32 + 5,
    TYPE_ARRAY_FLOAT      = 32 + 6,
    TYPE_ARRAY_DOUBLE     = 32 + 7,
    TYPE_ARRAY_STRING     = 32 + 8,
    TYPE_ARRAY_BYTES      = 32 + 9,
    TYPE_ARRAY_STRUCT     = 32 + 10,
    TYPE_ARRAY_ENUM       = 32 + 11,
};


typedef char                       BytesElement;
typedef std::vector<BytesElement>  Bytes;



template <int TYPEID>
class MetaTypeIdInfo
{
};

template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_BOOL>
{
public:
    typedef bool Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_INT32>
{
public:
    typedef std::int32_t Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_UINT32>
{
public:
    typedef std::uint32_t Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_INT64>
{
public:
    typedef std::int64_t Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_UINT64>
{
public:
    typedef std::uint64_t Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_FLOAT>
{
public:
    typedef float Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_DOUBLE>
{
public:
    typedef double Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_STRING>
{
public:
    typedef std::string Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_BYTES>
{
public:
    typedef Bytes Type;
};

template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_ARRAY_BOOL>
{
public:
    typedef std::vector<bool> Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_ARRAY_INT32>
{
public:
    typedef std::vector<std::int32_t> Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_ARRAY_UINT32>
{
public:
    typedef std::vector<std::uint32_t> Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_ARRAY_INT64>
{
public:
    typedef std::vector<std::int64_t> Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_ARRAY_UINT64>
{
public:
    typedef std::vector<std::uint64_t> Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_ARRAY_FLOAT>
{
public:
    typedef std::vector<float> Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_ARRAY_DOUBLE>
{
public:
    typedef std::vector<double> Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_ARRAY_STRING>
{
public:
    typedef std::vector<std::string> Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_ARRAY_BYTES>
{
public:
    typedef std::vector<Bytes> Type;
};

////////////////////////////////////////////////////

template <class TYPE>
class MetaTypeInfo
{
};

template <>
class MetaTypeInfo<bool>
{
public:
    static const int TypeId = MetaTypeId::TYPE_BOOL;
};
template <>
class MetaTypeInfo<std::int32_t>
{
public:
    static const int TypeId = MetaTypeId::TYPE_INT32;
};
template <>
class MetaTypeInfo<std::uint32_t>
{
public:
    static const int TypeId = MetaTypeId::TYPE_UINT32;
};
template <>
class MetaTypeInfo<std::int64_t>
{
public:
    static const int TypeId = MetaTypeId::TYPE_INT64;
};
template <>
class MetaTypeInfo<std::uint64_t>
{
public:
    static const int TypeId = MetaTypeId::TYPE_UINT64;
};
template <>
class MetaTypeInfo<float>
{
public:
    static const int TypeId = MetaTypeId::TYPE_FLOAT;
};
template <>
class MetaTypeInfo<double>
{
public:
    static const int TypeId = MetaTypeId::TYPE_DOUBLE;
};
template <>
class MetaTypeInfo<std::string>
{
public:
    static const int TypeId = MetaTypeId::TYPE_STRING;
};
template <>
class MetaTypeInfo<Bytes>
{
public:
    static const int TypeId = MetaTypeId::TYPE_BYTES;
};

template <>
class MetaTypeInfo<std::vector<bool>>
{
public:
    static const int TypeId = MetaTypeId::TYPE_ARRAY_BOOL;
};
template <>
class MetaTypeInfo<std::vector<std::int32_t>>
{
public:
    static const int TypeId = MetaTypeId::TYPE_ARRAY_INT32;
};
template <>
class MetaTypeInfo<std::vector<std::uint32_t>>
{
public:
    static const int TypeId = MetaTypeId::TYPE_ARRAY_UINT32;
};
template <>
class MetaTypeInfo<std::vector<std::int64_t>>
{
public:
    static const int TypeId = MetaTypeId::TYPE_ARRAY_INT64;
};
template <>
class MetaTypeInfo<std::vector<std::uint64_t>>
{
public:
    static const int TypeId = MetaTypeId::TYPE_ARRAY_UINT64;
};
template <>
class MetaTypeInfo<std::vector<float>>
{
public:
    static const int TypeId = MetaTypeId::TYPE_ARRAY_FLOAT;
};
template <>
class MetaTypeInfo<std::vector<double>>
{
public:
    static const int TypeId = MetaTypeId::TYPE_ARRAY_DOUBLE;
};
template <>
class MetaTypeInfo<std::vector<std::string>>
{
public:
    static const int TypeId = MetaTypeId::TYPE_ARRAY_STRING;
};
template <>
class MetaTypeInfo<std::vector<Bytes>>
{
public:
    static const int TypeId = MetaTypeId::TYPE_ARRAY_BYTES;
};
