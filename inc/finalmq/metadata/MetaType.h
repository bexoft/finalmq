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

#include <cstdint>
#include <string>
#include <vector>


namespace finalmq {


enum MetaTypeId : std::int32_t
{
    TYPE_NONE       = 0,

    TYPE_BOOL       = 1,
    TYPE_INT8       = 2,
    TYPE_UINT8      = 3,
    TYPE_INT16      = 4,
    TYPE_UINT16     = 5,
    TYPE_INT32      = 6,
    TYPE_UINT32     = 7,
    TYPE_INT64      = 8,
    TYPE_UINT64     = 9,
    TYPE_FLOAT      = 10,
    TYPE_DOUBLE     = 11,
    TYPE_STRING     = 12,
    TYPE_BYTES      = 13,
    TYPE_STRUCT     = 14,
    TYPE_ENUM       = 15,
    TYPE_VARIANT    = 16,
    TYPE_JSON       = 17,

    OFFSET_ARRAY_FLAG       = 1024,

    TYPE_ARRAY_BOOL       = OFFSET_ARRAY_FLAG + TYPE_BOOL,
    TYPE_ARRAY_INT8       = OFFSET_ARRAY_FLAG + TYPE_INT8,
// use Bytes    TYPE_ARRAY_UINT8      = OFFSET_ARRAY_FLAG + TYPE_UINT8,
    TYPE_ARRAY_INT16      = OFFSET_ARRAY_FLAG + TYPE_INT16,
    TYPE_ARRAY_UINT16     = OFFSET_ARRAY_FLAG + TYPE_UINT16,
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
class MetaTypeIdInfo<MetaTypeId::TYPE_INT8>
{
public:
    typedef std::int8_t Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_UINT8>
{
public:
    typedef std::uint8_t Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_INT16>
{
public:
    typedef std::int16_t Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_UINT16>
{
public:
    typedef std::uint16_t Type;
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
class MetaTypeIdInfo<MetaTypeId::TYPE_ARRAY_INT8>
{
public:
    typedef std::vector<std::int8_t> Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_ARRAY_INT16>
{
public:
    typedef std::vector<std::int16_t> Type;
};
template <>
class MetaTypeIdInfo<MetaTypeId::TYPE_ARRAY_UINT16>
{
public:
    typedef std::vector<std::uint16_t> Type;
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

template <class TYPE, typename Enable = void>
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
class MetaTypeInfo<std::int8_t>
{
public:
    static const int TypeId = MetaTypeId::TYPE_INT8;
};
template <>
class MetaTypeInfo<std::uint8_t>
{
public:
    static const int TypeId = MetaTypeId::TYPE_UINT8;
};
template <>
class MetaTypeInfo<std::int16_t>
{
public:
    static const int TypeId = MetaTypeId::TYPE_INT16;
};
template <>
class MetaTypeInfo<std::uint16_t>
{
public:
    static const int TypeId = MetaTypeId::TYPE_UINT16;
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
class MetaTypeInfo<std::vector<std::int8_t>>
{
public:
    static const int TypeId = MetaTypeId::TYPE_ARRAY_INT8;
};
template <>
class MetaTypeInfo<std::vector<std::int16_t>>
{
public:
    static const int TypeId = MetaTypeId::TYPE_ARRAY_INT16;
};
template <>
class MetaTypeInfo<std::vector<std::uint16_t>>
{
public:
    static const int TypeId = MetaTypeId::TYPE_ARRAY_UINT16;
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

}   // namespace finalmq
