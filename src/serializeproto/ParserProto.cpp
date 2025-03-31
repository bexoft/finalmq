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

#include "finalmq/serializeproto/ParserProto.h"

#include <iostream>

#include <assert.h>
#include <memory.h>

#include "finalmq/helpers/FmqDefines.h"
#include "finalmq/metadata/MetaData.h"

namespace finalmq
{
static constexpr int INDEX2ID = 1;

ParserProto::ParserProto(IParserVisitor& visitor, const char* ptr, ssize_t size)
    : m_ptr(ptr), m_size(size), m_visitor(visitor)
{
}

template<class T>
bool ParserProto::parseValue(T& value, bool zz)
{
    bool ok = true;
    WireType wireType = static_cast<WireType>(m_tag & 0x7);
    m_tag = 0;
    switch(wireType)
    {
        case WIRETYPE_VARINT:
            value = static_cast<T>(parseVarint());
            if (zz)
            {
                value = static_cast<T>(zigzag(value));
            }
            break;
        case WIRETYPE_FIXED32:
            value = static_cast<T>(parseFixed<std::uint32_t>());
            break;
        case WIRETYPE_FIXED64:
            value = static_cast<T>(parseFixed<std::uint64_t>());
            break;
        default:
            skip(wireType);
            ok = false;
            break;
    }
    return ok;
}

//template<class T, class D>
//bool ParserProto::parseZigZagValue(T& value)
//{
//    bool ok = true;
//    WireType wireType = static_cast<WireType>(m_tag & 0x7);
//    m_tag = 0;
//    switch (wireType)
//    {
//    case WIRETYPE_VARINT:
//        {
//            D v = static_cast<D>(parseVarint());
//            value = zigzag(v);
//        }
//        break;
//    case WIRETYPE_FIXED32:
//        value = static_cast<T>(parseFixed<std::uint32_t>());
//        break;
//    case WIRETYPE_FIXED64:
//        value = static_cast<T>(parseFixed<std::uint64_t>());
//        break;
//    default:
//        skip(wireType);
//        ok = false;
//        break;
//    }
//    return ok;
//}

bool ParserProto::parseString(const char*& buffer, ssize_t& size)
{
    bool ok = true;
    buffer = nullptr;
    size = 0;
    WireType wireType = static_cast<WireType>(m_tag & 0x7);
    m_tag = 0;
    if (wireType == WIRETYPE_LENGTH_DELIMITED)
    {
        int sizeBuffer = static_cast<std::int32_t>(parseVarint());
        if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && m_ptr)
        {
            buffer = m_ptr;
            size = sizeBuffer;
            m_ptr += sizeBuffer;
            m_size -= sizeBuffer;
        }
        else
        {
            m_ptr = nullptr;
            m_size = 0;
            ok = false;
        }
    }
    else
    {
        skip(wireType);
        ok = false;
    }
    return ok;
}

void ParserProto::parseStructWire(const MetaField& field)
{
    WireType wireType = static_cast<WireType>(m_tag & 0x7);

    const MetaStruct* stru = MetaDataGlobal::instance().getStruct(field);
    if (!stru)
    {
        m_visitor.notifyError(m_ptr, "typename not found");
        skip(wireType);
        return;
    }

    m_tag = 0;
    if (wireType == WIRETYPE_LENGTH_DELIMITED)
    {
        int sizeBuffer = static_cast<std::int32_t>(parseVarint());
        if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && m_ptr)
        {
            m_visitor.enterStruct(field);
            ParserProto parser(m_visitor, m_ptr, sizeBuffer);
            bool res = parser.parseStructIntern(*stru);
            m_visitor.exitStruct(field);
            if (res)
            {
                m_ptr += sizeBuffer;
                m_size -= sizeBuffer;
            }
            else
            {
                m_ptr = nullptr;
                m_size = 0;
            }
        }
        else
        {
            m_ptr = nullptr;
            m_size = 0;
        }
    }
    else
    {
        skip(wireType);
    }
}

template<class T, int WIRETYPE>
bool ParserProto::parseFixedValue(T& value)
{
    bool ok = true;
    WireType wireType = static_cast<WireType>(m_tag & 0x7);
    m_tag = 0;
    if (wireType == WIRETYPE)
    {
        value = parseFixed<T>();
    }
    else
    {
        skip(wireType);
        ok = false;
    }
    return ok;
}

template<class T, int WIRETYPE>
bool ParserProto::parseArrayFixed(std::vector<T>& array)
{
    if (m_ptr == nullptr)
    {
        return false;
    }
    bool ok = true;
    WireType wireType = static_cast<WireType>(m_tag & 0x7);
    std::uint32_t tag = m_tag;
    m_tag = 0;
    switch(wireType)
    {
        case WIRETYPE:
            do
            {
                T value = parseFixed<T>();
                if (m_ptr)
                {
                    array.push_back(value);
                    if (m_size > 0)
                    {
                        m_tag = static_cast<std::int32_t>(parseVarint());
                    }
                    else
                    {
                        m_tag = 0;
                        break;
                    }
                }
                else
                {
                    m_ptr = nullptr;
                    m_size = 0;
                    ok = false;
                }
            } while ((m_tag == tag) && m_ptr);
            break;
        case WIRETYPE_LENGTH_DELIMITED:
        {
            int sizeBuffer = static_cast<std::int32_t>(parseVarint());
            if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && m_ptr)
            {
                ssize_t sizeElements = sizeBuffer / sizeof(T);
                array.resize(sizeElements);
#ifdef FINALMQ_LITTLE_ENDIAN
                memcpy(array.data(), m_ptr, sizeElements * sizeof(T));
#else
                for (ssize_t i = 0; i < sizeElements; i++)
                {
                    EndianHelper<sizeof(T)>::read(&m_ptr[i * sizeof(T)], &array[i]);
                }
#endif
                m_ptr += sizeBuffer;
                m_size -= sizeBuffer;
            }
            else
            {
                m_ptr = nullptr;
                m_size = 0;
                ok = false;
            }
        }
        break;
        default:
            skip(wireType);
            ok = false;
            break;
    }

    return ok;
}

template<class T, bool ZIGZAG>
bool ParserProto::parseArrayVarint(std::vector<T>& array)
{
    bool ok = true;
    WireType wireType = static_cast<WireType>(m_tag & 0x7);
    std::uint32_t tag = m_tag;
    m_tag = 0;
    switch(wireType)
    {
        case WIRETYPE_VARINT:
            do
            {
                std::uint64_t value = parseVarint();
                if (m_ptr)
                {
                    T v = (ZIGZAG) ? static_cast<T>(zigzag(value)) : static_cast<T>(value);
                    array.push_back(v);
                    if (m_size > 0)
                    {
                        m_tag = static_cast<std::uint32_t>(parseVarint());
                    }
                    else
                    {
                        m_tag = 0;
                        break;
                    }
                }
                else
                {
                    m_ptr = nullptr;
                    m_size = 0;
                    ok = false;
                }
            } while ((m_tag == tag) && m_ptr);
            break;
        case WIRETYPE_LENGTH_DELIMITED:
        {
            int sizeBuffer = static_cast<std::int32_t>(parseVarint());
            if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && m_ptr)
            {
                int sizeEnd = static_cast<int>(m_size - sizeBuffer);
                sizeEnd = std::max(sizeEnd, 0);
                while (m_size > sizeEnd)
                {
                    std::uint64_t value = parseVarint();
                    T v = (ZIGZAG) ? static_cast<T>(zigzag(value)) : static_cast<T>(value);
                    array.push_back(v);
                }
            }
            else
            {
                m_ptr = nullptr;
                m_size = 0;
                ok = false;
            }
        }
        break;
        default:
            skip(wireType);
            ok = false;
            break;
    }

    return ok;
}

template<class T>
bool ParserProto::parseArrayString(std::vector<T>& array)
{
    if (m_ptr == nullptr)
    {
        return false;
    }
    bool ok = true;
    WireType wireType = static_cast<WireType>(m_tag & 0x7);
    const std::uint32_t tag = m_tag;
    m_tag = 0;
    if (wireType == WIRETYPE_LENGTH_DELIMITED)
    {
        do
        {
            int sizeBuffer = static_cast<std::int32_t>(parseVarint());
            if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && m_ptr)
            {
                array.emplace_back(m_ptr, m_ptr + sizeBuffer);
                m_ptr += sizeBuffer;
                m_size -= sizeBuffer;
                if (m_size > 0)
                {
                    m_tag = static_cast<std::uint32_t>(parseVarint());
                }
                else
                {
                    m_tag = 0;
                    break;
                }
            }
            else
            {
                m_ptr = nullptr;
                m_size = 0;
                ok = false;
                break;
            }
        } while ((m_tag == tag) && m_ptr);
    }
    else
    {
        skip(wireType);
        ok = false;
    }

    return ok;
}

void ParserProto::parseArrayStruct(const MetaField& field)
{
    if (m_ptr == nullptr)
    {
        return;
    }

    WireType wireType = static_cast<WireType>(m_tag & 0x7);

    const MetaStruct* stru = MetaDataGlobal::instance().getStruct(field);
    if (!stru)
    {
        m_visitor.notifyError(m_ptr, "typename not found");
        skip(wireType);
        return;
    }

    const std::uint32_t tag = m_tag;
    m_tag = 0;
    if (wireType == WIRETYPE_LENGTH_DELIMITED)
    {
        const MetaField* fieldWithoutArray = field.fieldWithoutArray;
        assert(fieldWithoutArray);
        m_visitor.enterArrayStruct(field);
        do
        {
            int sizeBuffer = static_cast<std::int32_t>(parseVarint());
            if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && m_ptr)
            {
                m_visitor.enterStruct(*fieldWithoutArray);
                ParserProto parser(m_visitor, m_ptr, sizeBuffer);
                bool res = parser.parseStructIntern(*stru);
                m_visitor.exitStruct(*fieldWithoutArray);
                if (res)
                {
                    m_ptr += sizeBuffer;
                    m_size -= sizeBuffer;
                }
                else
                {
                    m_ptr = nullptr;
                    m_size = 0;
                    break;
                }

                if (m_size > 0)
                {
                    m_tag = static_cast<std::uint32_t>(parseVarint());
                }
                else
                {
                    m_tag = 0;
                    break;
                }
            }
            else
            {
                m_ptr = nullptr;
                m_size = 0;
                break;
            }
        } while ((m_tag == tag) && m_ptr);
        m_visitor.exitArrayStruct(field);
    }
    else
    {
        skip(wireType);
    }
}

bool ParserProto::parseStruct(const std::string& typeName)
{
    static const char EMPTY[1] = { 0 };
    if (m_size == 0 && m_ptr == nullptr)
    {
        m_ptr = EMPTY;
    }

    if (!m_ptr || m_size < 0)
    {
        // end of data
        return false;
    }

    const MetaStruct* stru = MetaDataGlobal::instance().getStruct(typeName);
    if (!stru)
    {
        m_visitor.notifyError(m_ptr, "typename not found");
        return false;
    }

    m_visitor.startStruct(*stru);
    bool res = parseStructIntern(*stru);
    m_visitor.finished();
    return res;
}

bool ParserProto::parseStructIntern(const MetaStruct& stru)
{
    if (!m_ptr || m_size < 0)
    {
        // end of data
        return false;
    }

    while (m_size > 0)
    {
        if (m_tag == 0)
        {
            m_tag = static_cast<std::uint32_t>(parseVarint());
        }
        if (m_ptr)
        {
            int id = m_tag >> 3;
            int index = id - INDEX2ID;
            const MetaField* field = stru.getFieldByIndex(index);
            if (field)
            {
                switch(field->typeId)
                {
                    case MetaTypeId::TYPE_NONE:
                        break;
                    case MetaTypeId::TYPE_BOOL:
                    {
                        bool value = false;
                        bool ok = parseValue(value, false);
                        if (ok)
                        {
                            m_visitor.enterBool(*field, value);
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_INT8:
                    {
                        std::int32_t value = 0;
                        bool zz = (field->flags & METAFLAG_PROTO_ZIGZAG);
                        bool ok = parseValue(value, zz);
                        if (ok)
                        {
                            m_visitor.enterInt8(*field, static_cast<std::int8_t>(value));
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_UINT8:
                    {
                        std::uint32_t value = 0;
                        bool ok = parseValue(value, false);
                        if (ok)
                        {
                            m_visitor.enterUInt8(*field, static_cast<std::uint8_t>(value));
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_INT16:
                    {
                        std::int32_t value = 0;
                        bool zz = (field->flags & METAFLAG_PROTO_ZIGZAG);
                        bool ok = parseValue(value, zz);
                        if (ok)
                        {
                            m_visitor.enterInt16(*field, static_cast<std::int16_t>(value));
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_UINT16:
                    {
                        std::uint32_t value = 0;
                        bool ok = parseValue(value, false);
                        if (ok)
                        {
                            m_visitor.enterUInt16(*field, static_cast<std::uint16_t>(value));
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_INT32:
                    {
                        std::int32_t value = 0;
                        bool zz = (field->flags & METAFLAG_PROTO_ZIGZAG);
                        bool ok = parseValue(value, zz);
                        if (ok)
                        {
                            m_visitor.enterInt32(*field, value);
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_UINT32:
                    {
                        std::uint32_t value = 0;
                        bool ok = parseValue(value, false);
                        if (ok)
                        {
                            m_visitor.enterUInt32(*field, value);
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_INT64:
                    {
                        std::int64_t value = 0;
                        bool zz = (field->flags & METAFLAG_PROTO_ZIGZAG);
                        bool ok = parseValue(value, zz);
                        if (ok)
                        {
                            m_visitor.enterInt64(*field, value);
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_UINT64:
                    {
                        std::uint64_t value = 0;
                        bool ok = parseValue(value, false);
                        if (ok)
                        {
                            m_visitor.enterUInt64(*field, value);
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_FLOAT:
                    {
                        float value = 0.0;
                        bool ok = parseFixedValue<float, WIRETYPE_FIXED32>(value);
                        if (ok)
                        {
                            m_visitor.enterFloat(*field, value);
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_DOUBLE:
                    {
                        double value = 0.0;
                        bool ok = parseFixedValue<double, WIRETYPE_FIXED64>(value);
                        if (ok)
                        {
                            m_visitor.enterDouble(*field, value);
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_STRING:
                    {
                        const char* buffer = nullptr;
                        ssize_t size = 0;
                        bool ok = parseString(buffer, size);
                        if (ok)
                        {
                            m_visitor.enterString(*field, buffer, size);
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_BYTES:
                    {
                        const char* buffer = nullptr;
                        ssize_t size = 0;
                        bool ok = parseString(buffer, size);
                        if (ok)
                        {
                            m_visitor.enterBytes(*field, buffer, size);
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_STRUCT:
                        parseStructWire(*field);
                        break;
                    case MetaTypeId::TYPE_ENUM:
                    {
                        std::int32_t value = 0;
                        bool ok = parseValue(value, false);
                        if (ok)
                        {
                            m_visitor.enterEnum(*field, value);
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_JSON:
                    {
                        const char* buffer = nullptr;
                        ssize_t size = 0;
                        bool ok = parseString(buffer, size);
                        if (ok)
                        {
                            m_visitor.enterJsonString(*field, buffer, size);
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_ARRAY_BOOL:
                    {
                        std::vector<bool> array;
                        bool ok = parseArrayVarint(array);
                        if (ok)
                        {
                            m_visitor.enterArrayBool(*field, std::move(array));
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_ARRAY_INT8:
                    {
                        std::vector<std::int32_t> array;
                        bool ok = false;
                        if (field->flags & METAFLAG_PROTO_VARINT)
                        {
                            ok = parseArrayVarint(array);
                        }
                        else if (field->flags & METAFLAG_PROTO_ZIGZAG)
                        {
                            ok = parseArrayVarint<std::int32_t, true>(array);
                        }
                        else
                        {
                            ok = parseArrayFixed<std::int32_t, WIRETYPE_FIXED32>(array);
                        }
                        if (ok)
                        {
                            std::vector<std::int8_t> arrayInt8;
                            arrayInt8.resize(array.size());
                            for (size_t i = 0; i < array.size(); ++i)
                            {
                                arrayInt8[i] = static_cast<std::int8_t>(array[i]);
                            }
                            m_visitor.enterArrayInt8(*field, std::move(arrayInt8));
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_ARRAY_INT16:
                    {
                        std::vector<std::int32_t> array;
                        bool ok = false;
                        if (field->flags & METAFLAG_PROTO_VARINT)
                        {
                            ok = parseArrayVarint(array);
                        }
                        else if (field->flags & METAFLAG_PROTO_ZIGZAG)
                        {
                            ok = parseArrayVarint<std::int32_t, true>(array);
                        }
                        else
                        {
                            ok = parseArrayFixed<std::int32_t, WIRETYPE_FIXED32>(array);
                        }
                        if (ok)
                        {
                            std::vector<std::int16_t> arrayInt16;
                            arrayInt16.resize(array.size());
                            for (size_t i = 0; i < array.size(); ++i)
                            {
                                arrayInt16[i] = static_cast<std::int16_t>(array[i]);
                            }
                            m_visitor.enterArrayInt16(*field, std::move(arrayInt16));
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_ARRAY_UINT16:
                    {
                        std::vector<std::uint32_t> array;
                        bool ok = false;
                        if (field->flags & METAFLAG_PROTO_VARINT)
                        {
                            ok = parseArrayVarint(array);
                        }
                        else
                        {
                            ok = parseArrayFixed<std::uint32_t, WIRETYPE_FIXED32>(array);
                        }
                        if (ok)
                        {
                            std::vector<std::uint16_t> arrayUInt16;
                            arrayUInt16.resize(array.size());
                            for (size_t i = 0; i < array.size(); ++i)
                            {
                                arrayUInt16[i] = static_cast<std::uint16_t>(array[i]);
                            }
                            m_visitor.enterArrayUInt16(*field, std::move(arrayUInt16));
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_ARRAY_INT32:
                    {
                        std::vector<std::int32_t> array;
                        bool ok = false;
                        if (field->flags & METAFLAG_PROTO_VARINT)
                        {
                            ok = parseArrayVarint(array);
                        }
                        else if (field->flags & METAFLAG_PROTO_ZIGZAG)
                        {
                            ok = parseArrayVarint<std::int32_t, true>(array);
                        }
                        else
                        {
                            ok = parseArrayFixed<std::int32_t, WIRETYPE_FIXED32>(array);
                        }
                        if (ok)
                        {
                            m_visitor.enterArrayInt32(*field, std::move(array));
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_ARRAY_UINT32:
                    {
                        std::vector<std::uint32_t> array;
                        bool ok = false;
                        if (field->flags & METAFLAG_PROTO_VARINT)
                        {
                            ok = parseArrayVarint(array);
                        }
                        else
                        {
                            ok = parseArrayFixed<std::uint32_t, WIRETYPE_FIXED32>(array);
                        }
                        if (ok)
                        {
                            m_visitor.enterArrayUInt32(*field, std::move(array));
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_ARRAY_INT64:
                    {
                        std::vector<std::int64_t> array;
                        bool ok = false;
                        if (field->flags & METAFLAG_PROTO_VARINT)
                        {
                            ok = parseArrayVarint(array);
                        }
                        else if (field->flags & METAFLAG_PROTO_ZIGZAG)
                        {
                            ok = parseArrayVarint<std::int64_t, true>(array);
                        }
                        else
                        {
                            ok = parseArrayFixed<std::int64_t, WIRETYPE_FIXED64>(array);
                        }
                        if (ok)
                        {
                            m_visitor.enterArrayInt64(*field, std::move(array));
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_ARRAY_UINT64:
                    {
                        std::vector<std::uint64_t> array;
                        bool ok = false;
                        if (field->flags & METAFLAG_PROTO_VARINT)
                        {
                            ok = parseArrayVarint(array);
                        }
                        else
                        {
                            ok = parseArrayFixed<std::uint64_t, WIRETYPE_FIXED64>(array);
                        }
                        if (ok)
                        {
                            m_visitor.enterArrayUInt64(*field, std::move(array));
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_ARRAY_FLOAT:
                    {
                        std::vector<float> array;
                        bool ok = parseArrayFixed<float, WIRETYPE_FIXED32>(array);
                        if (ok)
                        {
                            m_visitor.enterArrayFloat(*field, std::move(array));
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_ARRAY_DOUBLE:
                    {
                        std::vector<double> array;
                        bool ok = parseArrayFixed<double, WIRETYPE_FIXED64>(array);
                        if (ok)
                        {
                            m_visitor.enterArrayDouble(*field, std::move(array));
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_ARRAY_STRING:
                    {
                        std::vector<std::string> array;
                        bool ok = parseArrayString(array);
                        if (ok)
                        {
                            m_visitor.enterArrayStringMove(*field, std::move(array));
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_ARRAY_BYTES:
                    {
                        std::vector<Bytes> array;
                        bool ok = parseArrayString(array);
                        if (ok)
                        {
                            m_visitor.enterArrayBytesMove(*field, std::move(array));
                        }
                    }
                    break;
                    case MetaTypeId::TYPE_ARRAY_STRUCT:
                        parseArrayStruct(*field);
                        break;
                    case MetaTypeId::TYPE_ARRAY_ENUM:
                    {
                        std::vector<std::int32_t> array;
                        bool ok = parseArrayVarint(array);
                        if (ok)
                        {
                            m_visitor.enterArrayEnum(*field, std::move(array));
                        }
                    }
                    break;
                    case MetaTypeId::OFFSET_ARRAY_FLAG:
                        assert(false);
                        break;
                    default:
                        assert(false);
                        break;
                }
            }
            else
            {
                WireType wireType = static_cast<WireType>(m_tag & 0x7);
                skip(wireType);
                m_tag = 0;
            }
        }
    }

    return (m_ptr != nullptr);
}

std::uint64_t ParserProto::parseVarint()
{
    std::uint64_t res = 0;
    if (m_size <= 0)
    {
        m_ptr = nullptr;
        m_size = 0;
        return 0;
    }
    std::uint64_t c = static_cast<std::uint8_t>(*m_ptr);
    res = c;
    ++m_ptr;
    --m_size;
    if (c < 128)
    {
        return res;
    }
    for (int shift = 7; shift < 70; shift += 7)
    {
        if (m_size <= 0)
        {
            break;
        }
        c = static_cast<std::uint8_t>(*m_ptr);
        res += (c - 1) << shift;
        ++m_ptr;
        --m_size;
        if (c < 128)
        {
            return res;
        }
    }
    m_ptr = nullptr;
    m_size = 0;
    return 0;
}

template<class T>
T ParserProto::parseFixed()
{
    T value = 0;
    if (m_size >= static_cast<ssize_t>(sizeof(T)))
    {
        EndianHelper<static_cast<int>(sizeof(T))>::read(m_ptr, value);
        m_ptr += sizeof(T);
        m_size -= sizeof(T);
    }
    else
    {
        m_ptr = nullptr;
        m_size = 0;
    }
    return value;
}

//std::int32_t ParserProto::zigzag(std::uint32_t value)
//{
//    return static_cast<std::int32_t>((value >> 1) ^ (~(value & 1) + 1));
//}

std::int64_t ParserProto::zigzag(std::uint64_t value)
{
    return static_cast<std::int64_t>((value >> 1) ^ (~(value & 1) + 1));
}

void ParserProto::skip(WireType wireType)
{
    switch(wireType)
    {
        case WIRETYPE_VARINT:
            parseVarint();
            break;
        case WIRETYPE_FIXED64:
            if (m_ptr)
            {
                m_ptr += sizeof(std::uint64_t);
                m_size -= sizeof(std::uint64_t);
            }
            break;
        case WIRETYPE_LENGTH_DELIMITED:
        {
            std::uint64_t len = parseVarint();
            if (m_ptr)
            {
                m_ptr += len;
                m_size -= len;
            }
        }
        break;
        case WIRETYPE_FIXED32:
            if (m_ptr)
            {
                m_ptr += sizeof(std::uint32_t);
                m_size -= sizeof(std::uint32_t);
            }
            break;
        default:
            m_ptr = nullptr;
            m_size = 0;
            break;
    }
    if (m_size < 0)
    {
        m_ptr = nullptr;
        m_size = 0;
    }
}

} // namespace finalmq
