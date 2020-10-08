
#include "serializeproto/ParserProto.h"
#include "metadata/MetaData.h"
#include "helpers/BexDefines.h"

#include <assert.h>
#include <memory.h>
#include <iostream>


static const int INDEX2ID = 1;


ParserProto::ParserProto(IParserVisitor& visitor, const char* ptr, int size)
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



bool ParserProto::parseString(const char*& buffer, int& size)
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
                    m_tag = parseVarint();
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
                    std::uint32_t sizeElements = sizeBuffer / sizeof(T);
                    array.resize(sizeElements);
#ifdef BEXMQ_LITTLE_ENDIAN
                    memcpy(array.data(), m_ptr, sizeElements * sizeof(T));
#else
                    for (int i = 0; i < sizeElements; i++)
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


template<class T, bool ZIGZAG = false>
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
                T v = (ZIGZAG) ? zigzag(value) : static_cast<T>(value);
                array.push_back(v);
                if (m_size > 0)
                {
                    m_tag = parseVarint();
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
                        T v = (ZIGZAG) ? zigzag(value) : static_cast<T>(value);
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



bool ParserProto::parseArrayString(std::vector<std::string>& array)
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
                    array.emplace_back(m_ptr, sizeBuffer);
                    m_ptr += sizeBuffer;
                    m_size -= sizeBuffer;
                    if (m_size > 0)
                    {
                        m_tag = parseVarint();
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


bool ParserProto::parseArrayBytes(std::vector<Bytes>& array)
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
                        m_tag = parseVarint();
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
                    m_tag = parseVarint();
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

    MetaField field = {MetaType::TYPE_STRUCT, typeName};
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
            m_tag = parseVarint();
        }
        if (m_ptr)
        {
            int id = m_tag >> 3;
            int index = id - INDEX2ID;
            const MetaField* field = stru.getFieldByIndex(index);
            if (field)
            {
                switch (field->type)
                {
                case MetaType::TYPE_NONE:
                    break;
                case MetaType::TYPE_BOOL:
                    {
                        bool value = 0;
                        bool ok = parseVarint(value);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterBool(*field, value);
                        }
                    }
                    break;
                case MetaType::TYPE_INT32:
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
                case MetaType::TYPE_UINT32:
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
                case MetaType::TYPE_INT64:
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
                case MetaType::TYPE_UINT64:
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
                case MetaType::TYPE_FLOAT:
                    {
                        float value = 0.0;
                        bool ok = parseFixedValue<float, WIRETYPE_FIXED32>(value);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterFloat(*field, value);
                        }
                    }
                    break;
                case MetaType::TYPE_DOUBLE:
                    {
                        double value = 0.0;
                        bool ok = parseFixedValue<double, WIRETYPE_FIXED64>(value);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterDouble(*field, value);
                        }
                    }
                    break;
                case MetaType::TYPE_STRING:
                    {
                        const char* buffer = nullptr;
                        int size = 0;
                        bool ok = parseString(buffer, size);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterString(*field, buffer, size);
                        }
                    }
                    break;
                case MetaType::TYPE_BYTES:
                    {
                        const char* buffer = nullptr;
                        int size = 0;
                        bool ok = parseString(buffer, size);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterBytes(*field, reinterpret_cast<const unsigned char*>(buffer), size);
                        }
                    }
                    break;
                case MetaType::TYPE_STRUCT:
                    parseStructWire(*field);
                    break;
                case MetaType::TYPE_ENUM:
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
                case MetaType::TYPE_ARRAY_BOOL:
                    {
                        std::vector<bool> array;
                        bool ok = parseArrayVarint(array);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterArrayBool(*field, std::move(array));
                        }
                    }
                    break;
                case MetaType::TYPE_ARRAY_INT32:
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
                case MetaType::TYPE_ARRAY_UINT32:
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
                case MetaType::TYPE_ARRAY_INT64:
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
                case MetaType::TYPE_ARRAY_UINT64:
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
                case MetaType::TYPE_ARRAY_FLOAT:
                    {
                        std::vector<float> array;
                        bool ok = parseArrayFixed<float, WIRETYPE_FIXED32>(array);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterArrayFloat(*field, std::move(array));
                        }
                    }
                    break;
                case MetaType::TYPE_ARRAY_DOUBLE:
                    {
                        std::vector<double> array;
                        bool ok = parseArrayFixed<double, WIRETYPE_FIXED64>(array);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterArrayDouble(*field, std::move(array));
                        }
                    }
                    break;
                case MetaType::TYPE_ARRAY_STRING:
                    {
                        std::vector<std::string> array;
                        bool ok = parseArrayString(array);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterArrayString(*field, std::move(array));
                        }
                    }
                    break;
                case MetaType::TYPE_ARRAY_BYTES:
                    {
                        std::vector<Bytes> array;
                        bool ok = parseArrayBytes(array);
                        if (ok && m_ptr)
                        {
                            m_visitor.enterArrayBytes(*field, std::move(array));
                        }
                    }
                    break;
                case MetaType::TYPE_ARRAY_STRUCT:
                    parseArrayStruct(*field);
                    break;
                case MetaType::TYPE_ARRAY_ENUM:
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
                case MetaType::TYPE_ARRAY_FLAG:
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
    if (m_size >= static_cast<int>(sizeof(T)))
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
        if (m_size >= static_cast<int>(sizeof(std::uint64_t)))
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
        if (m_size >= static_cast<int>(sizeof(std::uint32_t)))
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

