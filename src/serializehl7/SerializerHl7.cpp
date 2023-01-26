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
#include "finalmq/serialize/ParserProcessValuesInOrder.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/helpers/Utils.h"

#include <assert.h>
#include <algorithm>
#include <cmath>

namespace finalmq {


SerializerHl7::SerializerHl7(IZeroCopyBuffer& buffer, int maxBlockSize, bool enumAsString)
    : ParserConverter()
    , m_internal(buffer, maxBlockSize, enumAsString)
    , m_parserProcessValuesInOrder()
{
    m_parserProcessValuesInOrder = std::make_unique<ParserProcessValuesInOrder>(false, &m_internal);
    setVisitor(*m_parserProcessValuesInOrder);
}



SerializerHl7::Internal::Internal(IZeroCopyBuffer& buffer, int maxBlockSize, bool enumAsString)
    : m_uniqueHl7Builder(std::make_unique<Hl7Builder>(buffer, maxBlockSize, "|^~\\&"))
    , m_hl7Builder(*m_uniqueHl7Builder.get())
    , m_enumAsString(enumAsString)
{
}


static std::string removeNamespace(const std::string& typeName)
{
    return typeName.substr(typeName.find_last_of('.') + 1);
}


// IParserVisitor
void SerializerHl7::Internal::notifyError(const char* str, const char* message)
{
}

void SerializerHl7::Internal::startStruct(const MetaStruct& stru)
{
    std::vector<std::string> splitString;
    Utils::split(stru.getTypeName(), 0, stru.getTypeName().size(), '_', splitString);

    if (splitString.size() >= 1)
    {
        m_messageCode = removeNamespace(splitString[0]);
    }
    if (splitString.size() >= 2)
    {
        m_triggerEvent = removeNamespace(splitString[1]);
    }
    m_messageStructure = removeNamespace(stru.getTypeName());

}


void SerializerHl7::Internal::finished()
{
    m_hl7Builder.finished();
}


void SerializerHl7::Internal::enterStruct(const MetaField& field)
{
    if (!m_indexOfLeyer.empty())
    {
        ++m_indexOfLeyer.back();
    }
    m_indexOfLeyer.push_back(-1);

    const MetaStruct* stru = MetaDataGlobal::instance().getStruct(field);
    bool isSegment = (stru && (stru->getFlags() & METASTRUCTFLAG_HL7_SEGMENT));
    bool isStruct = (stru && (stru->getFlags() & METASTRUCTFLAG_HL7_STRUCT));

    if (!isStruct)
    {
        m_hl7Builder.enterStruct();
    }

    if ((m_levelSegment > 0) || isSegment)
    {
        if (m_levelSegment == 0)
        {
            m_hl7Builder.enterString(removeNamespace(field.typeName));
        }
        ++m_levelSegment;
    }
}

void SerializerHl7::Internal::exitStruct(const MetaField& field)
{
    m_indexOfLeyer.pop_back();
    if (m_levelSegment > 0)
    {
        --m_levelSegment;
    }

    const MetaStruct* stru = MetaDataGlobal::instance().getStruct(field);
    bool isStruct = (stru && (stru->getFlags() & METASTRUCTFLAG_HL7_STRUCT));
    if (!isStruct)
    {
        m_hl7Builder.exitStruct();
    }
}

void SerializerHl7::Internal::enterStructNull(const MetaField& field)
{
    if (!m_indexOfLeyer.empty())
    {
        ++m_indexOfLeyer.back();
    }

    if (m_levelSegment > 0)
    {
        m_hl7Builder.enterEmpty();
    }
}


void SerializerHl7::Internal::enterArrayStruct(const MetaField& field)
{
    if (!m_indexOfLeyer.empty())
    {
        ++m_indexOfLeyer.back();
    }
    if (m_levelSegment > 0)
    {
        m_hl7Builder.enterArray();
    }
}

void SerializerHl7::Internal::exitArrayStruct(const MetaField& field)
{
    if (!m_indexOfLeyer.empty())
    {
        ++m_indexOfLeyer.back();
    }
    if (m_levelSegment > 0)
    {
        m_hl7Builder.exitArray();
    }
}


void SerializerHl7::Internal::enterBool(const MetaField& field, bool value)
{
    if (!m_indexOfLeyer.empty())
    {
        ++m_indexOfLeyer.back();
    }
    if (m_levelSegment > 0)
    {
        m_hl7Builder.enterInt64(value ? 1 : 0);
    }
}

void SerializerHl7::Internal::enterInt32(const MetaField& field, std::int32_t value)
{
    if (!m_indexOfLeyer.empty())
    {
        ++m_indexOfLeyer.back();
    }
    if (m_levelSegment > 0)
    {
        m_hl7Builder.enterInt64(value);
    }
}

void SerializerHl7::Internal::enterUInt32(const MetaField& field, std::uint32_t value)
{
    if (!m_indexOfLeyer.empty())
    {
        ++m_indexOfLeyer.back();
    }
    if (m_levelSegment > 0)
    {
        m_hl7Builder.enterUInt64(value);
    }
}

void SerializerHl7::Internal::enterInt64(const MetaField& field, std::int64_t value)
{
    if (!m_indexOfLeyer.empty())
    {
        ++m_indexOfLeyer.back();
    }
    if (m_levelSegment > 0)
    {
        m_hl7Builder.enterInt64(value);
    }
}

void SerializerHl7::Internal::enterUInt64(const MetaField& field, std::uint64_t value)
{
    if (!m_indexOfLeyer.empty())
    {
        ++m_indexOfLeyer.back();
    }
    if (m_levelSegment > 0)
    {
        m_hl7Builder.enterUInt64(value);
    }
}


void SerializerHl7::Internal::enterFloat(const MetaField& field, float value)
{
    if (!m_indexOfLeyer.empty())
    {
        ++m_indexOfLeyer.back();
    }
    if (m_levelSegment > 0)
    {
        m_hl7Builder.enterDouble(value);
    }
}

void SerializerHl7::Internal::enterDouble(const MetaField& field, double value)
{
    if (!m_indexOfLeyer.empty())
    {
        ++m_indexOfLeyer.back();
    }
    if (m_levelSegment > 0)
    {
        m_hl7Builder.enterDouble(value);
    }
}

void SerializerHl7::Internal::enterString(const MetaField& field, std::string&& value)
{
    enterString(field, value.c_str(), value.size());
}

void SerializerHl7::Internal::enterString(const MetaField& field, const char* value, ssize_t size)
{
    if (!m_indexOfLeyer.empty())
    {
        ++m_indexOfLeyer.back();
    }
    if (m_levelSegment > 0)
    {
        if (m_indexOfLeyer.size() == 2 && m_indexOfLeyer[0] == 8 && m_indexOfLeyer[1] == 0)
        {
            m_hl7Builder.enterString(m_messageCode);
        }
        else if (m_indexOfLeyer.size() == 2 && m_indexOfLeyer[0] == 8 && m_indexOfLeyer[1] == 1)
        {
            m_hl7Builder.enterString(m_triggerEvent);
        }
        else if (m_indexOfLeyer.size() == 2 && m_indexOfLeyer[0] == 8 && m_indexOfLeyer[1] == 2)
        {
            m_hl7Builder.enterString(m_messageStructure);
        }
        else
        {
            m_hl7Builder.enterString(value, size);
        }
    }
}

void SerializerHl7::Internal::enterBytes(const MetaField& field, Bytes&& value)
{
}

void SerializerHl7::Internal::enterBytes(const MetaField& field, const BytesElement* value, ssize_t size)
{
}

void SerializerHl7::Internal::enterEnum(const MetaField& field, std::int32_t value)
{
}


void SerializerHl7::Internal::enterEnum(const MetaField& field, std::string&& value)
{
}

void SerializerHl7::Internal::enterEnum(const MetaField& field, const char* value, ssize_t size)
{
}

void SerializerHl7::Internal::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
}

void SerializerHl7::Internal::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
{
}

void SerializerHl7::Internal::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
{
}

void SerializerHl7::Internal::enterArrayInt32(const MetaField& field, const std::int32_t* value, ssize_t size)
{
}

void SerializerHl7::Internal::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
{
}

void SerializerHl7::Internal::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, ssize_t size)
{
}

void SerializerHl7::Internal::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
{
}

void SerializerHl7::Internal::enterArrayInt64(const MetaField& field, const std::int64_t* value, ssize_t size)
{
}

void SerializerHl7::Internal::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
{
}

void SerializerHl7::Internal::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, ssize_t size)
{
}

void SerializerHl7::Internal::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
{
}

void SerializerHl7::Internal::enterArrayFloat(const MetaField& field, const float* value, ssize_t size)
{
}

void SerializerHl7::Internal::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
{
}

void SerializerHl7::Internal::enterArrayDouble(const MetaField& field, const double* value, ssize_t size)
{
}

void SerializerHl7::Internal::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
{
}

void SerializerHl7::Internal::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
{
}

void SerializerHl7::Internal::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
{
}

void SerializerHl7::Internal::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
}

void SerializerHl7::Internal::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
}

void SerializerHl7::Internal::enterArrayEnum(const MetaField& field, const std::int32_t* value, ssize_t size)
{
}

void SerializerHl7::Internal::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
{
}

void SerializerHl7::Internal::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
{
}

}   // namespace finalmq
