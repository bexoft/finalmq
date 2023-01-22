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


#include "finalmq/serialize/ParserConverter.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/logger/LogStream.h"
#include "finalmq/helpers/ModulenameFinalmq.h"

#include "finalmq/conversions/itoa.h"
#include "finalmq/conversions/dtoa.h"

#include <assert.h>
#include <iostream>
#include <algorithm>


namespace finalmq {


ParserConverter::ParserConverter(IParserVisitor* visitor)
    : m_visitor(visitor)
{
}

void ParserConverter::setVisitor(IParserVisitor &visitor)
{
    m_visitor = &visitor;
}



// ParserConverter
void ParserConverter::notifyError(const char* str, const char* message)
{
    assert(m_visitor);
    m_visitor->notifyError(str, message);
}

void ParserConverter::startStruct(const MetaStruct& stru)
{
    assert(m_visitor);
    m_visitor->startStruct(stru);
}

void ParserConverter::finished()
{
    assert(m_visitor);
    m_visitor->finished();
}

void ParserConverter::enterStruct(const MetaField& field)
{
    assert(m_visitor);
    if (field.typeId == MetaTypeId::TYPE_STRUCT)
    {
        m_visitor->enterStruct(field);
    }
}
void ParserConverter::exitStruct(const MetaField& field)
{
    assert(m_visitor);
    if (field.typeId == MetaTypeId::TYPE_STRUCT)
    {
        m_visitor->exitStruct(field);
    }
}

void ParserConverter::enterStructNull(const MetaField& field)
{
    assert(m_visitor);
    if (field.typeId == MetaTypeId::TYPE_STRUCT)
    {
        m_visitor->enterStructNull(field);
    }
}

void ParserConverter::enterArrayStruct(const MetaField& field)
{
    assert(m_visitor);
    if (field.typeId == MetaTypeId::TYPE_ARRAY_STRUCT)
    {
        m_visitor->enterArrayStruct(field);
    }
}
void ParserConverter::exitArrayStruct(const MetaField& field)
{
    assert(m_visitor);
    if (field.typeId == MetaTypeId::TYPE_ARRAY_STRUCT)
    {
        m_visitor->exitArrayStruct(field);
    }
}

void ParserConverter::enterBool(const MetaField& field, bool value)
{
    if (field.typeId == MetaTypeId::TYPE_BOOL)
    {
        m_visitor->enterBool(field, value);
    }
    else
    {
        convertNumber(field, value);
    }
}
void ParserConverter::enterInt32(const MetaField& field, std::int32_t value)
{
    if (field.typeId == MetaTypeId::TYPE_INT32)
    {
        m_visitor->enterInt32(field, value);
    }
    else
    {
        convertNumber(field, value);
    }
}
void ParserConverter::enterUInt32(const MetaField& field, std::uint32_t value)
{
    if (field.typeId == MetaTypeId::TYPE_UINT32)
    {
        m_visitor->enterUInt32(field, value);
    }
    else
    {
        convertNumber(field, value);
    }
}
void ParserConverter::enterInt64(const MetaField& field, std::int64_t value)
{
    if (field.typeId == MetaTypeId::TYPE_INT64)
    {
        m_visitor->enterInt64(field, value);
    }
    else
    {
        convertNumber(field, value);
    }
}
void ParserConverter::enterUInt64(const MetaField& field, std::uint64_t value)
{
    if (field.typeId == MetaTypeId::TYPE_UINT64)
    {
        m_visitor->enterUInt64(field, value);
    }
    else
    {
        convertNumber(field, value);
    }
}
void ParserConverter::enterFloat(const MetaField& field, float value)
{
    if (field.typeId == MetaTypeId::TYPE_FLOAT)
    {
        m_visitor->enterFloat(field, value);
    }
    else
    {
        convertNumber(field, value);
    }
}
void ParserConverter::enterDouble(const MetaField& field, double value)
{
    if (field.typeId == MetaTypeId::TYPE_DOUBLE)
    {
        m_visitor->enterDouble(field, value);
    }
    else
    {
        convertNumber(field, value);
    }
}
void ParserConverter::enterString(const MetaField& field, std::string&& value)
{
    if (field.typeId == MetaTypeId::TYPE_STRING)
    {
        m_visitor->enterString(field, std::move(value));
    }
    else
    {
        convertString(field, value.data(), value.size());
    }
}
void ParserConverter::enterString(const MetaField& field, const char* value, ssize_t size)
{
    if (field.typeId == MetaTypeId::TYPE_STRING)
    {
        m_visitor->enterString(field, value, size);
    }
    else
    {
        convertString(field, value, size);
    }
}
void ParserConverter::enterBytes(const MetaField& field, Bytes&& value)
{
    if (field.typeId == MetaTypeId::TYPE_BYTES)
    {
        m_visitor->enterBytes(field, std::move(value));
    }
    else
    {
        convertString(field, value.data(), value.size());
    }
}
void ParserConverter::enterBytes(const MetaField& field, const BytesElement* value, ssize_t size)
{
    if (field.typeId == MetaTypeId::TYPE_BYTES)
    {
        m_visitor->enterBytes(field, value, size);
    }
    else
    {
        convertString(field, value, size);
    }
}
void ParserConverter::enterEnum(const MetaField& field, std::int32_t value)
{
    if (field.typeId == MetaTypeId::TYPE_ENUM)
    {
        m_visitor->enterEnum(field, value);
    }
    else
    {
        convertNumber(field, value);
    }
}
void ParserConverter::enterEnum(const MetaField& field, std::string&& value)
{
    if (field.typeId == MetaTypeId::TYPE_ENUM)
    {
        m_visitor->enterEnum(field, std::move(value));
    }
    else
    {
        convertString(field, value.data(), value.size());
    }
}
void ParserConverter::enterEnum(const MetaField& field, const char* value, ssize_t size)
{
    if (field.typeId == MetaTypeId::TYPE_ENUM)
    {
        m_visitor->enterEnum(field, std::string(value, size));
    }
    else
    {
        convertString(field, value, size);
    }
}

void ParserConverter::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_BOOL)
    {
        m_visitor->enterArrayBoolMove(field, std::move(value));
    }
    else
    {
        convertArraytNumber(field, value);
    }
}
void ParserConverter::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_BOOL)
    {
        m_visitor->enterArrayBool(field, value);
    }
    else
    {
        convertArraytNumber(field, value);
    }
}
void ParserConverter::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_INT32)
    {
        m_visitor->enterArrayInt32(field, std::move(value));
    }
    else
    {
        convertArraytNumber(field, value);
    }
}
void ParserConverter::enterArrayInt32(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_INT32)
    {
        m_visitor->enterArrayInt32(field, value, size);
    }
    else
    {
        convertArraytNumber(field, value, size);
    }
}
void ParserConverter::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_UINT32)
    {
        m_visitor->enterArrayUInt32(field, std::move(value));
    }
    else
    {
        convertArraytNumber(field, value);
    }
}
void ParserConverter::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, ssize_t size)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_UINT32)
    {
        m_visitor->enterArrayUInt32(field, value, size);
    }
    else
    {
        convertArraytNumber(field, value, size);
    }
}
void ParserConverter::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_INT64)
    {
        m_visitor->enterArrayInt64(field, std::move(value));
    }
    else
    {
        convertArraytNumber(field, value);
    }
}
void ParserConverter::enterArrayInt64(const MetaField& field, const std::int64_t* value, ssize_t size)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_INT64)
    {
        m_visitor->enterArrayInt64(field, value, size);
    }
    else
    {
        convertArraytNumber(field, value, size);
    }
}
void ParserConverter::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_UINT64)
    {
        m_visitor->enterArrayUInt64(field, std::move(value));
    }
    else
    {
        convertArraytNumber(field, value);
    }
}
void ParserConverter::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, ssize_t size)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_UINT64)
    {
        m_visitor->enterArrayUInt64(field, value, size);
    }
    else
    {
        convertArraytNumber(field, value, size);
    }
}
void ParserConverter::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_FLOAT)
    {
        m_visitor->enterArrayFloat(field, std::move(value));
    }
    else
    {
        convertArraytNumber(field, value);
    }
}
void ParserConverter::enterArrayFloat(const MetaField& field, const float* value, ssize_t size)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_FLOAT)
    {
        m_visitor->enterArrayFloat(field, value, size);
    }
    else
    {
        convertArraytNumber(field, value, size);
    }
}
void ParserConverter::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_DOUBLE)
    {
        m_visitor->enterArrayDouble(field, std::move(value));
    }
    else
    {
        convertArraytNumber(field, value);
    }
}
void ParserConverter::enterArrayDouble(const MetaField& field, const double* value, ssize_t size)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_DOUBLE)
    {
        m_visitor->enterArrayDouble(field, value, size);
    }
    else
    {
        convertArraytNumber(field, value, size);
    }
}
void ParserConverter::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_STRING)
    {
        m_visitor->enterArrayStringMove(field, std::move(value));
    }
    else
    {
        convertArraytString(field, value);
    }
}
void ParserConverter::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_STRING)
    {
        m_visitor->enterArrayString(field, value);
    }
    else
    {
        convertArraytString(field, value);
    }
}
void ParserConverter::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_BYTES)
    {
        m_visitor->enterArrayBytesMove(field, std::move(value));
    }
    else
    {
        std::vector<std::string> valueArrayString;
        valueArrayString.reserve(value.size());
        for (size_t i = 0; i < value.size(); ++i)
        {
            valueArrayString.emplace_back(value[i].data(), value[i].size());
        }
        convertArraytString(field, valueArrayString);
    }
}
void ParserConverter::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_BYTES)
    {
        m_visitor->enterArrayBytes(field, value);
    }
    else
    {
        std::vector<std::string> valueArrayString;
        valueArrayString.reserve(value.size());
        for (size_t i = 0; i < value.size(); ++i)
        {
            valueArrayString.emplace_back(value[i].data(), value[i].size());
        }
        convertArraytString(field, valueArrayString);
    }
}
void ParserConverter::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_ENUM)
    {
        m_visitor->enterArrayEnum(field, std::move(value));
    }
    else
    {
        convertArraytNumber(field, value);
    }
}
void ParserConverter::enterArrayEnum(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_ENUM)
    {
        m_visitor->enterArrayEnum(field, value, size);
    }
    else
    {
        convertArraytNumber(field, value, size);
    }
}
void ParserConverter::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_ENUM)
    {
        m_visitor->enterArrayEnum(field, std::move(value));
    }
    else
    {
        convertArraytString(field, value);
    }
}

void ParserConverter::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
{
    if (field.typeId == MetaTypeId::TYPE_ARRAY_ENUM)
    {
        m_visitor->enterArrayEnum(field, value);
    }
    else
    {
        convertArraytString(field, value);
    }
}



///////////////////////////////


template<class T>
void ParserConverter::convertNumber(const MetaField& field, T value)
{
    switch (field.typeId)
    {
    case MetaTypeId::TYPE_BOOL:
        m_visitor->enterBool(field, value);
        break;
    case MetaTypeId::TYPE_INT32:
        m_visitor->enterInt32(field, static_cast<std::int32_t>(value));
        break;
    case MetaTypeId::TYPE_UINT32:
        m_visitor->enterUInt32(field, static_cast<std::uint32_t>(value));
        break;
    case MetaTypeId::TYPE_INT64:
        m_visitor->enterInt64(field, static_cast<std::int64_t>(value));
        break;
    case MetaTypeId::TYPE_UINT64:
        m_visitor->enterUInt64(field, static_cast<std::uint64_t>(value));
        break;
    case MetaTypeId::TYPE_FLOAT:
        m_visitor->enterFloat(field, static_cast<float>(value));
        break;
    case MetaTypeId::TYPE_DOUBLE:
        m_visitor->enterDouble(field, static_cast<double>(value));
        break;
    case MetaTypeId::TYPE_STRING:
        m_visitor->enterString(field, std::to_string(value));
        break;
    case MetaTypeId::TYPE_ENUM:
        m_visitor->enterEnum(field, static_cast<std::int32_t>(value));
        break;
    case MetaTypeId::TYPE_ARRAY_BOOL:
        {
            bool v = value;
            m_visitor->enterArrayBool(field, std::vector<bool>(&v, &v + 1));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_INT32:
        {
            std::int32_t v = static_cast<std::int32_t>(value);
            m_visitor->enterArrayInt32(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_UINT32:
        {
            std::uint32_t v = static_cast<std::uint32_t>(value);
            m_visitor->enterArrayUInt32(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_INT64:
        {
            std::int64_t v = static_cast<std::int64_t>(value);
            m_visitor->enterArrayInt64(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_UINT64:
        {
            std::uint64_t v = static_cast<std::uint64_t>(value);
            m_visitor->enterArrayUInt64(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_FLOAT:
        {
            float v = static_cast<float>(value);
            m_visitor->enterArrayFloat(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_DOUBLE:
        {
            double v = static_cast<double>(value);
            m_visitor->enterArrayDouble(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_STRING:
        {
            m_visitor->enterArrayString(field, {std::to_string(value)});
        }
        break;
    case MetaTypeId::TYPE_ARRAY_BYTES:
        {
            std::string str = std::to_string(value);
            Bytes bytes(str.begin(), str.end());
            m_visitor->enterArrayBytes(field, { bytes });
        }
        break;
    case MetaTypeId::TYPE_ARRAY_ENUM:
        {
            std::int32_t v = static_cast<std::int32_t>(value);
            m_visitor->enterArrayEnum(field, &v, 1);
        }
        break;
    default:
        streamError << "number not expected";
        break;
    }
}


void ParserConverter::convertString(const MetaField& field, const char* value, ssize_t size)
{
    switch (field.typeId)
    {
    case MetaTypeId::TYPE_BOOL:
        {
            bool v = (size == 4 && (memcmp(value, "true", 4) == 0));
            m_visitor->enterBool(field, v);
        }
        break;
    case MetaTypeId::TYPE_INT32:
        {
            std::int32_t v = strtol(value, nullptr, 10);
            m_visitor->enterInt32(field, v);
        }
        break;
    case MetaTypeId::TYPE_UINT32:
        {
            std::uint32_t v = strtoul(value, nullptr, 10);
            m_visitor->enterUInt32(field, v);
        }
        break;
    case MetaTypeId::TYPE_INT64:
        {
            std::int64_t v = strtoll(value, nullptr, 10);
            m_visitor->enterInt64(field, v);
        }
        break;
    case MetaTypeId::TYPE_UINT64:
        {
            std::uint64_t v = strtoull(value, nullptr, 10);
            m_visitor->enterUInt64(field, v);
        }
        break;
    case MetaTypeId::TYPE_FLOAT:
        {
            float v = strtof32(value, nullptr);
            m_visitor->enterFloat(field, v);
        }
        break;
    case MetaTypeId::TYPE_DOUBLE:
        {
            double v = strtof64(value, nullptr);
            m_visitor->enterDouble(field, v);
        }
        break;
    case MetaTypeId::TYPE_STRING:
        m_visitor->enterString(field, value, size);
        break;
    case MetaTypeId::TYPE_ENUM:
        m_visitor->enterEnum(field, value, size);
        break;
    case MetaTypeId::TYPE_ARRAY_BOOL:
        {
            bool v = (size == 4 && (memcmp(value, "true", 4) == 0));
            m_visitor->enterArrayBool(field, std::vector<bool>(&v, &v + 1));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_INT32:
        {
            std::int32_t v = strtol(value, nullptr, 10);
            m_visitor->enterArrayInt32(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_UINT32:
        {
            std::uint32_t v = strtoul(value, nullptr, 10);
            m_visitor->enterArrayUInt32(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_INT64:
        {
            std::int64_t v = strtoll(value, nullptr, 10);
            m_visitor->enterArrayInt64(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_UINT64:
        {
            std::uint64_t v = strtoull(value, nullptr, 10);
            m_visitor->enterArrayUInt64(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_FLOAT:
        {
            float v = strtof32(value, nullptr);
            m_visitor->enterArrayFloat(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_DOUBLE:
        {
            double v = strtof64(value, nullptr);
            m_visitor->enterArrayDouble(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_STRING:
        {
            m_visitor->enterArrayString(field, {std::string(value, size)});
        }
        break;
    case MetaTypeId::TYPE_ARRAY_ENUM:
        {
            m_visitor->enterArrayEnum(field, {std::string(value, size)});
        }
        break;
    default:
        streamError << "number not expected";
        break;
    }
}


template<class T>
void ParserConverter::convertArraytNumber(const MetaField& field, const T* value, ssize_t size)
{
    switch (field.typeId)
    {
    case MetaTypeId::TYPE_BOOL:
        if (value && size > 0)
        {
            m_visitor->enterBool(field, value[0]);
        }
        break;
    case MetaTypeId::TYPE_INT32:
        if (value && size > 0)
        {
            m_visitor->enterInt32(field, static_cast<std::int32_t>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_UINT32:
        if (value && size > 0)
        {
            m_visitor->enterUInt32(field, static_cast<std::uint32_t>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_INT64:
        if (value && size > 0)
        {
            m_visitor->enterInt64(field, static_cast<std::int64_t>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_UINT64:
        if (value && size > 0)
        {
            m_visitor->enterUInt64(field, static_cast<std::uint64_t>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_FLOAT:
        if (value && size > 0)
        {
            m_visitor->enterFloat(field, static_cast<float>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_DOUBLE:
        if (value && size > 0)
        {
            m_visitor->enterDouble(field, static_cast<double>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_STRING:
        if (value && size > 0)
        {
            m_visitor->enterString(field, std::to_string(value[0]));
        }
        break;
    case MetaTypeId::TYPE_ENUM:
        if (value && size > 0)
        {
            m_visitor->enterEnum(field, static_cast<std::int32_t>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_BOOL:
        {
            std::vector<bool> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v] (const T& entry) {
                v.push_back(entry);
            });
            m_visitor->enterArrayBool(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_INT32:
        {
            std::vector<std::int32_t> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v] (const T& entry) {
                v.push_back(static_cast<std::int32_t>(entry));
            });
            m_visitor->enterArrayInt32(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_UINT32:
        {
            std::vector<std::uint32_t> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v] (const T& entry) {
                v.push_back(static_cast<std::uint32_t>(entry));
            });
            m_visitor->enterArrayUInt32(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_INT64:
        {
            std::vector<std::int64_t> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v] (const T& entry) {
                v.push_back(static_cast<std::int64_t>(entry));
            });
            m_visitor->enterArrayInt64(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_UINT64:
        {
            std::vector<std::uint64_t> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v] (const T& entry) {
                v.push_back(static_cast<std::uint64_t>(entry));
            });
            m_visitor->enterArrayUInt64(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_FLOAT:
        {
            std::vector<float> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v] (const T& entry) {
                v.push_back(static_cast<float>(entry));
            });
            m_visitor->enterArrayFloat(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_DOUBLE:
        {
            std::vector<double> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v] (const T& entry) {
                v.push_back(static_cast<double>(entry));
            });
            m_visitor->enterArrayDouble(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_STRING:
        {
            std::vector<std::string> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v] (const T& entry) {
                v.push_back(std::to_string(entry));
            });
            m_visitor->enterArrayString(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_ENUM:
        {
            std::vector<std::int32_t> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v] (const T& entry) {
                v.push_back(static_cast<std::int32_t>(entry));
            });
            m_visitor->enterArrayEnum(field, std::move(v));
        }
        break;
    default:
        streamError << "array not expected";
        break;
    }
}

template<class T>
void ParserConverter::convertArraytNumber(const MetaField& field, const std::vector<T>& value)
{
    ssize_t size = value.size();
    switch (field.typeId)
    {
    case MetaTypeId::TYPE_BOOL:
        if (value.size() > 0)
        {
            m_visitor->enterBool(field, value[0]);
        }
        break;
    case MetaTypeId::TYPE_INT32:
        if (value.size() > 0)
        {
            m_visitor->enterInt32(field, static_cast<std::int32_t>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_UINT32:
        if (value.size() > 0)
        {
            m_visitor->enterUInt32(field, static_cast<std::uint32_t>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_INT64:
        if (value.size() > 0)
        {
            m_visitor->enterInt64(field, static_cast<std::int64_t>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_UINT64:
        if (value.size() > 0)
        {
            m_visitor->enterUInt64(field, static_cast<std::uint64_t>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_FLOAT:
        if (value.size() > 0)
        {
            m_visitor->enterFloat(field, static_cast<float>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_DOUBLE:
        if (value.size() > 0)
        {
            m_visitor->enterDouble(field, static_cast<double>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_STRING:
        if (value.size() > 0)
        {
            m_visitor->enterString(field, std::to_string(value[0]));
        }
        break;
    case MetaTypeId::TYPE_ENUM:
        if (value.size() > 0)
        {
            m_visitor->enterEnum(field, static_cast<std::int32_t>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_BOOL:
        {
            std::vector<bool> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const T& entry) {
                v.push_back(entry);
            });
            m_visitor->enterArrayBool(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_INT32:
        {
            std::vector<std::int32_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const T& entry) {
                v.push_back(static_cast<std::int32_t>(entry));
            });
            m_visitor->enterArrayInt32(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_UINT32:
        {
            std::vector<std::uint32_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const T& entry) {
                v.push_back(static_cast<std::uint32_t>(entry));
            });
            m_visitor->enterArrayUInt32(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_INT64:
        {
            std::vector<std::int64_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const T& entry) {
                v.push_back(static_cast<std::int64_t>(entry));
            });
            m_visitor->enterArrayInt64(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_UINT64:
        {
            std::vector<std::uint64_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const T& entry) {
                v.push_back(static_cast<std::uint64_t>(entry));
            });
            m_visitor->enterArrayUInt64(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_FLOAT:
        {
            std::vector<float> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const T& entry) {
                v.push_back(static_cast<float>(entry));
            });
            m_visitor->enterArrayFloat(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_DOUBLE:
        {
            std::vector<double> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const T& entry) {
                v.push_back(static_cast<double>(entry));
            });
            m_visitor->enterArrayDouble(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_STRING:
        {
            std::vector<std::string> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const T& entry) {
                v.push_back(std::to_string(entry));
            });
            m_visitor->enterArrayString(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_ENUM:
        {
            std::vector<std::int32_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const T& entry) {
                v.push_back(static_cast<std::int32_t>(entry));
            });
            m_visitor->enterArrayEnum(field, std::move(v));
        }
        break;
    default:
        streamError << "array not expected";
        break;
    }
}

void ParserConverter::convertArraytString(const MetaField& field, const std::vector<std::string>& value)
{
    size_t size = value.size();
    switch (field.typeId)
    {
    case MetaTypeId::TYPE_BOOL:
        if (!value.empty())
        {
            bool v = (value[0].size() == 4 && (memcmp(value[0].c_str(), "true", 4) == 0));
            m_visitor->enterBool(field, v);
        }
        break;
    case MetaTypeId::TYPE_INT32:
        if (!value.empty())
        {
            std::int32_t v = strtol(value[0].c_str(), nullptr, 10);
            m_visitor->enterInt32(field, v);
        }
        break;
    case MetaTypeId::TYPE_UINT32:
        if (!value.empty())
        {
            std::uint32_t v = strtoul(value[0].c_str(), nullptr, 10);
            m_visitor->enterUInt32(field, v);
        }
        break;
    case MetaTypeId::TYPE_INT64:
        if (!value.empty())
        {
            std::int64_t v = strtoll(value[0].c_str(), nullptr, 10);
            m_visitor->enterInt64(field, v);
        }
        break;
    case MetaTypeId::TYPE_UINT64:
        if (!value.empty())
        {
            std::uint64_t v = strtoull(value[0].c_str(), nullptr, 10);
            m_visitor->enterUInt64(field, v);
        }
        break;
    case MetaTypeId::TYPE_FLOAT:
        if (!value.empty())
        {
            float v = strtof32(value[0].c_str(), nullptr);
            m_visitor->enterFloat(field, v);
        }
        break;
    case MetaTypeId::TYPE_DOUBLE:
        if (!value.empty())
        {
            double v = strtof64(value[0].c_str(), nullptr);
            m_visitor->enterDouble(field, v);
        }
        break;
    case MetaTypeId::TYPE_STRING:
        if (!value.empty())
        {
            m_visitor->enterString(field, value[0].c_str(), value[0].size());
        }
        break;
    case MetaTypeId::TYPE_ENUM:
        if (!value.empty())
        {
            m_visitor->enterEnum(field, value[0].c_str(), value[0].size());
        }
        break;
    case MetaTypeId::TYPE_ARRAY_BOOL:
        {
            std::vector<bool> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const std::string& entry) {
                bool val = (entry.size() == 4 && (memcmp(entry.c_str(), "true", 4) == 0));
                v.push_back(val);
            });
            m_visitor->enterArrayBool(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_INT32:
        {
            std::vector<std::int32_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const std::string& entry) {
                v.push_back(strtol(entry.c_str(), nullptr, 10));
            });
            m_visitor->enterArrayInt32(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_UINT32:
        {
            std::vector<std::uint32_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const std::string& entry) {
                v.push_back(strtoul(entry.c_str(), nullptr, 10));
            });
            m_visitor->enterArrayUInt32(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_INT64:
        {
            std::vector<std::int64_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const std::string& entry) {
                v.push_back(strtoll(entry.c_str(), nullptr, 10));
            });
            m_visitor->enterArrayInt64(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_UINT64:
        {
            std::vector<std::uint64_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const std::string& entry) {
                v.push_back(strtoull(entry.c_str(), nullptr, 10));
            });
            m_visitor->enterArrayUInt64(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_FLOAT:
        {
            std::vector<float> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const std::string& entry) {
                v.push_back(strtof32(entry.c_str(), nullptr));
            });
            m_visitor->enterArrayFloat(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_DOUBLE:
        {
            std::vector<double> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const std::string& entry) {
                v.push_back(strtof64(entry.c_str(), nullptr));
            });
            m_visitor->enterArrayDouble(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_STRING:
        m_visitor->enterArrayString(field, value);
        break;
    case MetaTypeId::TYPE_ARRAY_ENUM:
        {
            std::vector<std::string> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const std::string& entry) {
                v.push_back(entry);
            });
            m_visitor->enterArrayEnum(field, std::move(v));
        }
        break;
    default:
        streamError << "array not expected";
        break;
    }
}





}   //namespace finalmq
