
#include "serializeproto/SerializerProto.h"
#include "metadata/MetaData.h"
#include "helpers/BexDefines.h"

#include <assert.h>
#include <memory.h>
#include <iostream>



static const int INDEX2ID = 1;

enum WireType
{
    WIRETYPE_VARINT = 0,
    WIRETYPE_FIXED64 = 1,
    WIRETYPE_LENGTH_DELIMITED = 2,
    WIRETYPE_START_GROUP = 3,
    WIRETYPE_END_GROUP = 4,
    WIRETYPE_FIXED32 = 5,
};


SerializerProto::SerializerProto(IZeroCopyBuffer& buffer, int maxBlockSize)
    : m_zeroCopybuffer(buffer)
    , m_maxBlockSize(maxBlockSize)
{
}


static const int MAX_VARINT_SIZE = 10;



void SerializerProto::serializeVarint(std::uint64_t value)
{
    while (value >= 0x80)
    {
        *m_buffer = static_cast<char>(value | 0x80);
        value >>= 7;
        ++m_buffer;
    }
    *m_buffer = static_cast<char>(value);
    ++m_buffer;
}



template<class T>
void SerializerProto::serializeVarintValue(int id, T value)
{
    if (value == 0)
    {
        return;
    }

    reserveSpace(MAX_VARINT_SIZE + MAX_VARINT_SIZE);

    std::uint32_t tag = (id << 3) | WIRETYPE_VARINT;
    serializeVarint(tag);
    serializeVarint(static_cast<std::uint64_t>(value));
}





template<class T>
void SerializerProto::serializeZigZagValue(int id, T value)
{
    if (value == 0)
    {
        return;
    }

    auto v = zigzag(value);
    serializeVarintValue(id, v);
}


std::uint32_t SerializerProto::zigzag(std::int32_t value)
{
    return (static_cast<std::uint32_t>(value) << 1) ^ static_cast<std::uint32_t>(value >> 31);
}

std::uint64_t SerializerProto::zigzag(std::int64_t value)
{
    return (static_cast<std::uint64_t>(value) << 1) ^ static_cast<std::uint64_t>(value >> 63);
}




template<class T, int WIRETYPE>
void SerializerProto::serializeFixedValue(int id, T value)
{
    if (value == 0)
    {
        return;
    }

    reserveSpace(MAX_VARINT_SIZE + sizeof(value));

    std::uint32_t tag = (id << 3) | WIRETYPE;
    serializeVarint(tag);
    EndianHelper<sizeof(T)>::write(m_buffer, value);
    m_buffer += sizeof(T);
}


template<bool ignoreZeroLength>
void SerializerProto::serializeString(int id, const char* value, int size)
{
    if (size <= 0 && ignoreZeroLength)
    {
        return;
    }

    reserveSpace(MAX_VARINT_SIZE + MAX_VARINT_SIZE + size);

    std::uint32_t tag = (id << 3) | WIRETYPE_LENGTH_DELIMITED;
    serializeVarint(tag);
    serializeVarint(size);
    memcpy(m_buffer, value, size);
    m_buffer += size;
}





template<class T>
void SerializerProto::serializeArrayFixed(int id, const T* value, int size)
{
    if (size <= 0)
    {
        return;
    }

    int sizeByte = size * sizeof(T);
    reserveSpace(MAX_VARINT_SIZE + sizeByte);

    std::uint32_t tag = (id << 3) | WIRETYPE_LENGTH_DELIMITED;
    serializeVarint(tag);
    serializeVarint(sizeByte);
    assert(sizeByte <= m_bufferEnd - m_buffer);
#ifdef BEXMQ_LITTLE_ENDIAN
    memcpy(m_buffer, value, sizeByte);
#else
    for (int i = 0; i < value.size(); i++)
    {
        EndianHelper<sizeof(T)>::write(&m_buffer[i * sizeof(T)], value[i]);
    }
#endif
    m_buffer += sizeByte;
}


void SerializerProto::serializeArrayBool(int id, const std::vector<bool>& value)
{
    if (value.size() <= 0)
    {
        return;
    }

    int sizeByte = value.size() * 1;
    reserveSpace(MAX_VARINT_SIZE + sizeByte);

    std::uint32_t tag = (id << 3) | WIRETYPE_LENGTH_DELIMITED;
    serializeVarint(tag);
    serializeVarint(sizeByte);
    assert(sizeByte <= m_bufferEnd - m_buffer);
    for (size_t i = 0; i < value.size(); i++)
    {
        serializeVarint(value[i] ? 1 : 0);
    }
}


void SerializerProto::serializeArrayString(int id, const std::vector<std::string>& value)
{
    int size = value.size();
    if (size <= 0)
    {
        return;
    }

    for (int i = 0; i < size; i++)
    {
        const std::string& str = value[i];
        serializeString<false>(id, str.data(), str.size());
    }
}


void SerializerProto::serializeArrayBytes(int id, const std::vector<Bytes>& value)
{
    int size = value.size();
    if (size <= 0)
    {
        return;
    }

    for (int i = 0; i < size; i++)
    {
        const Bytes& bytes = value[i];
        serializeString<false>(id, reinterpret_cast<const char*>(bytes.data()), bytes.size());
    }
}



template<class T>
void SerializerProto::serializeArrayVarint(int id, const T* value, int size)
{
    if (size <= 0)
    {
        return;
    }

    reserveSpace(size * 2 * MAX_VARINT_SIZE);

    std::uint32_t tag = (id << 3) | WIRETYPE_VARINT;

    for (int i = 0; i < size; i++)
    {
        serializeVarint(tag);
        serializeVarint(value[i]);
    }
}


template<class T>
void SerializerProto::serializeArrayZigZag(int id, const T* value, int size)
{
    if (size <= 0)
    {
        return;
    }

    reserveSpace(size * 2 * MAX_VARINT_SIZE);

    std::uint32_t tag = (id << 3) | WIRETYPE_VARINT;

    for (int i = 0; i < size; i++)
    {
        serializeVarint(tag);
        auto v = zigzag(value[i]);
        serializeVarint(v);
    }
}




static const int RESERVE_STRUCT_SIZE = 8;
static const int DUMMY_ID = 2047;


char* SerializerProto::serializeStruct(int id)
{
    reserveSpace(MAX_VARINT_SIZE + RESERVE_STRUCT_SIZE);

    char* bufferStructStart = m_buffer;
    std::uint32_t tag = (id << 3) | WIRETYPE_LENGTH_DELIMITED;
    serializeVarint(tag);
    return bufferStructStart;
}




void SerializerProto::reserveSpace(int space)
{
    int sizeRemaining = m_bufferEnd - m_buffer;
    if (sizeRemaining < space)
    {
        int sizeNew = std::max(m_maxBlockSize, space);
        char* bufferStartNew = m_zeroCopybuffer.addBuffer(sizeNew);
        if (m_buffer != nullptr)
        {
            int size = m_buffer - m_bufferStart;
            assert(size >= 0);
            m_zeroCopybuffer.downsizeLastBuffer(size);

            for (auto it = m_stackStruct.begin(); it != m_stackStruct.end(); ++it)
            {
                StructData& structData = *it;
                if (structData.buffer)
                {
                    structData.allocateNextDataBuffer = true;
                    int sizeStruct = m_buffer - structData.buffer;
                    assert(sizeStruct >= 0);
                    structData.size += sizeStruct;
                    structData.buffer = bufferStartNew;
                }
            }
        }
        m_bufferStart = bufferStartNew;
        m_bufferEnd = m_bufferStart + sizeNew;
        m_buffer = m_bufferStart;
    }
}

void SerializerProto::resizeBuffer()
{
    if (m_buffer != nullptr)
    {
        int size = m_buffer - m_bufferStart;
        assert(size >= 0);
        m_zeroCopybuffer.downsizeLastBuffer(size);
        m_bufferStart = nullptr;
        m_bufferEnd = nullptr;
        m_buffer = nullptr;
    }
}






// IParserVisitor
void SerializerProto::notifyError(const char* str, const char* message)
{

}

void SerializerProto::finished()
{

}


void SerializerProto::enterStruct(const MetaField& field)
{
    int id = field.index + INDEX2ID;

    if (!m_stackStruct.empty())
    {
        char* bufferStructStart = serializeStruct(id);
        bool arrayEntry = m_stackStruct.back().arrayParent;
        m_stackStruct.emplace_back(bufferStructStart, m_buffer, m_buffer + RESERVE_STRUCT_SIZE, arrayEntry);
        m_buffer += RESERVE_STRUCT_SIZE;
    }
    else
    {
        m_stackStruct.emplace_back(nullptr, nullptr, nullptr, false);
    }
}

int SerializerProto::calculateStructSize(int& structSize)
{
    static constexpr std::uint32_t tagDummy = (DUMMY_ID << 3) | WIRETYPE_VARINT;

    int remainingSize = RESERVE_STRUCT_SIZE - 1;
    if (structSize + (RESERVE_STRUCT_SIZE - 1) < 128)
    {
        remainingSize = RESERVE_STRUCT_SIZE - 1;
        structSize += RESERVE_STRUCT_SIZE - 1;
    }
    else if (structSize + (RESERVE_STRUCT_SIZE - 1) == 128)
    {
        serializeVarint(tagDummy);
        serializeVarint(1);
        remainingSize = RESERVE_STRUCT_SIZE - 2;
        structSize += 3 + RESERVE_STRUCT_SIZE - 2;
    }
    else if (structSize + (RESERVE_STRUCT_SIZE - 2) < 16384)
    {
        remainingSize = RESERVE_STRUCT_SIZE - 2;
        structSize += RESERVE_STRUCT_SIZE - 2;
    }
    else if (structSize + (RESERVE_STRUCT_SIZE - 2) == 16384)
    {
        serializeVarint(tagDummy);
        serializeVarint(1);
        remainingSize = RESERVE_STRUCT_SIZE - 3;
        structSize += 3 + RESERVE_STRUCT_SIZE - 3;
    }
    else if (structSize + (RESERVE_STRUCT_SIZE - 3) < 2097152)
    {
        remainingSize = RESERVE_STRUCT_SIZE - 3;
        structSize += RESERVE_STRUCT_SIZE - 3;
    }
    else if (structSize + (RESERVE_STRUCT_SIZE - 3) == 2097152)
    {
        serializeVarint(tagDummy);
        serializeVarint(1);
        remainingSize = RESERVE_STRUCT_SIZE - 4;
        structSize += 3 + RESERVE_STRUCT_SIZE - 4;
    }
    else if (structSize + (RESERVE_STRUCT_SIZE - 4) < 268435456)
    {
        remainingSize = RESERVE_STRUCT_SIZE - 4;
        structSize += RESERVE_STRUCT_SIZE - 4;
    }
    else if (structSize + (RESERVE_STRUCT_SIZE - 4) == 268435456)
    {
        serializeVarint(tagDummy);
        serializeVarint(1);
        remainingSize = RESERVE_STRUCT_SIZE - 5;
        structSize += 3 + RESERVE_STRUCT_SIZE - 5;
    }
    else
    {
        remainingSize = RESERVE_STRUCT_SIZE - 5;
        structSize += RESERVE_STRUCT_SIZE - 5;
    }

    return remainingSize;
}

void SerializerProto::fillRemainingStruct(int remainingSize)
{
    switch (remainingSize)
    {
    case 1:
        serializeVarint(1);
        break;
    case 2:
        serializeVarint(128);
        break;
    case 3:
        serializeVarint(16384);
        break;
    case 4:
        serializeVarint(2097152);
        break;
    case 5:
        serializeVarint(268435456);
        break;
    default:
        assert(false);
        break;
    }
}

void SerializerProto::exitStruct(const MetaField& field)
{
    assert(!m_stackStruct.empty());
    StructData& structData = m_stackStruct.back();
    if (m_stackStruct.size() == 1)
    {
        resizeBuffer();
    }
    else
    {
        assert(structData.buffer);
        int structSize = (m_buffer - structData.buffer) + structData.size;
        if (structSize == 0 && !structData.allocateNextDataBuffer && !structData.arrayEntry)
        {
            m_buffer = structData.bufferStructStart;
        }
        else if (structSize <= 32 && !structData.allocateNextDataBuffer)
        {
            m_buffer = structData.bufferStructSize;
            serializeVarint(structSize);
            memmove(m_buffer, structData.bufferStructSize + RESERVE_STRUCT_SIZE, structSize);
            m_buffer += structSize;
        }
        else
        {
            int remainingSize = calculateStructSize(structSize);

            char* bufferCurrent = m_buffer;
            m_buffer = structData.bufferStructSize;

            serializeVarint(structSize);
            assert(remainingSize <= 7 && remainingSize >= 3);
            int remainingSizeFromBuffer = structData.bufferStructSize + RESERVE_STRUCT_SIZE - m_buffer;
            assert(remainingSizeFromBuffer == remainingSize);
            static constexpr std::uint32_t tagDummy = (DUMMY_ID << 3) | WIRETYPE_VARINT;
            serializeVarint(tagDummy);

            remainingSize -= 2;
            fillRemainingStruct(remainingSize);
            assert(m_buffer == structData.bufferStructSize + RESERVE_STRUCT_SIZE);
            m_buffer = bufferCurrent;
        }
    }
    m_stackStruct.pop_back();
}

void SerializerProto::enterArrayStruct(const MetaField& field)
{
    assert(!m_stackStruct.empty());
    m_stackStruct.back().arrayParent = true;
}
void SerializerProto::exitArrayStruct(const MetaField& field)
{
    assert(!m_stackStruct.empty());
    m_stackStruct.back().arrayParent = false;
}

void SerializerProto::enterBool(const MetaField& field, bool value)
{
    int id = field.index + INDEX2ID;
    serializeVarintValue(id, value);
}



void SerializerProto::enterInt32(const MetaField& field, std::int32_t value)
{
    int id = field.index + INDEX2ID;
    if (field.flags & METAFLAG_PROTO_VARINT)
    {
        serializeVarintValue(id, value);
    }
    else if (field.flags & METAFLAG_PROTO_ZIGZAG)
    {
        serializeZigZagValue(id, value);
    }
    else
    {
        serializeFixedValue<std::int32_t, WIRETYPE_FIXED32>(id, value);
    }
}
void SerializerProto::enterUInt32(const MetaField& field, std::uint32_t value)
{
    int id = field.index + INDEX2ID;
    if (field.flags & METAFLAG_PROTO_VARINT)
    {
        serializeVarintValue(id, value);
    }
    else
    {
        serializeFixedValue<std::uint32_t, WIRETYPE_FIXED32>(id, value);
    }
}
void SerializerProto::enterInt64(const MetaField& field, std::int64_t value)
{
    int id = field.index + INDEX2ID;
    if (field.flags & METAFLAG_PROTO_VARINT)
    {
        serializeVarintValue(id, value);
    }
    else if (field.flags & METAFLAG_PROTO_ZIGZAG)
    {
        serializeZigZagValue(id, value);
    }
    else
    {
        serializeFixedValue<std::int64_t, WIRETYPE_FIXED64>(id, value);
    }
}
void SerializerProto::enterUInt64(const MetaField& field, std::uint64_t value)
{
    int id = field.index + INDEX2ID;
    if (field.flags & METAFLAG_PROTO_VARINT)
    {
        serializeVarintValue(id, value);
    }
    else
    {
        serializeFixedValue<std::uint64_t, WIRETYPE_FIXED64>(id, value);
    }
}
void SerializerProto::enterFloat(const MetaField& field, float value)
{
    int id = field.index + INDEX2ID;
    serializeFixedValue<float, WIRETYPE_FIXED32>(id, value);
}
void SerializerProto::enterDouble(const MetaField& field, double value)
{
    int id = field.index + INDEX2ID;
    serializeFixedValue<double, WIRETYPE_FIXED64>(id, value);
}
void SerializerProto::enterString(const MetaField& field, std::string&& value)
{
    int id = field.index + INDEX2ID;
    serializeString<true>(id, value.data(), value.size());
}
void SerializerProto::enterString(const MetaField& field, const char* value, int size)
{
    int id = field.index + INDEX2ID;
    serializeString<true>(id, value, size);
}
void SerializerProto::enterBytes(const MetaField& field, Bytes&& value)
{
    int id = field.index + INDEX2ID;
    serializeString<true>(id, reinterpret_cast<const char*>(value.data()), value.size());
}
void SerializerProto::enterBytes(const MetaField& field, const unsigned char* value, int size)
{
    int id = field.index + INDEX2ID;
    serializeString<true>(id, reinterpret_cast<const char*>(value), size);
}
void SerializerProto::enterEnum(const MetaField& field, std::int32_t value)
{
    int id = field.index + INDEX2ID;
    serializeVarintValue(id, value);
}

void SerializerProto::enterEnum(const MetaField& field, std::string&& value)
{
    std::int32_t enumValue = MetaDataGlobal::instance().getEnumValueByName(field, value);
    enterEnum(field, enumValue);
}

void SerializerProto::enterEnum(const MetaField& field, const char* value, int size)
{
    enterEnum(field, std::string(value, size));
}



void SerializerProto::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayBool(id, value);
}
void SerializerProto::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayBool(id, value);
}
void SerializerProto::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
{
    enterArrayInt32(field, value.data(), value.size());
}
void SerializerProto::enterArrayInt32(const MetaField& field, const std::int32_t* value, int size)
{
    int id = field.index + INDEX2ID;
    if (field.flags & METAFLAG_PROTO_VARINT)
    {
        serializeArrayVarint(id, value, size);
    }
    else if (field.flags & METAFLAG_PROTO_ZIGZAG)
    {
        serializeArrayZigZag(id, value, size);
    }
    else
    {
        serializeArrayFixed(id, value, size);
    }
}
void SerializerProto::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
{
    enterArrayUInt32(field, value.data(), value.size());
}
void SerializerProto::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, int size)
{
    int id = field.index + INDEX2ID;
    if (field.flags & METAFLAG_PROTO_VARINT)
    {
        serializeArrayVarint(id, value, size);
    }
    else
    {
        serializeArrayFixed(id, value, size);
    }
}
void SerializerProto::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
{
    enterArrayInt64(field, value.data(), value.size());
}
void SerializerProto::enterArrayInt64(const MetaField& field, const std::int64_t* value, int size)
{
    int id = field.index + INDEX2ID;
    if (field.flags & METAFLAG_PROTO_VARINT)
    {
        serializeArrayVarint(id, value, size);
    }
    else if (field.flags & METAFLAG_PROTO_ZIGZAG)
    {
        serializeArrayZigZag(id, value, size);
    }
    else
    {
        serializeArrayFixed(id, value, size);
    }
}
void SerializerProto::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
{
    enterArrayUInt64(field, value.data(), value.size());
}
void SerializerProto::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, int size)
{
    int id = field.index + INDEX2ID;
    if (field.flags & METAFLAG_PROTO_VARINT)
    {
        serializeArrayVarint(id, value, size);
    }
    else
    {
        serializeArrayFixed(id, value, size);
    }
}
void SerializerProto::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayFixed(id, value.data(), value.size());
}
void SerializerProto::enterArrayFloat(const MetaField& field, const float* value, int size)
{
    int id = field.index + INDEX2ID;
    serializeArrayFixed(id, value, size);
}
void SerializerProto::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayFixed(id, value.data(), value.size());
}
void SerializerProto::enterArrayDouble(const MetaField& field, const double* value, int size)
{
    int id = field.index + INDEX2ID;
    serializeArrayFixed(id, value, size);
}
void SerializerProto::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayString(id, value);
}
void SerializerProto::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayString(id, value);
}
void SerializerProto::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayBytes(id, value);
}
void SerializerProto::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayBytes(id, value);
}
void SerializerProto::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayVarint(id, value.data(), value.size());
}
void SerializerProto::enterArrayEnum(const MetaField& field, const std::int32_t* value, int size)
{
    int id = field.index + INDEX2ID;
    serializeArrayVarint(id, value, size);
}

void SerializerProto::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
{
    enterArrayEnum(field, value);
}

void SerializerProto::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
{
    const MetaEnum* metaEnum = MetaDataGlobal::instance().getEnum(field);
    if (metaEnum)
    {
        if (value.size() <= 0)
        {
            return;
        }

        reserveSpace(value.size() * 2 * MAX_VARINT_SIZE);

        int id = field.index + INDEX2ID;
        std::uint32_t tag = (id << 3) | WIRETYPE_VARINT;

        for (size_t i = 0; i < value.size(); i++)
        {
            int intValue = 0;
            const MetaEnumEntry* entry = metaEnum->getEntryByName(value[i]);
            if (entry)
            {
                intValue = entry->id;
            }
            serializeVarint(tag);
            serializeVarint(intValue);
        }
    }
}
