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


#include "serializeproto/ParserProto.h"
#include "metadata/MetaData.h"
#include "helpers/BexDefines.h"


#include <assert.h>
#include <memory.h>
#include <iostream>



namespace finalmq {

static constexpr int INDEX2ID = 1;


ParserProto::ParserProto(IParserVisitor& visitor, const char* ptr, ssize_t size)
    : m_ptr(ptr)
    , m_size(size)
    , m_visitor(visitor)
{
}


template<class T>
bool ParserProto::parseVarint(T& value)
{
    bool ok = true;
    WireType wireType = static_cast<WireType>(m_tag & 0x7);
    m_tag = 0;
    switch (wireType)
    {
    case WIRETYPE_VARINT:
        value = static_cast<T>(parseVarint());
        break;
    default:
        skip(wireType);
        ok = false;
        break;
    }
    return ok;
}


template<class T, class D>
bool ParserProto::parseZigZag(T& value)
{
    bool ok = true;
    WireType wireType = static_cast<WireType>(m_tag & 0x7);
    m_tag = 0;
    switch (wireType)
    {
    case WIRETYPE_VARINT:
        {
            D v = static_cast<D>(parseVarint());
            value = zigzag(v);
        }
        break;
    default:
        skip(wireType);
        ok = false;
        break;
    }
    return ok;
}



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
        if (m_ptr)
        {
            if (sizeBuffer >= 0 && sizeBuffer <= m_size)
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
            }
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
        if (m_ptr && sizeBuffer > 0)
        {
            if (sizeBuffer >= 0 && sizeBuffer <= m_size)
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
    bool ok = true;
    WireType wireType = static_cast<WireType>(m_tag & 0x7);
    std::uint32_t tag = m_tag;
    m_tag = 0;
    switch (wireType)
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
        } while ((m_tag == tag) && m_ptr);
        break;
    case WIRETYPE_LENGTH_DELIMITED:
        {
            int sizeBuffer = static_cast<std::int32_t>(parseVarint());
            if (m_ptr)
            {
                if (sizeBuffer >= 0 && sizeBuffer <= m_size)
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
                }
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
    switch (wireType)
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
        } while ((m_tag == tag) && m_ptr);
        break;
    case WIRETYPE_LENGTH_DELIMITED:
        {
            int sizeBuffer = static_cast<std::int32_t>(parseVarint());
            if (m_ptr)
            {
                if (sizeBuffer >= 0 && sizeBuffer <= m_size)
                {
                    while (m_size > 0)
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
                }
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
    bool ok = true;
    WireType wireType = static_cast<WireType>(m_tag & 0x7);
    std::uint32_t tag = m_tag;
    m_tag = 0;
    if (wireType == WIRETYPE_LENGTH_DELIMITED)
    {
        do
        {
            int sizeBuffer = static_cast<std::int32_t>(parseVarint());
            if (m_ptr)
            {
                if (sizeBuffer >= 0 && sizeBuffer <= m_size)
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
                }
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
    WireType wireType = static_cast<WireType>(m_tag & 0x7);

    const MetaStruct* stru = MetaDataGlobal::instance().getStruct(field);
    if (!stru)
    {
        m_visitor.notifyError(m_ptr, "typename not found");
        skip(wireType);
        return;
    }

    std::uint32_t tag = m_tag;
    m_tag = 0;
    if (wireType == WIRETYPE_LENGTH_DELIMITED)
    {

        const MetaField* fieldWithoutArray = MetaDataGlobal::instance().getArrayField(field);
        assert(fieldWithoutArray);
        m_visitor.enterArrayStruct(field);
        do
        {
            int sizeBuffer = static_cast<std::int32_t>(parseVarint());
            if (m_ptr && (sizeBuffer >= 0 && sizeBuffer <= m_size))
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
    assert(m_ptr);
    assert(m_size >= 0);

    const MetaStruct* stru = MetaDataGlobal::instance().getStruct(typeName);
    if (!stru)
    {
        m_visitor.notifyError(m_ptr, "typename not found");
        m_visitor.finished();
        return false;
    }

    MetaField field{MetaTypeId::TYPE_STRUCT, typeName};
    field.metaStruct = stru;
    m_visitor.enterStruct(field);
    bool res = parseStructIntern(*stru);
    m_visitor.exitStruct(field);
    m_visitor.finished();
    return res;
}


bool ParserProto::parseStructIntern(const MetaStruct& stru)
{
    assert(m_ptr);
    assert(m_size >= 0);

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
                switch (field->typeId)
                {
                case MetaTypeId::TYPE_NONE:
                    break;
                case MetaTypeId::TYPE_BOOL:
                    {
                        bool value = 0;
                        bool ok = parseVarint(value);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterBool(*field, value);
                        }
                    }
                    break;
                case MetaTypeId::TYPE_INT32:
                    {
                        std::int32_t value = 0;
                        bool ok = false;
                        if (field->flags & METAFLAG_PROTO_VARINT)
                        {
                            ok = parseVarint(value);
                        }
                        else if (field->flags & METAFLAG_PROTO_ZIGZAG)
                        {
                            ok = parseZigZag<std::int32_t, std::uint32_t>(value);
                        }
                        else
                        {
                            ok = parseFixedValue<std::int32_t, WIRETYPE_FIXED32>(value);
                        }
                        if (ok && m_ptr)
                        {
                            m_visitor.enterInt32(*field, value);
                        }
                    }
                    break;
                case MetaTypeId::TYPE_UINT32:
                    {
                        std::uint32_t value = 0;
                        bool ok = false;
                        if (field->flags & METAFLAG_PROTO_VARINT)
                        {
                            ok = parseVarint(value);
                        }
                        else
                        {
                            ok = parseFixedValue<std::uint32_t, WIRETYPE_FIXED32>(value);
                        }
                        if (ok && m_ptr)
                        {
                            m_visitor.enterUInt32(*field, value);
                        }
                    }
                    break;
                case MetaTypeId::TYPE_INT64:
                    {
                        std::int64_t value = 0;
                        bool ok = false;
                        if (field->flags & METAFLAG_PROTO_VARINT)
                        {
                            ok = parseVarint(value);
                        }
                        else if (field->flags & METAFLAG_PROTO_ZIGZAG)
                        {
                            ok = parseZigZag<std::int64_t, std::uint64_t>(value);
                        }
                        else
                        {
                            ok = parseFixedValue<std::int64_t, WIRETYPE_FIXED64>(value);
                        }
                        if (ok && m_ptr)
                        {
                            m_visitor.enterInt64(*field, value);
                        }
                    }
                    break;
                case MetaTypeId::TYPE_UINT64:
                    {
                        std::uint64_t value = 0;
                        bool ok = false;
                        if (field->flags & METAFLAG_PROTO_VARINT)
                        {
                            ok = parseVarint(value);
                        }
                        else
                        {
                            ok = parseFixedValue<std::uint64_t, WIRETYPE_FIXED64>(value);
                        }
                        if (ok && m_ptr)
                        {
                            m_visitor.enterUInt64(*field, value);
                        }
                    }
                    break;
                case MetaTypeId::TYPE_FLOAT:
                    {
                        float value = 0.0;
                        bool ok = parseFixedValue<float, WIRETYPE_FIXED32>(value);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterFloat(*field, value);
                        }
                    }
                    break;
                case MetaTypeId::TYPE_DOUBLE:
                    {
                        double value = 0.0;
                        bool ok = parseFixedValue<double, WIRETYPE_FIXED64>(value);
                        if (ok && m_ptr)
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
                        if (ok && m_ptr)
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
                        if (ok && m_ptr)
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
                        bool ok = parseVarint(value);
                        if (ok && m_ptr)
                        {
                            const MetaEnum* metaEnum = MetaDataGlobal::instance().getEnum(*field);
                            if (metaEnum)
                            {
                                if (!metaEnum->isId(value))
                                {
                                    value = 0;
                                }
                                m_visitor.enterEnum(*field, value);
                            }
                        }
                    }
                    break;
                case MetaTypeId::TYPE_ARRAY_BOOL:
                    {
                        std::vector<bool> array;
                        bool ok = parseArrayVarint(array);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterArrayBool(*field, std::move(array));
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
                        if (ok && m_ptr)
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
                        if (ok && m_ptr)
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
                        if (ok && m_ptr)
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
                        if (ok && m_ptr)
                        {
                            m_visitor.enterArrayUInt64(*field, std::move(array));
                        }
                    }
                    break;
                case MetaTypeId::TYPE_ARRAY_FLOAT:
                    {
                        std::vector<float> array;
                        bool ok = parseArrayFixed<float, WIRETYPE_FIXED32>(array);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterArrayFloat(*field, std::move(array));
                        }
                    }
                    break;
                case MetaTypeId::TYPE_ARRAY_DOUBLE:
                    {
                        std::vector<double> array;
                        bool ok = parseArrayFixed<double, WIRETYPE_FIXED64>(array);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterArrayDouble(*field, std::move(array));
                        }
                    }
                    break;
                case MetaTypeId::TYPE_ARRAY_STRING:
                    {
                        std::vector<std::string> array;
                        bool ok = parseArrayString(array);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterArrayString(*field, std::move(array));
                        }
                    }
                    break;
                case MetaTypeId::TYPE_ARRAY_BYTES:
                    {
                        std::vector<Bytes> array;
                        bool ok = parseArrayString(array);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterArrayBytes(*field, std::move(array));
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
                        if (ok && m_ptr)
                        {
                            const MetaEnum* metaEnum = MetaDataGlobal::instance().getEnum(*field);
                            if (metaEnum)
                            {
                                for (size_t i = 0; i < array.size(); ++i)
                                {
                                    if (!metaEnum->isId(array[i]))
                                    {
                                        array[i] = 0;
                                    }
                                }
                                m_visitor.enterArrayEnum(*field, std::move(array));
                            }
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
    for (int i = 1; i < 10; ++i)
    {
        if (m_size <= 0)
        {
            break;
        }
        c = static_cast<std::uint8_t>(*m_ptr);
        res += (c - 1) << (7 * i);
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
        EndianHelper<sizeof(T)>::read(m_ptr, value);
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


std::int32_t ParserProto::zigzag(std::uint32_t value)
{
    return static_cast<std::int32_t>((value >> 1) ^ (~(value & 1) + 1));
}

std::int64_t ParserProto::zigzag(std::uint64_t value)
{
    return static_cast<std::int64_t>((value >> 1) ^ (~(value & 1) + 1));
}



void ParserProto::skip(WireType wireType)
{
    switch (wireType)
    {
    case WIRETYPE_VARINT:
        parseVarint();
        break;
    case WIRETYPE_FIXED64:
        if (m_size >= static_cast<ssize_t>(sizeof(std::uint64_t)))
        {
            m_ptr += sizeof(std::uint64_t);
            m_size -= sizeof(std::uint64_t);
        }
        else
        {
            m_ptr = nullptr;
            m_size = 0;
        }
        break;
    case WIRETYPE_LENGTH_DELIMITED:
        {
            std::uint64_t len = parseVarint();
            if (m_ptr)
            {
                m_ptr += len;
                m_size -= len;
                if (m_size < 0)
                {
                    m_ptr = nullptr;
                    m_size = 0;
                }
            }
        }
        break;
    case WIRETYPE_FIXED32:
        if (m_size >= static_cast<ssize_t>(sizeof(std::uint32_t)))
        {
            m_ptr += sizeof(std::uint32_t);
            m_size -= sizeof(std::uint32_t);
        }
        else
        {
            m_ptr = nullptr;
            m_size = 0;
        }
        break;
    default:
        m_ptr = nullptr;
        m_size = 0;
        break;
    }
}

}   // namespace finalmq
