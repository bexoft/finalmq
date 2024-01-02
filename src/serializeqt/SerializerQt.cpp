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


#include "finalmq/serializeqt/SerializerQt.h"
#include "finalmq/serialize/ParserProcessValuesInOrder.h"
#include "finalmq/metadata/MetaData.h"

//#include <assert.h>
//#include <algorithm>
//#include <cmath>
#include <codecvt>
#include <locale>

namespace finalmq {


    SerializerQt::SerializerQt(IZeroCopyBuffer& buffer, int maxBlockSize)
        : ParserConverter()
        , m_internal(buffer, maxBlockSize)
        , m_parserProcessValuesInOrder()
    {
        m_parserProcessValuesInOrder = std::make_unique<ParserProcessValuesInOrder>(false, &m_internal);
        setVisitor(*m_parserProcessValuesInOrder);
    }



    SerializerQt::Internal::Internal(IZeroCopyBuffer& buffer, int maxBlockSize)
        : m_zeroCopybuffer(buffer)
        , m_maxBlockSize(maxBlockSize)
    {
    }


    // IParserVisitor
    void SerializerQt::Internal::notifyError(const char* /*str*/, const char* /*message*/)
    {
    }

    void SerializerQt::Internal::startStruct(const MetaStruct& /*stru*/)
    {
    }


    void SerializerQt::Internal::finished()
    {
        resizeBuffer();
    }


    void SerializerQt::Internal::enterStruct(const MetaField& field)
    {
        if (m_arrayStructCounter >= 0)
        {
            ++m_arrayStructCounter;
        }
    }

    void SerializerQt::Internal::exitStruct(const MetaField& /*field*/)
    {
    }

    void SerializerQt::Internal::enterStructNull(const MetaField& field)
    {
    }


    void SerializerQt::Internal::enterArrayStruct(const MetaField& field)
    {
        reserveSpace(sizeof(std::uint32_t));
        m_arrayStructCounterBuffer = m_buffer;
        m_arrayStructCounter = 0;
        serialize(m_arrayStructCounter);
        m_buffer += sizeof(std::uint32_t);
        m_arrayStructCounter = 0;
    }

    void SerializerQt::Internal::exitArrayStruct(const MetaField& /*field*/)
    {
        if (m_arrayStructCounterBuffer)
        {
            char* buffer = m_buffer;
            m_buffer = m_arrayStructCounterBuffer;
            serialize(m_arrayStructCounter);
            m_buffer = buffer;
            m_arrayStructCounter = -1;
        }
    }


    void SerializerQt::Internal::enterBool(const MetaField& field, bool value)
    {
        assert(field.typeId == MetaTypeId::TYPE_BOOL);
        reserveSpace(sizeof(std::uint8_t));
        serialize(static_cast<std::uint8_t>(value ? 1 : 0));
    }

    void SerializerQt::Internal::enterInt8(const MetaField& field, std::int8_t value)
    {
        assert(field.typeId == MetaTypeId::TYPE_INT8);
        reserveSpace(sizeof(std::int8_t));
        serialize(value);
    }

    void SerializerQt::Internal::enterUInt8(const MetaField& field, std::uint8_t value)
    {
        assert(field.typeId == MetaTypeId::TYPE_UINT8);
        reserveSpace(sizeof(std::uint8_t));
        serialize(value);
    }

    void SerializerQt::Internal::enterInt16(const MetaField& field, std::int16_t value)
    {
        assert(field.typeId == MetaTypeId::TYPE_INT16);
        reserveSpace(sizeof(std::int16_t));
        serialize(value);
    }

    void SerializerQt::Internal::enterUInt16(const MetaField& field, std::uint16_t value)
    {
        assert(field.typeId == MetaTypeId::TYPE_UINT16);
        reserveSpace(sizeof(std::uint16_t));
        serialize(value);
    }

    void SerializerQt::Internal::enterInt32(const MetaField& field, std::int32_t value)
    {
        assert(field.typeId == MetaTypeId::TYPE_INT32);
        reserveSpace(sizeof(std::int32_t));
        serialize(value);
    }

    void SerializerQt::Internal::enterUInt32(const MetaField& field, std::uint32_t value)
    {
        assert(field.typeId == MetaTypeId::TYPE_UINT32);
        reserveSpace(sizeof(std::uint32_t));
        serialize(value);
    }

    void SerializerQt::Internal::enterInt64(const MetaField& field, std::int64_t value)
    {
        assert(field.typeId == MetaTypeId::TYPE_INT64);
        reserveSpace(sizeof(std::int64_t));
        serialize(value);
    }

    void SerializerQt::Internal::enterUInt64(const MetaField& field, std::uint64_t value)
    {
        assert(field.typeId == MetaTypeId::TYPE_UINT64);
        reserveSpace(sizeof(std::uint64_t));
        serialize(value);
    }

    void SerializerQt::Internal::enterFloat(const MetaField& field, float value)
    {
        assert(field.typeId == MetaTypeId::TYPE_FLOAT);
        reserveSpace(sizeof(double));
        serialize(value);
    }

    void SerializerQt::Internal::enterDouble(const MetaField& field, double value)
    {
        assert(field.typeId == MetaTypeId::TYPE_DOUBLE);
        reserveSpace(sizeof(double));
        serialize(value);
    }

    void SerializerQt::Internal::enterString(const MetaField& field, std::string&& value)
    {
        assert(field.typeId == MetaTypeId::TYPE_STRING);
        reserveSpace(sizeof(std::uint32_t) + 2 * value.size());
        serializeString(value);
    }

    void SerializerQt::Internal::enterString(const MetaField& field, const char* value, ssize_t size)
    {
        assert(field.typeId == MetaTypeId::TYPE_STRING);
        reserveSpace(sizeof(std::uint32_t) + 2 * size);
        serializeString(value, size);
    }

    void SerializerQt::Internal::enterBytes(const MetaField& field, Bytes&& value)
    {
        assert(field.typeId == MetaTypeId::TYPE_BYTES);
        enterBytes(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterBytes(const MetaField& field, const BytesElement* value, ssize_t size)
    {
        assert(field.typeId == MetaTypeId::TYPE_BYTES);
        reserveSpace(sizeof(std::uint32_t) + size);
        serialize(value, size);
    }

    void SerializerQt::Internal::enterEnum(const MetaField& field, std::int32_t value)
    {
        assert(field.typeId == MetaTypeId::TYPE_ENUM);
        reserveSpace(sizeof(std::int32_t));
        serialize(value);
    }

    void SerializerQt::Internal::enterEnum(const MetaField& field, std::string&& value)
    {
        std::int32_t enumValue = MetaDataGlobal::instance().getEnumValueByName(field, value);
        enterEnum(field, enumValue);
    }

    void SerializerQt::Internal::enterEnum(const MetaField& field, const char* value, ssize_t size)
    {
        enterEnum(field, std::string(value, size));
    }

    void SerializerQt::Internal::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
    {
        enterArrayBool(field, value);
    }

    void SerializerQt::Internal::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
    {
        assert(field.typeId == MetaTypeId::TYPE_ARRAY_BOOL);
        const ssize_t sizeBytes = (value.size() + 7) / 8;
        reserveSpace(sizeof(std::int32_t) + sizeBytes);
        serializeArrayBool(value);
    }

    void SerializerQt::Internal::enterArrayInt8(const MetaField& field, std::vector<std::int8_t>&& value)
    {
        enterArrayInt8(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayInt8(const MetaField& field, const std::int8_t* value, ssize_t size)
    {
        assert(field.typeId == MetaTypeId::TYPE_ARRAY_INT8);
        reserveSpace(sizeof(std::int32_t) + size * sizeof(std::int8_t));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayInt16(const MetaField& field, std::vector<std::int16_t>&& value)
    {
        enterArrayInt16(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayInt16(const MetaField& field, const std::int16_t* value, ssize_t size)
    {
        assert(field.typeId == MetaTypeId::TYPE_ARRAY_INT16);
        reserveSpace(sizeof(std::int32_t) + size * sizeof(std::int16_t));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayUInt16(const MetaField& field, std::vector<std::uint16_t>&& value)
    {
        enterArrayUInt16(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayUInt16(const MetaField& field, const std::uint16_t* value, ssize_t size)
    {
        assert(field.typeId == MetaTypeId::TYPE_ARRAY_UINT16);
        reserveSpace(sizeof(std::int32_t) + size * sizeof(std::uint16_t));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
    {
        enterArrayInt32(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayInt32(const MetaField& field, const std::int32_t* value, ssize_t size)
    {
        assert(field.typeId == MetaTypeId::TYPE_ARRAY_INT32);
        reserveSpace(sizeof(std::int32_t) + size * sizeof(std::int32_t));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
    {
        enterArrayUInt32(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, ssize_t size)
    {
        assert(field.typeId == MetaTypeId::TYPE_ARRAY_UINT32);
        reserveSpace(sizeof(std::int32_t) + size * sizeof(std::uint32_t));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
    {
        enterArrayInt64(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayInt64(const MetaField& field, const std::int64_t* value, ssize_t size)
    {
        assert(field.typeId == MetaTypeId::TYPE_ARRAY_INT64);
        reserveSpace(sizeof(std::int32_t) + size * sizeof(std::int64_t));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
    {
        enterArrayUInt64(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, ssize_t size)
    {
        assert(field.typeId == MetaTypeId::TYPE_ARRAY_UINT64);
        reserveSpace(sizeof(std::int32_t) + size * sizeof(std::uint64_t));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
    {
        enterArrayFloat(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayFloat(const MetaField& field, const float* value, ssize_t size)
    {
        assert(field.typeId == MetaTypeId::TYPE_ARRAY_FLOAT);
        reserveSpace(sizeof(std::int32_t) + size * sizeof(double));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
    {
        enterArrayDouble(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayDouble(const MetaField& field, const double* value, ssize_t size)
    {
        assert(field.typeId == MetaTypeId::TYPE_ARRAY_DOUBLE);
        reserveSpace(sizeof(std::int32_t) + size * sizeof(double));
        serialize(value, size);
    }

    void SerializerQt::Internal::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
    {
        enterArrayString(field, value);
    }

    void SerializerQt::Internal::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
    {
        assert(field.typeId == MetaTypeId::TYPE_ARRAY_STRING);
        std::uint32_t len = sizeof(std::uint32_t);
        for (size_t i = 0; i < value.size(); ++i)
        {
            len += sizeof(std::uint32_t) + 2 * static_cast<std::uint32_t>(value.size());
        }
        reserveSpace(len);
        serializeArrayString(value);
    }

    void SerializerQt::Internal::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
    {
        enterArrayBytes(field, value);
    }

    void SerializerQt::Internal::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
    {
        assert(field.typeId == MetaTypeId::TYPE_ARRAY_BYTES);
        std::uint32_t len = sizeof(std::uint32_t);
        for (size_t i = 0; i < value.size(); ++i)
        {
            len += sizeof(std::uint32_t) + static_cast<std::uint32_t>(value.size());
        }
        reserveSpace(len);
        serializeArrayBytes(value);
    }

    void SerializerQt::Internal::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
    {
        enterArrayEnum(field, value.data(), value.size());
    }

    void SerializerQt::Internal::enterArrayEnum(const MetaField& field, const std::int32_t* value, ssize_t size)
    {
        assert(field.typeId == MetaTypeId::TYPE_ARRAY_ENUM);
    }

    void SerializerQt::Internal::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
    {
        enterArrayEnum(field, value);
    }

    void SerializerQt::Internal::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
    {
        assert(field.typeId == MetaTypeId::TYPE_ARRAY_ENUM);
    }


    void SerializerQt::Internal::serialize(std::int8_t value)
    {
        serialize(static_cast<std::uint8_t>(value));
    }

    void SerializerQt::Internal::serialize(std::uint8_t value)
    {
        *m_buffer = value;
        ++m_buffer;
    }

    void SerializerQt::Internal::serialize(std::int16_t value)
    {
        serialize(static_cast<std::uint16_t>(value));
    }

    void SerializerQt::Internal::serialize(std::uint16_t value)
    {
        *m_buffer = (value >> 8) & 0xff;
        ++m_buffer;
        *m_buffer = (value) & 0xff;
        ++m_buffer;
    }

    void SerializerQt::Internal::serialize(std::int32_t value)
    {
        serialize(static_cast<std::uint32_t>(value));
    }

    void SerializerQt::Internal::serialize(std::uint32_t value)
    {
        *m_buffer = (value >> 24) & 0xff;
        ++m_buffer;
        *m_buffer = (value >> 16) & 0xff;
        ++m_buffer;
        *m_buffer = (value >> 8) & 0xff;
        ++m_buffer;
        *m_buffer = (value) & 0xff;
        ++m_buffer;
    }

    void SerializerQt::Internal::serialize(std::int64_t value)
    {
        serialize(static_cast<std::uint64_t>(value));
    }

    void SerializerQt::Internal::serialize(std::uint64_t value)
    {
        *m_buffer = (value >> 56) & 0xff;
        ++m_buffer;
        *m_buffer = (value >> 48) & 0xff;
        ++m_buffer;
        *m_buffer = (value >> 40) & 0xff;
        ++m_buffer;
        *m_buffer = (value >> 32) & 0xff;
        ++m_buffer;
        *m_buffer = (value >> 24) & 0xff;
        ++m_buffer;
        *m_buffer = (value >> 16) & 0xff;
        ++m_buffer;
        *m_buffer = (value >> 8) & 0xff;
        ++m_buffer;
        *m_buffer = (value) & 0xff;
        ++m_buffer;
    }

    void SerializerQt::Internal::serialize(float value)
    {
        union
        {
            std::uint64_t v;
            double d;
        } u;
        u.d = static_cast<double>(value);
        serialize(u.v);
    }

    void SerializerQt::Internal::serialize(double value)
    {
        union
        {
            std::uint64_t v;
            double d;
        } u;
        u.d = value;
        serialize(u.v);
    }

    void SerializerQt::Internal::serializeString(const std::string& value)
    {
        std::u16string u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(value);
        assert(u16.size() <= value.size());
        serialize(u16.data(), u16.size() * 2);
    }
        
    void SerializerQt::Internal::serializeString(const char* value, ssize_t size)
    {
        serializeString(std::string(value, value + size));
    }

    template<class T>
    void SerializerQt::Internal::serialize(const T* value, ssize_t size)
    {
        std::uint32_t s = static_cast<std::uint32_t>(size);
        serialize(s);
        for (std::uint32_t i = 0; i < s; ++i)
        {
            serialize(value[i]);
        }
    }

    void SerializerQt::Internal::serializeArrayBool(const std::vector<bool>& value)
    {
        std::uint32_t s = static_cast<std::uint32_t>(value.size());
        serialize(s);
        const ssize_t sizeBytes = (value.size() + 7) / 8;
        for (ssize_t i = 0; i < sizeBytes; ++i)
        {
            m_buffer[i] = 0;
        }
        for (std::uint32_t i = 0; i < s; ++i)
        {
            if (value[i])
            {
                const std::uint32_t indexBytes = i / 8;
                const std::uint32_t indexBits = i % 8;
                m_buffer[indexBytes] |= (1 << indexBits);
            }
        }
        m_buffer += sizeBytes;
    }

    void SerializerQt::Internal::serializeArrayString(const std::vector<std::string>& value)
    {
        std::uint32_t s = static_cast<std::uint32_t>(value.size());
        serialize(s);
        for (std::uint32_t i = 0; i < s; ++i)
        {
            serializeString(value[i]);
        }
    }

    void SerializerQt::Internal::serializeArrayBytes(const std::vector<Bytes>& value)
    {
        std::uint32_t s = static_cast<std::uint32_t>(value.size());
        serialize(s);
        for (std::uint32_t i = 0; i < s; ++i)
        {
            serialize(value[i].data(), value[i].size());
        }
    }

    void SerializerQt::Internal::reserveSpace(ssize_t space)
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

            m_bufferStart = bufferStartNew;
            m_bufferEnd = m_bufferStart + sizeNew;
            m_buffer = m_bufferStart;
        }
    }

    void SerializerQt::Internal::resizeBuffer()
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



}   // namespace finalmq
