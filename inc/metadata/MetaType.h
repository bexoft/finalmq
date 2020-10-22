#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum MetaTypeId : std::int32_t
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

    OFFSET_ARRAY_FLAG       = 1024,

    TYPE_ARRAY_BOOL       = OFFSET_ARRAY_FLAG + TYPE_BOOL,
    TYPE_ARRAY_INT32      = OFFSET_ARRAY_FLAG + TYPE_INT32,
    TYPE_ARRAY_UINT32     = OFFSET_ARRAY_FLAG + TYPE_UINT32,
    TYPE_ARRAY_INT64      = OFFSET_ARRAY_FLAG + TYPE_INT64,
    TYPE_ARRAY_UINT64     = OFFSET_ARRAY_FLAG + TYPE_UINT64,
    TYPE_ARRAY_FLOAT      = OFFSET_ARRAY_FLAG + TYPE_FLOAT,
    TYPE_ARRAY_DOUBLE     = OFFSET_ARRAY_FLAG + TYPE_DOUBLE,
    TYPE_ARRAY_STRING     = OFFSET_ARRAY_FLAG + TYPE_STRING,
    TYPE_ARRAY_BYTES      = OFFSET_ARRAY_FLAG + TYPE_BYTES,
    TYPE_ARRAY_STRUCT     = OFFSET_ARRAY_FLAG + TYPE_STRUCT,
    TYPE_ARRAY_ENUM       = OFFSET_ARRAY_FLAG + TYPE_ENUM,
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
