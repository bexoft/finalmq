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

#include "finalmq/serializejson/ParserJson.h"

#include <assert.h>

#include "finalmq/conversions/dtoa.h"
#include "finalmq/conversions/itoa.h"
#include "finalmq/helpers/ModulenameFinalmq.h"
#include "finalmq/helpers/base64.h"
#include "finalmq/logger/LogStream.h"
#include "finalmq/metadata/MetaData.h"
//#include <memory.h>
#include <cmath>
#include <iostream>

namespace finalmq
{
ParserJson::ParserJson(IParserVisitor& visitor, const char* ptr, ssize_t size)
    : m_ptr(ptr), m_size(size), m_visitor(visitor), m_parser(*this)
{
}

const char* ParserJson::parseStruct(const std::string& typeName)
{
    assert(m_ptr);
    assert(m_size >= CHECK_ON_ZEROTERM);

    const MetaStruct* stru = MetaDataGlobal::instance().getStruct(typeName);
    if (!stru)
    {
        m_visitor.notifyError(m_ptr, "typename not found");
        return nullptr;
    }

    MetaField field(MetaTypeId::TYPE_STRUCT, typeName, {}, {});
    m_fieldCurrent = &field;

    m_visitor.startStruct(*stru);
    const char* str = m_parser.parse(m_ptr, m_size);
    m_visitor.finished();

    return str;
}

void ParserJson::syntaxError(const char* /*str*/, const char* /*message*/)
{
}

void ParserJson::enterNull()
{
    if (m_jsonTypeActive > 0)
    {
        if (m_jsonTypeActive == 1)
        {
            m_jsonTypeActive = 0;
            endOfJson();
        }
        return;
    }
    if (!m_fieldCurrent)
    {
        // unknown key
        return;
    }
    if (m_fieldCurrent->typeId == MetaTypeId::TYPE_STRUCT && m_fieldCurrent->flags & METAFLAG_NULLABLE)
    {
        m_visitor.enterStructNull(*m_fieldCurrent);
    }
}

template<class T>
void ParserJson::enterNumber(T value)
{
    if (m_jsonTypeActive > 0)
    {
        if (m_jsonTypeActive == 1)
        {
            m_jsonTypeActive = 0;
            endOfJson();
        }
        return;
    }
    if (!m_fieldCurrent)
    {
        // unknown key
        return;
    }
    switch(m_fieldCurrent->typeId)
    {
        case MetaTypeId::TYPE_BOOL:
#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
            m_visitor.enterBool(*m_fieldCurrent, value);
#ifndef WIN32
#pragma GCC diagnostic pop
#endif
            break;
        case MetaTypeId::TYPE_INT8:
            m_visitor.enterInt8(*m_fieldCurrent, static_cast<std::int8_t>(value));
            break;
        case MetaTypeId::TYPE_UINT8:
            m_visitor.enterUInt8(*m_fieldCurrent, static_cast<std::uint8_t>(value));
            break;
        case MetaTypeId::TYPE_INT16:
            m_visitor.enterInt16(*m_fieldCurrent, static_cast<std::int16_t>(value));
            break;
        case MetaTypeId::TYPE_UINT16:
            m_visitor.enterUInt16(*m_fieldCurrent, static_cast<std::uint16_t>(value));
            break;
        case MetaTypeId::TYPE_INT32:
            m_visitor.enterInt32(*m_fieldCurrent, static_cast<std::int32_t>(value));
            break;
        case MetaTypeId::TYPE_UINT32:
            m_visitor.enterUInt32(*m_fieldCurrent, static_cast<std::uint32_t>(value));
            break;
        case MetaTypeId::TYPE_INT64:
            m_visitor.enterInt64(*m_fieldCurrent, static_cast<std::int64_t>(value));
            break;
        case MetaTypeId::TYPE_UINT64:
            m_visitor.enterUInt64(*m_fieldCurrent, static_cast<std::uint64_t>(value));
            break;
        case MetaTypeId::TYPE_FLOAT:
            m_visitor.enterFloat(*m_fieldCurrent, static_cast<float>(value));
            break;
        case MetaTypeId::TYPE_DOUBLE:
            m_visitor.enterDouble(*m_fieldCurrent, static_cast<double>(value));
            break;
        case MetaTypeId::TYPE_STRING:
            m_visitor.enterString(*m_fieldCurrent, std::to_string(value));
            break;
        case MetaTypeId::TYPE_ENUM:
            m_visitor.enterEnum(*m_fieldCurrent, static_cast<std::int32_t>(value));
            break;
        case MetaTypeId::TYPE_ARRAY_BOOL:
#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
            m_arrayBool.push_back(value);
#ifndef WIN32
#pragma GCC diagnostic pop
#endif
            break;
        case MetaTypeId::TYPE_ARRAY_INT8:
            m_arrayInt8.push_back(static_cast<std::int8_t>(value));
            break;
        case MetaTypeId::TYPE_ARRAY_INT16:
            m_arrayInt16.push_back(static_cast<std::int16_t>(value));
            break;
        case MetaTypeId::TYPE_ARRAY_UINT16:
            m_arrayUInt16.push_back(static_cast<std::uint16_t>(value));
            break;
        case MetaTypeId::TYPE_ARRAY_INT32:
            m_arrayInt32.push_back(static_cast<std::int32_t>(value));
            break;
        case MetaTypeId::TYPE_ARRAY_UINT32:
            m_arrayUInt32.push_back(static_cast<std::uint32_t>(value));
            break;
        case MetaTypeId::TYPE_ARRAY_INT64:
            m_arrayInt64.push_back(static_cast<std::int64_t>(value));
            break;
        case MetaTypeId::TYPE_ARRAY_UINT64:
            m_arrayUInt64.push_back(static_cast<std::uint64_t>(value));
            break;
        case MetaTypeId::TYPE_ARRAY_FLOAT:
            m_arrayFloat.push_back(static_cast<float>(value));
            break;
        case MetaTypeId::TYPE_ARRAY_DOUBLE:
            m_arrayDouble.push_back(static_cast<double>(value));
            break;
        case MetaTypeId::TYPE_ARRAY_STRING:
            m_arrayString.push_back(std::to_string(value));
            break;
        case MetaTypeId::TYPE_ARRAY_BYTES:
            m_arrayString.push_back(std::to_string(value));
            break;
        case MetaTypeId::TYPE_ARRAY_ENUM:
            if (m_arrayString.empty() || !m_arrayInt32.empty())
            {
                m_arrayInt32.push_back(static_cast<std::int32_t>(value));
            }
            else
            {
                const std::string& v = MetaDataGlobal::instance().getEnumAliasByValue(*m_fieldCurrent, static_cast<std::int32_t>(value));
                m_arrayString.push_back(v);
            }
            break;
        default:
            streamError << "number not expected";
            break;
    }
}

void ParserJson::enterBool(bool value)
{
    enterNumber(value);
}

void ParserJson::enterInt32(std::int32_t value)
{
    enterNumber(value);
}

void ParserJson::enterUInt32(std::uint32_t value)
{
    enterNumber(value);
}

void ParserJson::enterInt64(std::int64_t value)
{
    enterNumber(value);
}

void ParserJson::enterUInt64(std::uint64_t value)
{
    enterNumber(value);
}

void ParserJson::enterDouble(double value)
{
    enterNumber(value);
}

template<>
float ParserJson::convert<float>(const char* value, ssize_t size)
{
    float v = 0;
    if (size == 3 && memcmp(value, "NaN", 3) == 0)
    {
        v = NAN;
    }
    else if (size == 8 && memcmp(value, "Infinity", 8) == 0)
    {
        v = std::numeric_limits<float>::infinity();
        ;
    }
    else if (size == 9 && memcmp(value, "-Infinity", 9) == 0)
    {
        v = -std::numeric_limits<float>::infinity();
        ;
    }
    else
    {
        v = strtof32(value, nullptr);
    }
    return v;
}

template<>
double ParserJson::convert<double>(const char* value, ssize_t size)
{
    double v = 0;
    if (size == 3 && memcmp(value, "NaN", 3) == 0)
    {
        v = NAN;
    }
    else if (size == 8 && memcmp(value, "Infinity", 8) == 0)
    {
        v = std::numeric_limits<double>::infinity();
        ;
    }
    else if (size == 9 && memcmp(value, "-Infinity", 9) == 0)
    {
        v = -std::numeric_limits<double>::infinity();
        ;
    }
    else
    {
        v = strtof64(value, nullptr);
    }
    return v;
}

void ParserJson::enterString(const char* value, ssize_t size)
{
    if (m_jsonTypeActive > 0)
    {
        if (m_jsonTypeActive == 1)
        {
            m_jsonTypeActive = 0;
            endOfJson();
        }
        return;
    }
    if (!m_fieldCurrent)
    {
        // unknown key
        return;
    }
    switch(m_fieldCurrent->typeId)
    {
        case MetaTypeId::TYPE_BOOL:
        {
            bool v = (size == 4 && (memcmp(value, "true", 4) == 0));
            m_visitor.enterBool(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_INT8:
        {
            std::int8_t v = static_cast<std::int8_t>(strtol(value, nullptr, 10));
            m_visitor.enterInt8(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_UINT8:
        {
            std::uint8_t v = static_cast<std::uint8_t>(strtoul(value, nullptr, 10));
            m_visitor.enterUInt8(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_INT16:
        {
            std::int16_t v = static_cast<std::int16_t>(strtol(value, nullptr, 10));
            m_visitor.enterInt16(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_UINT16:
        {
            std::uint16_t v = static_cast<std::uint16_t>(strtoul(value, nullptr, 10));
            m_visitor.enterUInt16(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_INT32:
        {
            std::int32_t v = static_cast<std::int32_t>(strtol(value, nullptr, 10));
            m_visitor.enterInt32(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_UINT32:
        {
            std::uint32_t v = static_cast<std::uint32_t>(strtoul(value, nullptr, 10));
            m_visitor.enterUInt32(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_INT64:
        {
            std::int64_t v = strtoll(value, nullptr, 10);
            m_visitor.enterInt64(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_UINT64:
        {
            std::uint64_t v = strtoull(value, nullptr, 10);
            m_visitor.enterUInt64(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_FLOAT:
        {
            float v = convert<float>(value, size);
            m_visitor.enterFloat(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_DOUBLE:
        {
            double v = convert<double>(value, size);
            m_visitor.enterDouble(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_STRING:
            m_visitor.enterString(*m_fieldCurrent, value, size);
            break;
        case MetaTypeId::TYPE_BYTES:
        {
            // convert from base64
            std::vector<char> bin;
            Base64::decode(value, size, bin);
            m_visitor.enterBytes(*m_fieldCurrent, std::move(bin));
        }
        break;
        case MetaTypeId::TYPE_ENUM:
            m_visitor.enterEnum(*m_fieldCurrent, value, size);
            break;
        case MetaTypeId::TYPE_ARRAY_BOOL:
        {
            bool v = (size == 4 && (memcmp(value, "true", 4) == 0));
            m_arrayBool.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT8:
        {
            std::int8_t v = static_cast<std::int8_t>(strtol(value, nullptr, 10));
            m_arrayInt8.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT16:
        {
            std::int16_t v = static_cast<std::int16_t>(strtol(value, nullptr, 10));
            m_arrayInt16.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_UINT16:
        {
            std::uint16_t v = static_cast<std::uint16_t>(strtoul(value, nullptr, 10));
            m_arrayUInt16.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT32:
        {
            std::int32_t v = static_cast<std::int32_t>(strtol(value, nullptr, 10));
            m_arrayInt32.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_UINT32:
        {
            std::uint32_t v = static_cast<std::uint32_t>(strtoul(value, nullptr, 10));
            m_arrayUInt32.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT64:
        {
            std::int64_t v = strtoll(value, nullptr, 10);
            m_arrayInt64.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_UINT64:
        {
            std::uint64_t v = strtoull(value, nullptr, 10);
            m_arrayUInt64.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_FLOAT:
        {
            float v = convert<float>(value, size);
            m_arrayFloat.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_DOUBLE:
        {
            double v = convert<double>(value, size);
            m_arrayDouble.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_STRING:
            m_arrayString.emplace_back(value, size);
            break;
        case MetaTypeId::TYPE_ARRAY_BYTES:
        {
            // convert from base64
            std::vector<char> bin;
            Base64::decode(value, size, bin);
            m_arrayBytes.emplace_back(std::move(bin));
        }
        break;
        case MetaTypeId::TYPE_ARRAY_ENUM:
            if (!m_arrayString.empty() || m_arrayInt32.empty())
            {
                m_arrayString.emplace_back(value, size);
            }
            else
            {
                std::int32_t valueInt = MetaDataGlobal::instance().getEnumValueByName(*m_fieldCurrent, std::string(value, size));
                m_arrayInt32.push_back(valueInt);
            }
            break;
        default:
            streamError << "string not expected";
            break;
    }
}

void ParserJson::enterString(std::string&& value)
{
    if (m_jsonTypeActive > 0)
    {
        if (m_jsonTypeActive == 1)
        {
            m_jsonTypeActive = 0;
            endOfJson();
        }
        return;
    }

    if (!m_fieldCurrent)
    {
        // unknown key
        return;
    }
    switch(m_fieldCurrent->typeId)
    {
        case MetaTypeId::TYPE_BOOL:
        {
            bool v = (value.size() == 4 && (memcmp(value.c_str(), "true", 4) == 0));
            m_visitor.enterBool(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_INT8:
        {
            std::int8_t v = static_cast<std::int8_t>(strtol(value.c_str(), nullptr, 10));
            m_visitor.enterInt8(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_UINT8:
        {
            std::uint8_t v = static_cast<std::uint8_t>(strtoul(value.c_str(), nullptr, 10));
            m_visitor.enterUInt8(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_INT16:
        {
            std::int16_t v = static_cast<std::int16_t>(strtol(value.c_str(), nullptr, 10));
            m_visitor.enterInt16(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_UINT16:
        {
            std::uint16_t v = static_cast<std::uint16_t>(strtoul(value.c_str(), nullptr, 10));
            m_visitor.enterUInt16(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_INT32:
        {
            std::int32_t v = static_cast<std::int32_t>(strtol(value.c_str(), nullptr, 10));
            m_visitor.enterInt32(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_UINT32:
        {
            std::uint32_t v = static_cast<std::uint32_t>(strtoul(value.c_str(), nullptr, 10));
            m_visitor.enterUInt32(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_INT64:
        {
            std::int64_t v = strtoll(value.c_str(), nullptr, 10);
            m_visitor.enterInt64(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_UINT64:
        {
            std::uint64_t v = strtoull(value.c_str(), nullptr, 10);
            m_visitor.enterUInt64(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_FLOAT:
        {
            float v = convert<float>(value.c_str(), value.size());
            m_visitor.enterFloat(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_DOUBLE:
        {
            double v = convert<double>(value.c_str(), value.size());
            m_visitor.enterDouble(*m_fieldCurrent, v);
        }
        break;
        case MetaTypeId::TYPE_STRING:
            m_visitor.enterString(*m_fieldCurrent, std::move(value));
            break;
        case MetaTypeId::TYPE_BYTES:
        {
            // convert from base64
            std::vector<char> bin;
            Base64::decode(value, bin);
            m_visitor.enterBytes(*m_fieldCurrent, std::move(bin));
        }
        break;
        case MetaTypeId::TYPE_ENUM:
            m_visitor.enterEnum(*m_fieldCurrent, std::move(value));
            break;
        case MetaTypeId::TYPE_ARRAY_BOOL:
        {
            bool v = (value.size() == 4 && (memcmp(value.c_str(), "true", 4) == 0));
            m_arrayBool.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT32:
        {
            std::int32_t v = static_cast<std::int32_t>(strtol(value.c_str(), nullptr, 10));
            m_arrayInt32.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_UINT32:
        {
            std::uint32_t v = static_cast<std::uint32_t>(strtoul(value.c_str(), nullptr, 10));
            m_arrayUInt32.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT64:
        {
            std::int64_t v = strtoll(value.c_str(), nullptr, 10);
            m_arrayInt64.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_UINT64:
        {
            std::uint64_t v = strtoull(value.c_str(), nullptr, 10);
            m_arrayUInt64.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_FLOAT:
        {
            float v = convert<float>(value.c_str(), value.size());
            m_arrayFloat.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_DOUBLE:
        {
            double v = convert<double>(value.c_str(), value.size());
            m_arrayDouble.push_back(v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_STRING:
            m_arrayString.push_back(std::move(value));
            break;
        case MetaTypeId::TYPE_ARRAY_BYTES:
        {
            // convert from base64
            std::vector<char> bin;
            Base64::decode(value, bin);
            m_arrayBytes.emplace_back(std::move(bin));
        }
        break;
        case MetaTypeId::TYPE_ARRAY_ENUM:
            if (!m_arrayString.empty() || m_arrayInt32.empty())
            {
                m_arrayString.push_back(std::move(value));
            }
            else
            {
                std::int32_t valueInt = MetaDataGlobal::instance().getEnumValueByName(*m_fieldCurrent, value);
                m_arrayInt32.push_back(valueInt);
            }
            break;
        default:
            streamError << "string not expected";
            break;
    }
}

void ParserJson::enterArray()
{
    if (m_jsonTypeActive > 0)
    {
        ++m_jsonTypeActive;
        return;
    }

    if (m_fieldCurrent && (static_cast<int>(m_fieldCurrent->typeId) & static_cast<int>(MetaTypeId::OFFSET_ARRAY_FLAG)))
    {
        switch(m_fieldCurrent->typeId)
        {
            case MetaTypeId::TYPE_ARRAY_BOOL:
                m_arrayBool.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_INT8:
                m_arrayInt8.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_INT16:
                m_arrayInt16.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_UINT16:
                m_arrayUInt16.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_INT32:
                m_arrayInt32.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_UINT32:
                m_arrayUInt32.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_INT64:
                m_arrayInt64.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_UINT64:
                m_arrayUInt64.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_FLOAT:
                m_arrayFloat.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_DOUBLE:
                m_arrayDouble.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_STRING:
                m_arrayString.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_BYTES:
                m_arrayString.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_ENUM:
                m_arrayInt32.clear();
                m_arrayString.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_STRUCT:
                m_visitor.enterArrayStruct(*m_fieldCurrent);
                m_stack.emplace_back(m_fieldCurrent);
                m_fieldCurrent = m_fieldCurrent->fieldWithoutArray;
                //            m_stack.emplace_back(m_fieldCurrent);
                m_structCurrent = nullptr;
                break;
            default:
                assert(false);
                break;
        }
    }
}

void ParserJson::exitArray()
{
    if (m_jsonTypeActive > 0)
    {
        --m_jsonTypeActive;
        if (m_jsonTypeActive == 1)
        {
            m_jsonTypeActive = 0;
            endOfJson();
        }
        return;
    }

    if (!m_fieldCurrent)
    {
        // unknown key
        return;
    }

    if (static_cast<int>(m_fieldCurrent->typeId) & static_cast<int>(MetaTypeId::OFFSET_ARRAY_FLAG))
    {
        switch(m_fieldCurrent->typeId)
        {
            case MetaTypeId::TYPE_ARRAY_BOOL:
                m_visitor.enterArrayBoolMove(*m_fieldCurrent, std::move(m_arrayBool));
                m_arrayBool.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_INT8:
                m_visitor.enterArrayInt8(*m_fieldCurrent, std::move(m_arrayInt8));
                m_arrayInt8.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_INT16:
                m_visitor.enterArrayInt16(*m_fieldCurrent, std::move(m_arrayInt16));
                m_arrayInt16.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_UINT16:
                m_visitor.enterArrayUInt16(*m_fieldCurrent, std::move(m_arrayUInt16));
                m_arrayUInt16.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_INT32:
                m_visitor.enterArrayInt32(*m_fieldCurrent, std::move(m_arrayInt32));
                m_arrayInt32.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_UINT32:
                m_visitor.enterArrayUInt32(*m_fieldCurrent, std::move(m_arrayUInt32));
                m_arrayUInt32.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_INT64:
                m_visitor.enterArrayInt64(*m_fieldCurrent, std::move(m_arrayInt64));
                m_arrayInt64.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_UINT64:
                m_visitor.enterArrayUInt64(*m_fieldCurrent, std::move(m_arrayUInt64));
                m_arrayUInt64.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_FLOAT:
                m_visitor.enterArrayFloat(*m_fieldCurrent, std::move(m_arrayFloat));
                m_arrayFloat.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_DOUBLE:
                m_visitor.enterArrayDouble(*m_fieldCurrent, std::move(m_arrayDouble));
                m_arrayDouble.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_STRING:
                m_visitor.enterArrayStringMove(*m_fieldCurrent, std::move(m_arrayString));
                m_arrayString.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_BYTES:
                m_visitor.enterArrayBytesMove(*m_fieldCurrent, std::move(m_arrayBytes));
                m_arrayBytes.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_ENUM:
                if (!m_arrayString.empty())
                {
                    m_visitor.enterArrayEnum(*m_fieldCurrent, std::move(m_arrayString));
                }
                else
                {
                    m_visitor.enterArrayEnum(*m_fieldCurrent, std::move(m_arrayInt32));
                }
                m_arrayInt32.clear();
                m_arrayString.clear();
                break;
            case MetaTypeId::TYPE_ARRAY_STRUCT:
                assert(false);
                break;
            default:
                assert(false);
                break;
        }
    }
    else if (m_fieldCurrent->typeId == MetaTypeId::TYPE_STRUCT)
    {
        m_structCurrent = nullptr;
        m_fieldCurrent = nullptr;
        if (!m_stack.empty())
        {
            //            m_stack.pop_back();
            m_fieldCurrent = m_stack.back();
            if (m_fieldCurrent)
            {
                m_visitor.exitArrayStruct(*m_fieldCurrent);
            }
            m_stack.pop_back();
        }

        if (!m_stack.empty())
        {
            m_fieldCurrent = m_stack.back();
            if (m_fieldCurrent)
            {
                m_structCurrent = MetaDataGlobal::instance().getStruct(*m_fieldCurrent);
            }
        }
    }
}

void ParserJson::enterObject()
{
    if (m_jsonTypeActive > 0)
    {
        ++m_jsonTypeActive;
        return;
    }

    m_stack.emplace_back(m_fieldCurrent);
    m_structCurrent = nullptr;
    if (m_fieldCurrent && m_fieldCurrent->typeId == MetaTypeId::TYPE_STRUCT)
    {
        const MetaStruct* stru = MetaDataGlobal::instance().getStruct(*m_fieldCurrent);
        if (stru)
        {
            m_structCurrent = stru;
            // the outer object shall not trigger enterStruct
            if (m_stack.size() > 1)
            {
                m_visitor.enterStruct(*m_fieldCurrent);
            }
            m_fieldCurrent = nullptr;
        }
        else
        {
            m_fieldCurrent = nullptr;
        }
    }
    else
    {
        m_fieldCurrent = nullptr;
    }
}

void ParserJson::exitObject()
{
    if (m_jsonTypeActive > 0)
    {
        --m_jsonTypeActive;
        if (m_jsonTypeActive == 1)
        {
            m_jsonTypeActive = 0;
            endOfJson();
        }
        return;
    }

    m_structCurrent = nullptr;
    m_fieldCurrent = nullptr;
    if (!m_stack.empty())
    {
        m_fieldCurrent = m_stack.back();
        // the outer object shall not trigger exitStruct
        if (m_fieldCurrent && (m_stack.size() > 1))
        {
            m_visitor.exitStruct(*m_fieldCurrent);
        }
        m_stack.pop_back();
    }
    if (!m_stack.empty())
    {
        m_fieldCurrent = m_stack.back();
        if (m_fieldCurrent)
        {
            if (static_cast<int>(m_fieldCurrent->typeId) & static_cast<int>(MetaTypeId::OFFSET_ARRAY_FLAG))
            {
                m_fieldCurrent = m_fieldCurrent->fieldWithoutArray;
            }
            else
            {
                m_structCurrent = MetaDataGlobal::instance().getStruct(*m_fieldCurrent);
            }
        }
    }
}

void ParserJson::enterKey(const char* key, ssize_t size)
{
    enterKey(std::string(key, size));
}

void ParserJson::enterKey(std::string&& key)
{
    if (m_jsonTypeActive > 0)
    {
        return;
    }
    m_fieldCurrent = nullptr;
    if (m_structCurrent)
    {
        m_fieldCurrent = m_structCurrent->getFieldByName(key);
        if (m_fieldCurrent && (m_fieldCurrent->typeId == TYPE_JSON))
        {
            assert(m_jsonTypeActive == 0);
            m_jsonTypeActive = 1;

            startOfJson();
        }
    }
}

void ParserJson::startOfJson()
{
    assert(m_jsonStart == nullptr);
    m_jsonStart = m_parser.getCurrentPosition();
}

void ParserJson::endOfJson()
{
    assert(m_fieldCurrent != nullptr);
    assert(m_jsonStart != nullptr);
    const char* jsonEnd = m_parser.getCurrentPosition();
    assert(jsonEnd != nullptr);
    while ((*m_jsonStart != ':') && (m_jsonStart < jsonEnd))
    {
        ++m_jsonStart;
    }
    ++m_jsonStart;
    const std::size_t size = jsonEnd - m_jsonStart;
    m_visitor.enterJsonString(*m_fieldCurrent, m_jsonStart, size);
}


void ParserJson::finished()
{
}

} // namespace finalmq
