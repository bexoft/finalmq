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


#include "serialize/ParserProcessDefaultValues.h"
#include "metadata/MetaData.h"

#include "conversions/itoa.h"
#include "conversions/dtoa.h"

#include <assert.h>
#include <iostream>
#include <algorithm>


namespace finalmq {


ParserProcessDefaultValues::ParserProcessDefaultValues(bool skipDefaultValues, IParserVisitor* visitor)
    : m_visitor(visitor)
    , m_skipDefaultValues(skipDefaultValues)
{

}

void ParserProcessDefaultValues::setVisitor(IParserVisitor &visitor)
{
    m_visitor = &visitor;
}


// ParserProcessDefaultValues
void ParserProcessDefaultValues::notifyError(const char* str, const char* message)
{
    assert(m_visitor);
    m_visitor->notifyError(str, message);
}

void ParserProcessDefaultValues::finished()
{
    assert(m_visitor);
    m_visitor->finished();
}

void ParserProcessDefaultValues::enterStruct(const MetaField& field)
{
    assert(m_visitor);

    if (!m_skipDefaultValues)
    {
        const MetaStruct* stru = MetaDataGlobal::instance().getStruct(field);
        if (stru)
        {
            m_stackFieldsDone.emplace_back(stru->getFieldsSize(), false);
        }
        else
        {
            m_stackFieldsDone.emplace_back(0, false);
        }
    }

    m_visitor->enterStruct(field);
}
void ParserProcessDefaultValues::exitStruct(const MetaField& field)
{
    if (!m_skipDefaultValues)
    {
        assert(!m_stackFieldsDone.empty());
        const std::vector<bool>& fieldsDone = m_stackFieldsDone.back();

        const MetaStruct* stru = MetaDataGlobal::instance().getStruct(field);
        if (stru)
        {
            processDefaultValues(*stru, fieldsDone);
        }

        m_stackFieldsDone.pop_back();
    }

    m_visitor->exitStruct(field);
}




void ParserProcessDefaultValues::processDefaultValues(const MetaStruct& stru, const std::vector<bool>& fieldsDone)
{
    assert(!m_skipDefaultValues);
    for (size_t index = 0; index < fieldsDone.size(); ++index)
    {
        if (!fieldsDone[index])
        {
            const MetaField* field = stru.getFieldByIndex(index);
            if (field)
            {
                switch (field->typeId)
                {
                case MetaTypeId::TYPE_NONE:
                    break;
                case MetaTypeId::TYPE_BOOL:
                    m_visitor->enterBool(*field, false);
                    break;
                case MetaTypeId::TYPE_INT32:
                    m_visitor->enterInt32(*field, 0);
                    break;
                case MetaTypeId::TYPE_UINT32:
                    m_visitor->enterUInt32(*field, 0);
                    break;
                case MetaTypeId::TYPE_INT64:
                    m_visitor->enterInt64(*field, 0);
                    break;
                case MetaTypeId::TYPE_UINT64:
                    m_visitor->enterUInt64(*field, 0);
                    break;
                case MetaTypeId::TYPE_FLOAT:
                    m_visitor->enterFloat(*field, 0.0);
                    break;
                case MetaTypeId::TYPE_DOUBLE:
                    m_visitor->enterDouble(*field, 0.0);
                    break;
                case MetaTypeId::TYPE_STRING:
                    m_visitor->enterString(*field, "", 0);
                    break;
                case MetaTypeId::TYPE_BYTES:
                    {
                        static const BytesElement dummy[] = {};
                        m_visitor->enterBytes(*field, dummy, 0);
                    }
                    break;
                case MetaTypeId::TYPE_STRUCT:
                    {
                        m_visitor->enterStruct(*field);
                        const MetaStruct* substru = MetaDataGlobal::instance().getStruct(*field);
                        if (!substru)
                        {
                            std::vector<bool> subfieldsDone(substru->getFieldsSize(), false);
                            processDefaultValues(*substru, subfieldsDone);
                        }
                        m_visitor->exitStruct(*field);
                    }
                    break;
                case MetaTypeId::TYPE_ENUM:
                    m_visitor->enterEnum(*field, 0);
                    break;
                case MetaTypeId::TYPE_ARRAY_BOOL:
                    m_visitor->enterArrayBoolMove(*field, {});
                    break;
                case MetaTypeId::TYPE_ARRAY_INT32:
                    m_visitor->enterArrayInt32(*field, {});
                    break;
                case MetaTypeId::TYPE_ARRAY_UINT32:
                    m_visitor->enterArrayUInt32(*field, {});
                    break;
                case MetaTypeId::TYPE_ARRAY_INT64:
                    m_visitor->enterArrayInt64(*field, {});
                    break;
                case MetaTypeId::TYPE_ARRAY_UINT64:
                    m_visitor->enterArrayUInt64(*field, {});
                    break;
                case MetaTypeId::TYPE_ARRAY_FLOAT:
                    m_visitor->enterArrayFloat(*field, {});
                    break;
                case MetaTypeId::TYPE_ARRAY_DOUBLE:
                    m_visitor->enterArrayDouble(*field, {});
                    break;
                case MetaTypeId::TYPE_ARRAY_STRING:
                    m_visitor->enterArrayStringMove(*field, {});
                    break;
                case MetaTypeId::TYPE_ARRAY_BYTES:
                    m_visitor->enterArrayBytesMove(*field, {});
                    break;
                case MetaTypeId::TYPE_ARRAY_STRUCT:
                    m_visitor->enterArrayStruct(*field);
                    m_visitor->exitArrayStruct(*field);
                    break;
                case MetaTypeId::TYPE_ARRAY_ENUM:
                    m_visitor->enterArrayEnum(*field, std::vector<std::int32_t>());
                    break;
                case MetaTypeId::OFFSET_ARRAY_FLAG:
                    assert(false);
                    break;
                default:
                    assert(false);
                    break;
                }
            }
        }
    }
}


void ParserProcessDefaultValues::markAsDone(const MetaField& field)
{
    if (!m_skipDefaultValues)
    {
        assert(!m_stackFieldsDone.empty());
        std::vector<bool>& fieldsDone = m_stackFieldsDone.back();
        int index = field.index;
        if (index >= 0 && index < static_cast<int>(fieldsDone.size()))
        {
            fieldsDone[index] = true;
        }
    }
}


void ParserProcessDefaultValues::enterArrayStruct(const MetaField& field)
{
    markAsDone(field);
    m_visitor->enterArrayStruct(field);
}
void ParserProcessDefaultValues::exitArrayStruct(const MetaField& field)
{
    m_visitor->exitArrayStruct(field);
}

void ParserProcessDefaultValues::enterBool(const MetaField& field, bool value)
{
    markAsDone(field);
    if (value != false || !m_skipDefaultValues)
    {
        m_visitor->enterBool(field, value);
    }
}
void ParserProcessDefaultValues::enterInt32(const MetaField& field, std::int32_t value)
{
    markAsDone(field);
    if (value != 0 || !m_skipDefaultValues)
    {
        m_visitor->enterInt32(field, value);
    }
}
void ParserProcessDefaultValues::enterUInt32(const MetaField& field, std::uint32_t value)
{
    markAsDone(field);
    if (value != 0 || !m_skipDefaultValues)
    {
        m_visitor->enterUInt32(field, value);
    }
}
void ParserProcessDefaultValues::enterInt64(const MetaField& field, std::int64_t value)
{
    markAsDone(field);
    if (value != 0 || !m_skipDefaultValues)
    {
        m_visitor->enterInt64(field, value);
    }
}
void ParserProcessDefaultValues::enterUInt64(const MetaField& field, std::uint64_t value)
{
    markAsDone(field);
    if (value != 0 || !m_skipDefaultValues)
    {
        m_visitor->enterUInt64(field, value);
    }
}
void ParserProcessDefaultValues::enterFloat(const MetaField& field, float value)
{
    markAsDone(field);
    if (value != 0 || !m_skipDefaultValues)
    {
        m_visitor->enterFloat(field, value);
    }
}
void ParserProcessDefaultValues::enterDouble(const MetaField& field, double value)
{
    markAsDone(field);
    if (value != 0 || !m_skipDefaultValues)
    {
        m_visitor->enterDouble(field, value);
    }
}
void ParserProcessDefaultValues::enterString(const MetaField& field, std::string&& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterString(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterString(const MetaField& field, const char* value, int size)
{
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        m_visitor->enterString(field, value, size);
    }
}
void ParserProcessDefaultValues::enterBytes(const MetaField& field, Bytes&& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterBytes(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterBytes(const MetaField& field, const BytesElement* value, int size)
{
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        m_visitor->enterBytes(field, value, size);
    }
}
void ParserProcessDefaultValues::enterEnum(const MetaField& field, std::int32_t value)
{
    markAsDone(field);
    if (value != 0 || !m_skipDefaultValues)
    {
        m_visitor->enterEnum(field, value);
    }
}
void ParserProcessDefaultValues::enterEnum(const MetaField& field, std::string&& value)
{
    markAsDone(field);
    std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(field, value);
    if (v != 0 || !m_skipDefaultValues)
    {
        m_visitor->enterEnum(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterEnum(const MetaField& field, const char* value, int size)
{
    enterEnum(field, std::string(value, size));
}

void ParserProcessDefaultValues::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterArrayBoolMove(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterArrayBool(field, value);
    }
}
void ParserProcessDefaultValues::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterArrayInt32(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayInt32(const MetaField& field, const std::int32_t* value, int size)
{
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        m_visitor->enterArrayInt32(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterArrayUInt32(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, int size)
{
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        m_visitor->enterArrayUInt32(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterArrayInt64(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayInt64(const MetaField& field, const std::int64_t* value, int size)
{
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        m_visitor->enterArrayInt64(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterArrayUInt64(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, int size)
{
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        m_visitor->enterArrayUInt64(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterArrayFloat(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayFloat(const MetaField& field, const float* value, int size)
{
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        m_visitor->enterArrayFloat(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterArrayDouble(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayDouble(const MetaField& field, const double* value, int size)
{
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        m_visitor->enterArrayDouble(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterArrayStringMove(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterArrayString(field, value);
    }
}
void ParserProcessDefaultValues::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterArrayBytesMove(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterArrayBytes(field, value);
    }
}
void ParserProcessDefaultValues::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterArrayEnum(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayEnum(const MetaField& field, const std::int32_t* value, int size)
{
    markAsDone(field);
    if (size > 0 || !m_skipDefaultValues)
    {
        m_visitor->enterArrayEnum(field, value, size);
    }
}
void ParserProcessDefaultValues::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterArrayEnum(field, std::move(value));
    }
}
void ParserProcessDefaultValues::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
{
    markAsDone(field);
    if (!value.empty() || !m_skipDefaultValues)
    {
        m_visitor->enterArrayEnum(field, value);
    }
}


}   // namespace finalmq
