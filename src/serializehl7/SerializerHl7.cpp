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


#include "finalmq/serializehl7/SerializerHl7.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/helpers/base64.h"
#include "finalmq/helpers/Utils.h"
#include "finalmq/helpers/ZeroCopyBuffer.h"
#include "finalmq/jsonvariant/VariantToJson.h"

#include <assert.h>
#include <algorithm>
#include <cmath>

namespace finalmq {



SerializerHl7::SerializerHl7(IZeroCopyBuffer& buffer, int maxBlockSize)
    : ParserProcessDefaultValues(false)
    , m_internal(buffer, maxBlockSize)
{
    setVisitor(m_internal);
}



SerializerHl7::Internal::Internal(IZeroCopyBuffer& buffer, int maxBlockSize)
    : m_uniqueHl7Builder(std::make_unique<Hl7Builder>(buffer, maxBlockSize, "|^~\\&"))
    , m_hl7Builder(*m_uniqueHl7Builder.get())
{
}


// IParserVisitor
void SerializerHl7::Internal::notifyError(const char* /*str*/, const char* /*message*/)
{
}

void SerializerHl7::Internal::startStruct(const MetaStruct& stru)
{
    std::vector<std::string> splitString;
    std::string messageStructure = stru.getTypeNameWithoutNamespace();
    Utils::split(messageStructure, 0, messageStructure.size(), '_', splitString);

    m_indexOfLayer.push_back(-1);

    int indexMessageType[3] = { 0, 8, 0 };
    if (splitString.size() >= 1)
    {
        m_hl7Builder.enterString(indexMessageType, 3, 0, std::move(splitString[0]));
    }
    if (splitString.size() >= 2)
    {
        m_hl7Builder.enterString(indexMessageType, 3, 1, std::move(splitString[1]));
    }
    m_hl7Builder.enterString(indexMessageType, 3, 2, std::move(messageStructure));
}


void SerializerHl7::Internal::finished()
{
    m_hl7Builder.finished();
}


void SerializerHl7::Internal::enterStruct(const MetaField& field)
{
    const MetaStruct* stru = MetaDataGlobal::instance().getStruct(field);
    bool isSegment = (stru && (stru->getFlags() & METASTRUCTFLAG_HL7_SEGMENT));

    if (m_inSegment || isSegment)
    {
        if (isSegment)
        {
            assert(m_indexOfLayer.size() == 1);
            ++m_indexOfLayer.back();
            const std::string& typeNameWithoutNamespace = stru->getTypeNameWithoutNamespace();
            m_hl7Builder.enterString(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), -1, typeNameWithoutNamespace.c_str(), typeNameWithoutNamespace.size());
            m_inSegment = true;
        }
        else
        {
            // at index 2 there is the array index
            if (m_indexOfLayer.size() != 2)
            {
                m_indexOfLayer.push_back(field.index);
            }
            // at size = 2 add the array index (at index 2)
            if (m_indexOfLayer.size() == 2)
            {
                if (m_ixArrayStruct == NO_ARRAY_STRUCT)
                {
                    m_indexOfLayer.push_back(0);
                }
                else
                {
                    ++m_ixArrayStruct;
                    m_hl7Builder.addArrayStruct(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), m_ixArrayStruct);
                    m_indexOfLayer.push_back(m_ixArrayStruct);
                }
            }
        }
    }
}

void SerializerHl7::Internal::exitStruct(const MetaField& /*field*/)
{
    if (m_indexOfLayer.size() > 1)
    {
        if (m_indexOfLayer.size() == 3 && m_ixArrayStruct == NO_ARRAY_STRUCT)
        {
            m_indexOfLayer.pop_back();
        }
        m_indexOfLayer.pop_back();
    }
    else
    {
        m_inSegment = false;
    }
}

void SerializerHl7::Internal::enterStructNull(const MetaField& /*field*/)
{
}


void SerializerHl7::Internal::enterArrayStruct(const MetaField& field)
{
    if (m_inSegment)
    {
        m_ixArrayStruct = -1;
        assert(m_indexOfLayer.size() == 1);
        m_indexOfLayer.push_back(field.index);
    }
}

void SerializerHl7::Internal::exitArrayStruct(const MetaField& /*field*/)
{
    if (m_inSegment)
    {
        m_ixArrayStruct = NO_ARRAY_STRUCT;
        assert(m_indexOfLayer.size() == 2);
        m_indexOfLayer.pop_back();
    }
}


void SerializerHl7::Internal::enterBool(const MetaField& field, bool value)
{
    if (!m_indexOfLayer.empty())
    {
        m_hl7Builder.enterInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value ? 1 : 0);
    }
}

void SerializerHl7::Internal::enterInt8(const MetaField& field, std::int8_t value)
{
    if (!m_indexOfLayer.empty())
    {
        m_hl7Builder.enterInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value);
    }
}

void SerializerHl7::Internal::enterUInt8(const MetaField& field, std::uint8_t value)
{
    if (!m_indexOfLayer.empty())
    {
        m_hl7Builder.enterUInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value);
    }
}

void SerializerHl7::Internal::enterInt16(const MetaField& field, std::int16_t value)
{
    if (!m_indexOfLayer.empty())
    {
        m_hl7Builder.enterInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value);
    }
}

void SerializerHl7::Internal::enterUInt16(const MetaField& field, std::uint16_t value)
{
    if (!m_indexOfLayer.empty())
    {
        m_hl7Builder.enterUInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value);
    }
}

void SerializerHl7::Internal::enterInt32(const MetaField& field, std::int32_t value)
{
    if (!m_indexOfLayer.empty())
    {
        m_hl7Builder.enterInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value);
    }
}

void SerializerHl7::Internal::enterUInt32(const MetaField& field, std::uint32_t value)
{
    if (!m_indexOfLayer.empty())
    {
        m_hl7Builder.enterUInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value);
    }
}

void SerializerHl7::Internal::enterInt64(const MetaField& field, std::int64_t value)
{
    if (!m_indexOfLayer.empty())
    {
        m_hl7Builder.enterInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value);
    }
}

void SerializerHl7::Internal::enterUInt64(const MetaField& field, std::uint64_t value)
{
    if (!m_indexOfLayer.empty())
    {
        m_hl7Builder.enterUInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value);
    }
}


void SerializerHl7::Internal::enterFloat(const MetaField& field, float value)
{
    if (!m_indexOfLayer.empty())
    {
        m_hl7Builder.enterDouble(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value);
    }
}

void SerializerHl7::Internal::enterDouble(const MetaField& field, double value)
{
    if (!m_indexOfLayer.empty())
    {
        m_hl7Builder.enterDouble(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value);
    }
}


bool SerializerHl7::Internal::filterEnterString(size_t valueSize) const
{
    if (!m_indexOfLayer.empty() && valueSize > 0)
    {
        // skip message type
        if (!(m_indexOfLayer.size() == 3 && m_indexOfLayer[0] == 0 && m_indexOfLayer[1] == 8))
        {
            return true;
        }
    }
    return false;
}

void SerializerHl7::Internal::enterString(const MetaField& field, std::string&& value)
{
    if (filterEnterString(value.size()))
    {
        m_hl7Builder.enterString(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, std::move(value));
    }
}

void SerializerHl7::Internal::enterString(const MetaField& field, const char* value, ssize_t size)
{
    if (filterEnterString(size))
    {
        m_hl7Builder.enterString(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value, size);
    }
}

void SerializerHl7::Internal::enterBytes(const MetaField& field, Bytes&& value)
{
    // convert to base64
    std::string base64;
    Base64::encode(value, base64);
    enterString(field, std::move(base64));
}

void SerializerHl7::Internal::enterBytes(const MetaField& field, const BytesElement* value, ssize_t size)
{
    enterBytes(field, Bytes(value, value + size));
}

void SerializerHl7::Internal::enterEnum(const MetaField& field, std::int32_t value)
{
    const std::string& name = MetaDataGlobal::instance().getEnumAliasByValue(field, value);
    enterString(field, std::string(name));
}


void SerializerHl7::Internal::enterEnum(const MetaField& field, std::string&& value)
{
    enterString(field, std::move(value));
}

void SerializerHl7::Internal::enterEnum(const MetaField& field, const char* value, ssize_t size)
{
    enterString(field, value, size);
}

void SerializerHl7::Internal::enterJsonString(const MetaField& field, std::string&& value)
{
    enterString(field, std::move(value));
}
void SerializerHl7::Internal::enterJsonString(const MetaField& field, const char* value, ssize_t size)
{
    enterString(field, value, size);
}
void SerializerHl7::Internal::enterJsonVariant(const MetaField& field, const Variant& value)
{
    ZeroCopyBuffer buffer;
    VariantToJson variantToJson(buffer);
    variantToJson.parse(value);
    std::string json = buffer.getData();

    enterJsonString(field, std::move(json));
}
void SerializerHl7::Internal::enterJsonVariantMove(const MetaField& field, Variant&& value)
{
    enterJsonVariant(field, value);
}

void SerializerHl7::Internal::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
    enterArrayBool(field, value);
}

void SerializerHl7::Internal::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
{
    if (!m_indexOfLayer.empty())
    {
        for (size_t i = 0; i < value.size(); ++i)
        {
            m_hl7Builder.enterInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value[i] ? 1 : 0);
        }
    }
}

void SerializerHl7::Internal::enterArrayInt8(const MetaField& field, std::vector<std::int8_t>&& value)
{
    enterArrayInt8(field, value.data(), value.size());
}

void SerializerHl7::Internal::enterArrayInt8(const MetaField& field, const std::int8_t* value, ssize_t size)
{
    if (!m_indexOfLayer.empty())
    {
        for (ssize_t i = 0; i < size; ++i)
        {
            m_hl7Builder.enterInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value[i]);
        }
    }
}

void SerializerHl7::Internal::enterArrayInt16(const MetaField& field, std::vector<std::int16_t>&& value)
{
    enterArrayInt16(field, value.data(), value.size());
}

void SerializerHl7::Internal::enterArrayInt16(const MetaField& field, const std::int16_t* value, ssize_t size)
{
    if (!m_indexOfLayer.empty())
    {
        for (ssize_t i = 0; i < size; ++i)
        {
            m_hl7Builder.enterInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value[i]);
        }
    }
}

void SerializerHl7::Internal::enterArrayUInt16(const MetaField& field, std::vector<std::uint16_t>&& value)
{
    enterArrayUInt16(field, value.data(), value.size());
}

void SerializerHl7::Internal::enterArrayUInt16(const MetaField& field, const std::uint16_t* value, ssize_t size)
{
    if (!m_indexOfLayer.empty())
    {
        for (ssize_t i = 0; i < size; ++i)
        {
            m_hl7Builder.enterUInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value[i]);
        }
    }
}

void SerializerHl7::Internal::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
{
    enterArrayInt32(field, value.data(), value.size());
}

void SerializerHl7::Internal::enterArrayInt32(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    if (!m_indexOfLayer.empty())
    {
        for (ssize_t i = 0; i < size; ++i)
        {
            m_hl7Builder.enterInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value[i]);
        }
    }
}

void SerializerHl7::Internal::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
{
    enterArrayUInt32(field, value.data(), value.size());
}

void SerializerHl7::Internal::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, ssize_t size)
{
    if (!m_indexOfLayer.empty())
    {
        for (ssize_t i = 0; i < size; ++i)
        {
            m_hl7Builder.enterUInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value[i]);
        }
    }
}

void SerializerHl7::Internal::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
{
    enterArrayInt64(field, value.data(), value.size());
}

void SerializerHl7::Internal::enterArrayInt64(const MetaField& field, const std::int64_t* value, ssize_t size)
{
    if (!m_indexOfLayer.empty())
    {
        for (ssize_t i = 0; i < size; ++i)
        {
            m_hl7Builder.enterInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value[i]);
        }
    }
}

void SerializerHl7::Internal::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
{
    enterArrayUInt64(field, value.data(), value.size());
}

void SerializerHl7::Internal::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, ssize_t size)
{
    if (!m_indexOfLayer.empty())
    {
        for (ssize_t i = 0; i < size; ++i)
        {
            m_hl7Builder.enterUInt64(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value[i]);
        }
    }
}

void SerializerHl7::Internal::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
{
    enterArrayFloat(field, value.data(), value.size());
}

void SerializerHl7::Internal::enterArrayFloat(const MetaField& field, const float* value, ssize_t size)
{
    if (!m_indexOfLayer.empty())
    {
        for (ssize_t i = 0; i < size; ++i)
        {
            m_hl7Builder.enterDouble(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value[i]);
        }
    }
}

void SerializerHl7::Internal::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
{
    enterArrayDouble(field, value.data(), value.size());
}

void SerializerHl7::Internal::enterArrayDouble(const MetaField& field, const double* value, ssize_t size)
{
    if (!m_indexOfLayer.empty())
    {
        for (ssize_t i = 0; i < size; ++i)
        {
            m_hl7Builder.enterDouble(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value[i]);
        }
    }
}

void SerializerHl7::Internal::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
{
    if (!m_indexOfLayer.empty())
    {
        for (size_t i = 0; i < value.size(); ++i)
        {
            m_hl7Builder.enterString(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, std::move(value[i]));
        }
    }
}

void SerializerHl7::Internal::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
{
    if (!m_indexOfLayer.empty())
    {
        for (size_t i = 0; i < value.size(); ++i)
        {
            m_hl7Builder.enterString(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value[i].c_str(), value[i].size());
        }
    }
}

void SerializerHl7::Internal::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
{
    enterArrayBytes(field, value);
}

void SerializerHl7::Internal::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
    if (!m_indexOfLayer.empty())
    {
        for (size_t i = 0; i < value.size(); ++i)
        {
            // convert to base64
            std::string base64;
            Base64::encode(value[i], base64);
            m_hl7Builder.enterString(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, std::move(base64));
        }
    }
}

void SerializerHl7::Internal::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
    enterArrayEnum(field, value.data(), value.size());
}

void SerializerHl7::Internal::enterArrayEnum(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    if (!m_indexOfLayer.empty())
    {
        for (ssize_t i = 0; i < size; ++i)
        {
            const std::string& name = MetaDataGlobal::instance().getEnumAliasByValue(field, value[i]);
            m_hl7Builder.enterString(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, name.c_str(), name.size());
        }
    }
}

void SerializerHl7::Internal::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
{
    if (!m_indexOfLayer.empty())
    {
        for (size_t i = 0; i < value.size(); ++i)
        {
            m_hl7Builder.enterString(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, std::move(value[i]));
        }
    }
}

void SerializerHl7::Internal::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
{
    if (!m_indexOfLayer.empty())
    {
        for (size_t i = 0; i < value.size(); ++i)
        {
            m_hl7Builder.enterString(m_indexOfLayer.data(), static_cast<int>(m_indexOfLayer.size()), field.index, value[i].c_str(), value[i].size());
        }
    }
}

}   // namespace finalmq
