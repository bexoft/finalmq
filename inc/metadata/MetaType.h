#pragma once

#include <cstdint>
#include <string>

enum MetaType
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

template <int TYPE>
class MetaTypeInfo
{
};

template <>
class MetaTypeInfo<MetaType::TYPE_BOOL>
{
public:
    typedef bool Type;
};

template <>
class MetaTypeInfo<MetaType::TYPE_INT32>
{
public:
    typedef std::int32_t Type;
};

template <>
class MetaTypeInfo<MetaType::TYPE_UINT32>
{
public:
    typedef std::uint32_t Type;
};

template <>
class MetaTypeInfo<MetaType::TYPE_INT64>
{
public:
    typedef std::int64_t Type;
};

template <>
class MetaTypeInfo<MetaType::TYPE_UINT64>
{
public:
    typedef std::uint64_t Type;
};

template <>
class MetaTypeInfo<MetaType::TYPE_FLOAT>
{
public:
    typedef float Type;
};

template <>
class MetaTypeInfo<MetaType::TYPE_DOUBLE>
{
public:
    typedef double Type;
};

template <>
class MetaTypeInfo<MetaType::TYPE_STRING>
{
public:
    typedef std::string Type;
};

template <>
class MetaTypeInfo<MetaType::TYPE_BYTES>
{
public:
    typedef std::string Type;
};

