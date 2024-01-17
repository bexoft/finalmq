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


#include "finalmq/serializeproto/SerializerProto.h"
#include "finalmq/serialize/ParserAbortAndIndex.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/helpers/FmqDefines.h"
#include "finalmq/helpers/ModulenameFinalmq.h"
#include "finalmq/logger/LogStream.h"

#include <assert.h>
#include <memory.h>
#include <iostream>


namespace finalmq {

static constexpr int INDEX2ID = 1;
static constexpr int MAX_VARINT_SIZE = 10;
static constexpr int STRUCT_SIZE_COPY = 128;




SerializerProto::SerializerProto(IZeroCopyBuffer& buffer, int maxBlockSize)
    : ParserConverter()
    , m_internal(buffer, maxBlockSize)
    , m_parserAbortAndIndex()
{
    m_parserAbortAndIndex = std::make_unique<ParserAbortAndIndex>(&m_internal);
    ParserConverter::setVisitor(*m_parserAbortAndIndex);
}



SerializerProto::Internal::Internal(IZeroCopyBuffer& buffer, int maxBlockSize)
    : m_zeroCopybuffer(buffer)
    , m_maxBlockSize(maxBlockSize)
{
}




void SerializerProto::Internal::serializeVarint(std::uint64_t value)
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



void SerializerProto::Internal::serializeVarintValue(int id, std::uint64_t value)
{
    if (value == 0)
    {
        return;
    }

    reserveSpace(MAX_VARINT_SIZE + MAX_VARINT_SIZE);

    std::uint32_t tag = (id << 3) | WIRETYPE_VARINT;
    serializeVarint(tag);
    serializeVarint(value);
}





void SerializerProto::Internal::serializeZigZagValue(int id, std::int64_t value)
{
    if (value == 0)
    {
        return;
    }

    auto v = zigzag(value);
    serializeVarintValue(id, v);
}


std::uint64_t SerializerProto::Internal::zigzag(std::int64_t value)
{
    return (static_cast<std::uint64_t>(value) << 1) ^ static_cast<std::uint64_t>(value >> 63);
}




template<class T, int WIRETYPE>
void SerializerProto::Internal::serializeFixedValue(int id, T value)
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
void SerializerProto::Internal::serializeString(int id, const char* value, ssize_t size)
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
void SerializerProto::Internal::serializeArrayFixed(int id, const T* value, ssize_t size)
{
    if (size <= 0)
    {
        return;
    }

    ssize_t sizeByte = size * sizeof(T);
    reserveSpace(MAX_VARINT_SIZE + sizeByte);

    std::uint32_t tag = (id << 3) | WIRETYPE_LENGTH_DELIMITED;
    serializeVarint(tag);
    serializeVarint(sizeByte);
    assert(sizeByte <= m_bufferEnd - m_buffer);
#ifdef FINALMQ_LITTLE_ENDIAN
    memcpy(m_buffer, value, sizeByte);
#else
    for (int i = 0; i < value.size(); i++)
    {
        EndianHelper<sizeof(T)>::write(&m_buffer[i * sizeof(T)], value[i]);
    }
#endif
    m_buffer += sizeByte;
}


void SerializerProto::Internal::serializeArrayBool(int id, const std::vector<bool>& value)
{
    if (value.size() <= 0)
    {
        return;
    }

    ssize_t sizeByte = value.size() * 1;
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


void SerializerProto::Internal::serializeArrayString(int id, const std::vector<std::string>& value)
{
    ssize_t size = value.size();
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


void SerializerProto::Internal::serializeArrayBytes(int id, const std::vector<Bytes>& value)
{
    ssize_t size = value.size();
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
void SerializerProto::Internal::serializeArrayVarint(int id, const T* value, ssize_t size)
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
void SerializerProto::Internal::serializeArrayZigZag(int id, const T* value, ssize_t size)
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


char* SerializerProto::Internal::serializeStruct(int id)
{
    reserveSpace(MAX_VARINT_SIZE + RESERVE_STRUCT_SIZE);

    char* bufferStructStart = m_buffer;
    std::uint32_t tag = (id << 3) | WIRETYPE_LENGTH_DELIMITED;
    serializeVarint(tag);
    return bufferStructStart;
}




void SerializerProto::Internal::reserveSpace(ssize_t space)
{
    ssize_t sizeRemaining = m_bufferEnd - m_buffer;
    if (sizeRemaining < space)
    {
        if (m_buffer != nullptr)
        {
            ssize_t size = m_buffer - m_bufferStart;
            assert(size >= 0);
            m_zeroCopybuffer.downsizeLastBuffer(size);
        }

        const ssize_t sizeRemainingZeroCopyBuffer = m_zeroCopybuffer.getRemainingSize();
        ssize_t sizeNew = m_maxBlockSize;
        if (space <= sizeRemainingZeroCopyBuffer)
        {
            sizeNew = sizeRemainingZeroCopyBuffer;
        }
        sizeNew = std::max(sizeNew, space);
        char* bufferStartNew = m_zeroCopybuffer.addBuffer(sizeNew);

        if (m_buffer != nullptr)
        {
            for (auto it = m_stackStruct.begin(); it != m_stackStruct.end(); ++it)
            {
                StructData& structData = *it;
                if (structData.buffer)
                {
                    structData.allocateNextDataBuffer = true;
                    ssize_t sizeStruct = m_buffer - structData.buffer;
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

void SerializerProto::Internal::resizeBuffer()
{
    if (m_buffer != nullptr)
    {
        ssize_t size = m_buffer - m_bufferStart;
        assert(size >= 0);
        m_zeroCopybuffer.downsizeLastBuffer(size);
        m_bufferStart = nullptr;
        m_bufferEnd = nullptr;
        m_buffer = nullptr;
    }
}






// IParserVisitor
void SerializerProto::Internal::notifyError(const char* /*str*/, const char* /*message*/)
{

}



void SerializerProto::Internal::startStruct(const MetaStruct& /*stru*/)
{
}


void SerializerProto::Internal::finished()
{
    resizeBuffer();
}


void SerializerProto::Internal::enterStruct(const MetaField& field)
{
    int id = field.index + INDEX2ID;
    char* bufferStructStart = serializeStruct(id);
    bool arrayEntry = (!m_stackStruct.empty()) ? m_stackStruct.back().arrayParent : m_arrayParent;
    m_stackStruct.emplace_back(bufferStructStart, m_buffer, m_buffer + RESERVE_STRUCT_SIZE, arrayEntry);
    m_buffer += RESERVE_STRUCT_SIZE;
}

ssize_t SerializerProto::Internal::calculateStructSize(ssize_t& structSize)
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

void SerializerProto::Internal::fillRemainingStruct(ssize_t remainingSize)
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

void SerializerProto::Internal::exitStruct(const MetaField& /*field*/)
{
    assert(!m_stackStruct.empty());
    StructData& structData = m_stackStruct.back();
    assert(structData.buffer);
    ssize_t structSize = (m_buffer - structData.buffer) + structData.size;
    if (structSize == 0 && !structData.allocateNextDataBuffer && !structData.arrayEntry)
    {
        m_buffer = structData.bufferStructStart;
    }
    else if (structSize <= STRUCT_SIZE_COPY && !structData.allocateNextDataBuffer)
    {
        m_buffer = structData.bufferStructSize;
        serializeVarint(structSize);
        memmove(m_buffer, structData.bufferStructSize + RESERVE_STRUCT_SIZE, structSize);
        m_buffer += structSize;
    }
    else
    {
        ssize_t remainingSize = calculateStructSize(structSize);

        char* bufferCurrent = m_buffer;
        m_buffer = structData.bufferStructSize;

        serializeVarint(structSize);
        assert(remainingSize <= 7 && remainingSize >= 3);
        ssize_t remainingSizeFromBuffer = structData.bufferStructSize + RESERVE_STRUCT_SIZE - m_buffer;
        if (remainingSizeFromBuffer != remainingSize)
        {
            streamFatal << "Struct calculations are wrong";
        }
        assert(remainingSizeFromBuffer == remainingSize);
        static constexpr std::uint32_t tagDummy = (DUMMY_ID << 3) | WIRETYPE_VARINT;
        serializeVarint(tagDummy);

        remainingSize -= 2;
        fillRemainingStruct(remainingSize);
        assert(m_buffer == structData.bufferStructSize + RESERVE_STRUCT_SIZE);
        m_buffer = bufferCurrent;
    }
    m_stackStruct.pop_back();
}

void SerializerProto::Internal::enterStructNull(const MetaField& /*field*/)
{
}

void SerializerProto::Internal::enterArrayStruct(const MetaField& /*field*/)
{
    if (!m_stackStruct.empty())
    {
        m_stackStruct.back().arrayParent = true;
    }
    else
    {
        m_arrayParent = true;
    }
}
void SerializerProto::Internal::exitArrayStruct(const MetaField& /*field*/)
{
    if (!m_stackStruct.empty())
    {
        m_stackStruct.back().arrayParent = false;
    }
    else
    {
        m_arrayParent = false;
    }
}

void SerializerProto::Internal::enterBool(const MetaField& field, bool value)
{
    int id = field.index + INDEX2ID;
    serializeVarintValue(id, value);
}



void SerializerProto::Internal::enterInt8(const MetaField& field, std::int8_t value)
{
    enterInt32(field, value);
}
void SerializerProto::Internal::enterUInt8(const MetaField& field, std::uint8_t value)
{
    enterUInt32(field, value);
}
void SerializerProto::Internal::enterInt16(const MetaField& field, std::int16_t value)
{
    enterInt32(field, value);
}
void SerializerProto::Internal::enterUInt16(const MetaField& field, std::uint16_t value)
{
    enterUInt32(field, value);
}
void SerializerProto::Internal::enterInt32(const MetaField& field, std::int32_t value)
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
void SerializerProto::Internal::enterUInt32(const MetaField& field, std::uint32_t value)
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
void SerializerProto::Internal::enterInt64(const MetaField& field, std::int64_t value)
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
void SerializerProto::Internal::enterUInt64(const MetaField& field, std::uint64_t value)
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
void SerializerProto::Internal::enterFloat(const MetaField& field, float value)
{
    int id = field.index + INDEX2ID;
    serializeFixedValue<float, WIRETYPE_FIXED32>(id, value);
}
void SerializerProto::Internal::enterDouble(const MetaField& field, double value)
{
    int id = field.index + INDEX2ID;
    serializeFixedValue<double, WIRETYPE_FIXED64>(id, value);
}
void SerializerProto::Internal::enterString(const MetaField& field, std::string&& value)
{
    int id = field.index + INDEX2ID;
    serializeString<true>(id, value.data(), value.size());
}
void SerializerProto::Internal::enterString(const MetaField& field, const char* value, ssize_t size)
{
    int id = field.index + INDEX2ID;
    serializeString<true>(id, value, size);
}
void SerializerProto::Internal::enterBytes(const MetaField& field, Bytes&& value)
{
    int id = field.index + INDEX2ID;
    serializeString<true>(id, reinterpret_cast<const char*>(value.data()), value.size());
}
void SerializerProto::Internal::enterBytes(const MetaField& field, const BytesElement* value, ssize_t size)
{
    int id = field.index + INDEX2ID;
    serializeString<true>(id, value, size);
}
void SerializerProto::Internal::enterEnum(const MetaField& field, std::int32_t value)
{
    int id = field.index + INDEX2ID;
    serializeVarintValue(id, value);
}

void SerializerProto::Internal::enterEnum(const MetaField& field, std::string&& value)
{
    std::int32_t enumValue = MetaDataGlobal::instance().getEnumValueByName(field, value);
    enterEnum(field, enumValue);
}

void SerializerProto::Internal::enterEnum(const MetaField& field, const char* value, ssize_t size)
{
    enterEnum(field, std::string(value, size));
}



void SerializerProto::Internal::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayBool(id, value);
}
void SerializerProto::Internal::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayBool(id, value);
}
void SerializerProto::Internal::enterArrayInt8(const MetaField& field, std::vector<std::int8_t>&& value)
{
    enterArrayInt8(field, value.data(), value.size());
}
void SerializerProto::Internal::enterArrayInt8(const MetaField& field, const std::int8_t* value, ssize_t size)
{
    std::vector<std::int32_t> array;
    array.resize(size);
    for (ssize_t i = 0; i < size; ++i)
    {
        array[i] = static_cast<std::int32_t>(value[i]);
    }
    enterArrayInt32(field, array.data(), array.size());
}
void SerializerProto::Internal::enterArrayInt16(const MetaField& field, std::vector<std::int16_t>&& value)
{
    enterArrayInt16(field, value.data(), value.size());
}
void SerializerProto::Internal::enterArrayInt16(const MetaField& field, const std::int16_t* value, ssize_t size)
{
    std::vector<std::int32_t> array;
    array.resize(size);
    for (ssize_t i = 0; i < size; ++i)
    {
        array[i] = static_cast<std::int32_t>(value[i]);
    }
    enterArrayInt32(field, array.data(), array.size());
}
void SerializerProto::Internal::enterArrayUInt16(const MetaField& field, std::vector<std::uint16_t>&& value)
{
    enterArrayUInt16(field, value.data(), value.size());
}
void SerializerProto::Internal::enterArrayUInt16(const MetaField& field, const std::uint16_t* value, ssize_t size)
{
    std::vector<std::uint32_t> array;
    array.resize(size);
    for (ssize_t i = 0; i < size; ++i)
    {
        array[i] = static_cast<std::uint32_t>(value[i]);
    }
    enterArrayUInt32(field, array.data(), array.size());
}
void SerializerProto::Internal::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
{
    enterArrayInt32(field, value.data(), value.size());
}
void SerializerProto::Internal::enterArrayInt32(const MetaField& field, const std::int32_t* value, ssize_t size)
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
void SerializerProto::Internal::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
{
    enterArrayUInt32(field, value.data(), value.size());
}
void SerializerProto::Internal::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, ssize_t size)
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
void SerializerProto::Internal::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
{
    enterArrayInt64(field, value.data(), value.size());
}
void SerializerProto::Internal::enterArrayInt64(const MetaField& field, const std::int64_t* value, ssize_t size)
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
void SerializerProto::Internal::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
{
    enterArrayUInt64(field, value.data(), value.size());
}
void SerializerProto::Internal::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, ssize_t size)
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
void SerializerProto::Internal::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayFixed(id, value.data(), value.size());
}
void SerializerProto::Internal::enterArrayFloat(const MetaField& field, const float* value, ssize_t size)
{
    int id = field.index + INDEX2ID;
    serializeArrayFixed(id, value, size);
}
void SerializerProto::Internal::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayFixed(id, value.data(), value.size());
}
void SerializerProto::Internal::enterArrayDouble(const MetaField& field, const double* value, ssize_t size)
{
    int id = field.index + INDEX2ID;
    serializeArrayFixed(id, value, size);
}
void SerializerProto::Internal::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayString(id, value);
}
void SerializerProto::Internal::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayString(id, value);
}
void SerializerProto::Internal::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayBytes(id, value);
}
void SerializerProto::Internal::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayBytes(id, value);
}
void SerializerProto::Internal::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
    int id = field.index + INDEX2ID;
    serializeArrayVarint(id, value.data(), value.size());
}
void SerializerProto::Internal::enterArrayEnum(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    int id = field.index + INDEX2ID;
    serializeArrayVarint(id, value, size);
}

void SerializerProto::Internal::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
{
    enterArrayEnum(field, value);
}

void SerializerProto::Internal::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
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

}   // namespace finalmq
