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

#include "finalmq/serializejson/SerializerJson.h"

#include <algorithm>
#include <cmath>

#include <assert.h>

#include "finalmq/helpers/base64.h"
#include "finalmq/helpers/ZeroCopyBuffer.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/serialize/ParserAbortAndIndex.h"
#include "finalmq/serialize/ParserProcessDefaultValues.h"
#include "finalmq/jsonvariant/VariantToJson.h"

namespace finalmq
{
SerializerJson::SerializerJson(IZeroCopyBuffer& buffer, int maxBlockSize, bool enumAsString, bool skipDefaultValues)
    : ParserConverter(), m_internal(buffer, maxBlockSize, enumAsString), m_parserAbortAndIndex(), m_parserProcessDefaultValues()
{
    m_parserAbortAndIndex = std::make_unique<ParserAbortAndIndex>(&m_internal);
    m_parserProcessDefaultValues = std::make_unique<ParserProcessDefaultValues>(skipDefaultValues, m_parserAbortAndIndex.get());
    ParserConverter::setVisitor(*m_parserProcessDefaultValues);
}

SerializerJson::Internal::Internal(IZeroCopyBuffer& buffer, int maxBlockSize, bool enumAsString)
    : m_uniqueJsonBuilder(std::make_unique<JsonBuilder>(buffer, maxBlockSize)), m_jsonBuilder(*m_uniqueJsonBuilder.get()), m_enumAsString(enumAsString)
{
}

void SerializerJson::Internal::setKey(const MetaField& field)
{
    const std::string& name = field.name;
    if (!name.empty())
    {
        m_jsonBuilder.enterKey(name.c_str(), name.size());
    }
}

// IParserVisitor
void SerializerJson::Internal::notifyError(const char* /*str*/, const char* /*message*/)
{
}

void SerializerJson::Internal::startStruct(const MetaStruct& /*stru*/)
{
    // outer curly brackets
    m_jsonBuilder.enterObject();
}

void SerializerJson::Internal::finished()
{
    // outer curly brackets
    m_jsonBuilder.exitObject();
    m_jsonBuilder.finished();
}

void SerializerJson::Internal::enterStruct(const MetaField& field)
{
    setKey(field);
    m_jsonBuilder.enterObject();
}

void SerializerJson::Internal::exitStruct(const MetaField& /*field*/)
{
    m_jsonBuilder.exitObject();
}

void SerializerJson::Internal::enterStructNull(const MetaField& field)
{
    setKey(field);
    m_jsonBuilder.enterNull();
}

void SerializerJson::Internal::enterArrayStruct(const MetaField& field)
{
    setKey(field);
    m_jsonBuilder.enterArray();
}

void SerializerJson::Internal::exitArrayStruct(const MetaField& /*field*/)
{
    m_jsonBuilder.exitArray();
}

void SerializerJson::Internal::enterBool(const MetaField& field, bool value)
{
    assert(field.typeId == MetaTypeId::TYPE_BOOL);
    setKey(field);
    m_jsonBuilder.enterBool(value);
}

void SerializerJson::Internal::enterInt8(const MetaField& field, std::int8_t value)
{
    assert(field.typeId == MetaTypeId::TYPE_INT8);
    setKey(field);
    m_jsonBuilder.enterInt32(value);
}

void SerializerJson::Internal::enterUInt8(const MetaField& field, std::uint8_t value)
{
    assert(field.typeId == MetaTypeId::TYPE_UINT8);
    setKey(field);
    m_jsonBuilder.enterUInt32(value);
}

void SerializerJson::Internal::enterInt16(const MetaField& field, std::int16_t value)
{
    assert(field.typeId == MetaTypeId::TYPE_INT16);
    setKey(field);
    m_jsonBuilder.enterInt32(value);
}

void SerializerJson::Internal::enterUInt16(const MetaField& field, std::uint16_t value)
{
    assert(field.typeId == MetaTypeId::TYPE_UINT16);
    setKey(field);
    m_jsonBuilder.enterUInt32(value);
}

void SerializerJson::Internal::enterInt32(const MetaField& field, std::int32_t value)
{
    assert(field.typeId == MetaTypeId::TYPE_INT32);
    setKey(field);
    m_jsonBuilder.enterInt32(value);
}

void SerializerJson::Internal::enterUInt32(const MetaField& field, std::uint32_t value)
{
    assert(field.typeId == MetaTypeId::TYPE_UINT32);
    setKey(field);
    m_jsonBuilder.enterUInt32(value);
}

void SerializerJson::Internal::enterInt64(const MetaField& field, std::int64_t value)
{
    assert(field.typeId == MetaTypeId::TYPE_INT64);
    setKey(field);
    m_jsonBuilder.enterString(std::to_string(value));
}

void SerializerJson::Internal::enterUInt64(const MetaField& field, std::uint64_t value)
{
    assert(field.typeId == MetaTypeId::TYPE_UINT64);
    setKey(field);
    m_jsonBuilder.enterString(std::to_string(value));
}

void SerializerJson::Internal::handleDouble(double value)
{
#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
    if (std::isnan(value))
    {
        m_jsonBuilder.enterString("NaN");
    }
    else if (value == std::numeric_limits<double>::infinity())
    {
        m_jsonBuilder.enterString("Infinity");
    }
    else if (value == -std::numeric_limits<double>::infinity())
    {
        m_jsonBuilder.enterString("-Infinity");
    }
    else
    {
        m_jsonBuilder.enterDouble(value);
    }
#ifndef WIN32
#pragma GCC diagnostic pop
#endif
}

void SerializerJson::Internal::enterFloat(const MetaField& field, float value)
{
    assert(field.typeId == MetaTypeId::TYPE_FLOAT);
    setKey(field);
    handleDouble(value);
}

void SerializerJson::Internal::enterDouble(const MetaField& field, double value)
{
    assert(field.typeId == MetaTypeId::TYPE_DOUBLE);
    setKey(field);
    handleDouble(value);
}

void SerializerJson::Internal::enterString(const MetaField& field, std::string&& value)
{
    assert(field.typeId == MetaTypeId::TYPE_STRING);
    setKey(field);
    m_jsonBuilder.enterString(value.c_str(), value.size());
}

void SerializerJson::Internal::enterString(const MetaField& field, const char* value, ssize_t size)
{
    assert(field.typeId == MetaTypeId::TYPE_STRING);
    setKey(field);
    m_jsonBuilder.enterString(value, size);
}

void SerializerJson::Internal::enterBytes(const MetaField& field, Bytes&& value)
{
    assert(field.typeId == MetaTypeId::TYPE_BYTES);
    setKey(field);
    // convert to base64
    std::string base64;
    Base64::encode(value, base64);
    m_jsonBuilder.enterString(base64.data(), base64.size());
}

void SerializerJson::Internal::enterBytes(const MetaField& field, const BytesElement* value, ssize_t size)
{
    assert(field.typeId == MetaTypeId::TYPE_BYTES);
    setKey(field);
    // convert to base64
    std::string base64;
    Base64::encode(value, size, base64);
    m_jsonBuilder.enterString(base64.data(), base64.size());
}

void SerializerJson::Internal::enterEnum(const MetaField& field, std::int32_t value)
{
    assert(field.typeId == MetaTypeId::TYPE_ENUM);
    setKey(field);
    if (m_enumAsString)
    {
        const std::string& name = MetaDataGlobal::instance().getEnumAliasByValue(field, value);
        m_jsonBuilder.enterString(name.c_str(), name.size());
    }
    else
    {
        m_jsonBuilder.enterInt32(value);
    }
}

void SerializerJson::Internal::enterEnum(const MetaField& field, std::string&& value)
{
    assert(field.typeId == MetaTypeId::TYPE_ENUM);
    setKey(field);
    if (m_enumAsString)
    {
        m_jsonBuilder.enterString(std::move(value));
    }
    else
    {
        std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(field, value);
        m_jsonBuilder.enterInt32(v);
    }
}

void SerializerJson::Internal::enterEnum(const MetaField& field, const char* value, ssize_t size)
{
    assert(field.typeId == MetaTypeId::TYPE_ENUM);
    setKey(field);
    if (m_enumAsString)
    {
        m_jsonBuilder.enterString(value, size);
    }
    else
    {
        std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(field, std::string(value, size));
        m_jsonBuilder.enterInt32(v);
    }
}

void SerializerJson::Internal::enterJsonString(const MetaField& field, std::string&& value)
{
    enterJsonString(field, value.c_str(), value.size());
}
void SerializerJson::Internal::enterJsonString(const MetaField& field, const char* value, ssize_t size)
{
    assert(field.typeId == MetaTypeId::TYPE_JSON);
    setKey(field);
    m_jsonBuilder.putJson(value, size);
}
void SerializerJson::Internal::enterJsonVariant(const MetaField& field, const Variant& value)
{
    ZeroCopyBuffer buffer;
    VariantToJson variantToJson(buffer);
    variantToJson.parse(value);
    std::string json = buffer.getData();

    enterJsonString(field, json.c_str(), json.size());
}
void SerializerJson::Internal::enterJsonVariantMove(const MetaField& field, Variant&& value)
{
    enterJsonVariant(field, value);
}


void SerializerJson::Internal::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_BOOL);
    setKey(field);
    m_jsonBuilder.enterArray();
    std::for_each(value.begin(), value.end(), [this](bool entry) {
        m_jsonBuilder.enterBool(entry);
    });
    m_jsonBuilder.exitArray();
}

void SerializerJson::Internal::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_BOOL);
    setKey(field);
    m_jsonBuilder.enterArray();
    std::for_each(value.begin(), value.end(), [this](bool entry) {
        m_jsonBuilder.enterBool(entry);
    });
    m_jsonBuilder.exitArray();
}

void SerializerJson::Internal::enterArrayInt8(const MetaField& field, std::vector<std::int8_t>&& value)
{
    enterArrayInt8(field, value.data(), value.size());
}

void SerializerJson::Internal::enterArrayInt8(const MetaField& field, const std::int8_t* value, ssize_t size)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_INT8);
    setKey(field);
    m_jsonBuilder.enterArray();
    std::for_each(value, value + size, [this](std::int8_t entry) {
        m_jsonBuilder.enterInt32(entry);
    });
    m_jsonBuilder.exitArray();
}

void SerializerJson::Internal::enterArrayInt16(const MetaField& field, std::vector<std::int16_t>&& value)
{
    enterArrayInt16(field, value.data(), value.size());
}

void SerializerJson::Internal::enterArrayInt16(const MetaField& field, const std::int16_t* value, ssize_t size)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_INT16);
    setKey(field);
    m_jsonBuilder.enterArray();
    std::for_each(value, value + size, [this](std::int16_t entry) {
        m_jsonBuilder.enterInt32(entry);
    });
    m_jsonBuilder.exitArray();
}

void SerializerJson::Internal::enterArrayUInt16(const MetaField& field, std::vector<std::uint16_t>&& value)
{
    enterArrayUInt16(field, value.data(), value.size());
}

void SerializerJson::Internal::enterArrayUInt16(const MetaField& field, const std::uint16_t* value, ssize_t size)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_UINT16);
    setKey(field);
    m_jsonBuilder.enterArray();
    std::for_each(value, value + size, [this](std::uint16_t entry) {
        m_jsonBuilder.enterUInt32(entry);
    });
    m_jsonBuilder.exitArray();
}

void SerializerJson::Internal::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
{
    enterArrayInt32(field, value.data(), value.size());
}

void SerializerJson::Internal::enterArrayInt32(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_INT32);
    setKey(field);
    m_jsonBuilder.enterArray();
    std::for_each(value, value + size, [this](std::int32_t entry) {
        m_jsonBuilder.enterInt32(entry);
    });
    m_jsonBuilder.exitArray();
}

void SerializerJson::Internal::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
{
    enterArrayUInt32(field, value.data(), value.size());
}

void SerializerJson::Internal::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, ssize_t size)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_UINT32);
    setKey(field);
    m_jsonBuilder.enterArray();
    std::for_each(value, value + size, [this](std::uint32_t entry) {
        m_jsonBuilder.enterUInt32(entry);
    });
    m_jsonBuilder.exitArray();
}

void SerializerJson::Internal::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
{
    enterArrayInt64(field, value.data(), value.size());
}

void SerializerJson::Internal::enterArrayInt64(const MetaField& field, const std::int64_t* value, ssize_t size)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_INT64);
    setKey(field);
    m_jsonBuilder.enterArray();
    std::for_each(value, value + size, [this](std::int64_t entry) {
        m_jsonBuilder.enterString(std::to_string(entry));
    });
    m_jsonBuilder.exitArray();
}

void SerializerJson::Internal::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
{
    enterArrayUInt64(field, value.data(), value.size());
}

void SerializerJson::Internal::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, ssize_t size)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_UINT64);
    setKey(field);
    m_jsonBuilder.enterArray();
    std::for_each(value, value + size, [this](std::uint64_t entry) {
        m_jsonBuilder.enterString(std::to_string(entry));
    });
    m_jsonBuilder.exitArray();
}

void SerializerJson::Internal::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
{
    enterArrayFloat(field, value.data(), value.size());
}

void SerializerJson::Internal::enterArrayFloat(const MetaField& field, const float* value, ssize_t size)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_FLOAT);
    setKey(field);
    m_jsonBuilder.enterArray();
    std::for_each(value, value + size, [this](float entry) {
        handleDouble(entry);
    });
    m_jsonBuilder.exitArray();
}

void SerializerJson::Internal::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
{
    enterArrayDouble(field, value.data(), value.size());
}

void SerializerJson::Internal::enterArrayDouble(const MetaField& field, const double* value, ssize_t size)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_DOUBLE);
    setKey(field);
    m_jsonBuilder.enterArray();
    std::for_each(value, value + size, [this](double entry) {
        handleDouble(entry);
    });
    m_jsonBuilder.exitArray();
}

void SerializerJson::Internal::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
{
    enterArrayString(field, value);
}

void SerializerJson::Internal::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_STRING);
    setKey(field);
    m_jsonBuilder.enterArray();
    std::for_each(value.begin(), value.end(), [this](const std::string& entry) {
        m_jsonBuilder.enterString(entry.c_str(), entry.size());
    });
    m_jsonBuilder.exitArray();
}

void SerializerJson::Internal::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
{
    enterArrayBytes(field, value);
}

void SerializerJson::Internal::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_BYTES);
    setKey(field);
    m_jsonBuilder.enterArray();
    std::for_each(value.begin(), value.end(), [this](const Bytes& entry) {
        // convert to base64
        std::string base64;
        Base64::encode(entry, base64);
        m_jsonBuilder.enterString(base64.data(), base64.size());
    });
    m_jsonBuilder.exitArray();
}

void SerializerJson::Internal::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
    enterArrayEnum(field, value.data(), value.size());
}

void SerializerJson::Internal::enterArrayEnum(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_ENUM);
    setKey(field);
    m_jsonBuilder.enterArray();

    if (m_enumAsString)
    {
        std::for_each(value, value + size, [this, &field](std::int32_t entry) {
            const std::string& name = MetaDataGlobal::instance().getEnumAliasByValue(field, entry);
            m_jsonBuilder.enterString(name.c_str(), name.size());
        });
    }
    else
    {
        std::for_each(value, value + size, [this](std::int32_t entry) {
            m_jsonBuilder.enterInt32(entry);
        });
    }

    m_jsonBuilder.exitArray();
}

void SerializerJson::Internal::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
{
    enterArrayEnum(field, value);
}

void SerializerJson::Internal::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_ENUM);
    setKey(field);
    m_jsonBuilder.enterArray();

    if (m_enumAsString)
    {
        std::for_each(value.begin(), value.end(), [this](const std::string& entry) {
            m_jsonBuilder.enterString(entry.c_str(), entry.size());
        });
    }
    else
    {
        std::for_each(value.begin(), value.end(), [this, &field](const std::string& entry) {
            std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(field, entry);
            m_jsonBuilder.enterInt32(v);
        });
    }

    m_jsonBuilder.exitArray();
}

} // namespace finalmq
