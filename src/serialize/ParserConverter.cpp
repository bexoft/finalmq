
#include "serialize/ParserConverter.h"
#include "metadata/MetaData.h"

#include "conversions/itoa.h"
#include "conversions/dtoa.h"

#include <assert.h>
#include <iostream>
#include <algorithm>




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

void ParserConverter::finished()
{
    assert(m_visitor);
    m_visitor->finished();
}

void ParserConverter::enterStruct(const MetaField& field)
{
    assert(m_visitor);
    if (field.type == MetaTypeId::TYPE_STRUCT)
    {
        m_visitor->enterStruct(field);
    }
}
void ParserConverter::exitStruct(const MetaField& field)
{
    if (field.type == MetaTypeId::TYPE_STRUCT)
    {
        m_visitor->exitStruct(field);
    }
}

void ParserConverter::enterArrayStruct(const MetaField& field)
{
    if (field.type == MetaTypeId::TYPE_ARRAY_STRUCT)
    {
        m_visitor->enterArrayStruct(field);
    }
}
void ParserConverter::exitArrayStruct(const MetaField& field)
{
    if (field.type == MetaTypeId::TYPE_ARRAY_STRUCT)
    {
        m_visitor->exitArrayStruct(field);
    }
}

void ParserConverter::enterBool(const MetaField& field, bool value)
{
    if (field.type == MetaTypeId::TYPE_BOOL)
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
    if (field.type == MetaTypeId::TYPE_INT32)
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
    if (field.type == MetaTypeId::TYPE_UINT32)
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
    if (field.type == MetaTypeId::TYPE_INT64)
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
    if (field.type == MetaTypeId::TYPE_UINT64)
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
    if (field.type == MetaTypeId::TYPE_FLOAT)
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
    if (field.type == MetaTypeId::TYPE_DOUBLE)
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
    if (field.type == MetaTypeId::TYPE_STRING)
    {
        m_visitor->enterString(field, std::move(value));
    }
    else
    {
        convertString(field, value.data(), value.size());
    }
}
void ParserConverter::enterString(const MetaField& field, const char* value, int size)
{
    if (field.type == MetaTypeId::TYPE_STRING)
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
    if (field.type == MetaTypeId::TYPE_BYTES)
    {
        m_visitor->enterBytes(field, std::move(value));
    }
    else
    {
        std::cout << "bytes not expected" << std::endl;
    }
}
void ParserConverter::enterBytes(const MetaField& field, const BytesElement* value, int size)
{
    if (field.type == MetaTypeId::TYPE_BYTES)
    {
        m_visitor->enterBytes(field, value, size);
    }
    else
    {
        std::cout << "bytes not expected" << std::endl;
    }
}
void ParserConverter::enterEnum(const MetaField& field, std::int32_t value)
{
    if (field.type == MetaTypeId::TYPE_ENUM)
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
    if (field.type == MetaTypeId::TYPE_ENUM)
    {
        m_visitor->enterEnum(field, std::move(value));
    }
    else
    {
        convertString(field, value.data(), value.size());
    }
}
void ParserConverter::enterEnum(const MetaField& field, const char* value, int size)
{
    if (field.type == MetaTypeId::TYPE_ENUM)
    {
        m_visitor->enterEnum(field, value);
    }
    else
    {
        convertString(field, value, size);
    }
}

void ParserConverter::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
    if (field.type == MetaTypeId::TYPE_ARRAY_BOOL)
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
    if (field.type == MetaTypeId::TYPE_ARRAY_BOOL)
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
    if (field.type == MetaTypeId::TYPE_ARRAY_INT32)
    {
        m_visitor->enterArrayInt32(field, std::move(value));
    }
    else
    {
        convertArraytNumber(field, value);
    }
}
void ParserConverter::enterArrayInt32(const MetaField& field, const std::int32_t* value, int size)
{
    if (field.type == MetaTypeId::TYPE_ARRAY_INT32)
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
    if (field.type == MetaTypeId::TYPE_ARRAY_UINT32)
    {
        m_visitor->enterArrayUInt32(field, std::move(value));
    }
    else
    {
        convertArraytNumber(field, value);
    }
}
void ParserConverter::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, int size)
{
    if (field.type == MetaTypeId::TYPE_ARRAY_UINT32)
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
    if (field.type == MetaTypeId::TYPE_ARRAY_INT64)
    {
        m_visitor->enterArrayInt64(field, std::move(value));
    }
    else
    {
        convertArraytNumber(field, value);
    }
}
void ParserConverter::enterArrayInt64(const MetaField& field, const std::int64_t* value, int size)
{
    if (field.type == MetaTypeId::TYPE_ARRAY_INT64)
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
    if (field.type == MetaTypeId::TYPE_ARRAY_UINT64)
    {
        m_visitor->enterArrayUInt64(field, std::move(value));
    }
    else
    {
        convertArraytNumber(field, value);
    }
}
void ParserConverter::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, int size)
{
    if (field.type == MetaTypeId::TYPE_ARRAY_UINT64)
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
    if (field.type == MetaTypeId::TYPE_ARRAY_FLOAT)
    {
        m_visitor->enterArrayFloat(field, std::move(value));
    }
    else
    {
        convertArraytNumber(field, value);
    }
}
void ParserConverter::enterArrayFloat(const MetaField& field, const float* value, int size)
{
    if (field.type == MetaTypeId::TYPE_ARRAY_FLOAT)
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
    if (field.type == MetaTypeId::TYPE_ARRAY_DOUBLE)
    {
        m_visitor->enterArrayDouble(field, std::move(value));
    }
    else
    {
        convertArraytNumber(field, value);
    }
}
void ParserConverter::enterArrayDouble(const MetaField& field, const double* value, int size)
{
    if (field.type == MetaTypeId::TYPE_ARRAY_DOUBLE)
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
    if (field.type == MetaTypeId::TYPE_ARRAY_STRING)
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
    if (field.type == MetaTypeId::TYPE_ARRAY_STRING)
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
    if (field.type == MetaTypeId::TYPE_ARRAY_BYTES)
    {
        m_visitor->enterArrayBytesMove(field, std::move(value));
    }
    else
    {
        std::cout << "bytes array not expected" << std::endl;
    }
}
void ParserConverter::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
    if (field.type == MetaTypeId::TYPE_ARRAY_BYTES)
    {
        m_visitor->enterArrayBytes(field, value);
    }
    else
    {
        std::cout << "bytes array not expected" << std::endl;
    }
}
void ParserConverter::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
    if (field.type == MetaTypeId::TYPE_ARRAY_ENUM)
    {
        m_visitor->enterArrayEnum(field, std::move(value));
    }
    else
    {
        convertArraytNumber(field, value);
    }
}
void ParserConverter::enterArrayEnum(const MetaField& field, const std::int32_t* value, int size)
{
    if (field.type == MetaTypeId::TYPE_ARRAY_ENUM)
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
    if (field.type == MetaTypeId::TYPE_ARRAY_ENUM)
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
    if (field.type == MetaTypeId::TYPE_ARRAY_ENUM)
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
    switch (field.type)
    {
    case MetaTypeId::TYPE_BOOL:
        m_visitor->enterBool(field, value);
        break;
    case MetaTypeId::TYPE_INT32:
        m_visitor->enterInt32(field, value);
        break;
    case MetaTypeId::TYPE_UINT32:
        m_visitor->enterUInt32(field, value);
        break;
    case MetaTypeId::TYPE_INT64:
        m_visitor->enterInt64(field, value);
        break;
    case MetaTypeId::TYPE_UINT64:
        m_visitor->enterUInt64(field, value);
        break;
    case MetaTypeId::TYPE_FLOAT:
        m_visitor->enterFloat(field, value);
        break;
    case MetaTypeId::TYPE_DOUBLE:
        m_visitor->enterDouble(field, value);
        break;
    case MetaTypeId::TYPE_STRING:
        m_visitor->enterString(field, std::to_string(value));
        break;
    case MetaTypeId::TYPE_ENUM:
        m_visitor->enterEnum(field, value);
        break;
    case MetaTypeId::TYPE_ARRAY_BOOL:
        {
            bool v = value;
            m_visitor->enterArrayBool(field, std::vector<bool>(&v, &v + 1));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_INT32:
        {
            std::int32_t v = value;
            m_visitor->enterArrayInt32(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_UINT32:
        {
            std::uint32_t v = value;
            m_visitor->enterArrayUInt32(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_INT64:
        {
            std::int64_t v = value;
            m_visitor->enterArrayInt64(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_UINT64:
        {
            std::uint64_t v = value;
            m_visitor->enterArrayUInt64(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_FLOAT:
        {
            float v = value;
            m_visitor->enterArrayFloat(field, &v, 1);
        }
        break;
    case MetaTypeId::TYPE_ARRAY_DOUBLE:
        {
            double v = value;
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
            m_visitor->enterArrayString(field, {std::to_string(value)});
        }
        break;
    case MetaTypeId::TYPE_ARRAY_ENUM:
        {
            std::int32_t v = value;
            m_visitor->enterArrayEnum(field, &v, 1);
        }
        break;
    default:
        std::cout << "number not expected";
        break;
    }
}


void ParserConverter::convertString(const MetaField& field, const char* value, int size)
{
    switch (field.type)
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
        std::cout << "number not expected";
        break;
    }
}


template<class T>
void ParserConverter::convertArraytNumber(const MetaField& field, const T* value, int size)
{
    switch (field.type)
    {
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
                v.push_back(entry);
            });
            m_visitor->enterArrayInt32(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_UINT32:
        {
            std::vector<std::uint32_t> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v] (const T& entry) {
                v.push_back(entry);
            });
            m_visitor->enterArrayUInt32(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_INT64:
        {
            std::vector<std::int64_t> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v] (const T& entry) {
                v.push_back(entry);
            });
            m_visitor->enterArrayInt64(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_UINT64:
        {
            std::vector<std::uint64_t> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v] (const T& entry) {
                v.push_back(entry);
            });
            m_visitor->enterArrayUInt64(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_FLOAT:
        {
            std::vector<float> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v] (const T& entry) {
                v.push_back(entry);
            });
            m_visitor->enterArrayFloat(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_DOUBLE:
        {
            std::vector<double> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v] (const T& entry) {
                v.push_back(entry);
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
                v.push_back(entry);
            });
            m_visitor->enterArrayEnum(field, std::move(v));
        }
        break;
    default:
        std::cout << "array not expected";
        break;
    }
}

template<class T>
void ParserConverter::convertArraytNumber(const MetaField& field, const std::vector<T>& value)
{
    int size = value.size();
    switch (field.type)
    {
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
                v.push_back(entry);
            });
            m_visitor->enterArrayInt32(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_UINT32:
        {
            std::vector<std::uint32_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const T& entry) {
                v.push_back(entry);
            });
            m_visitor->enterArrayUInt32(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_INT64:
        {
            std::vector<std::int64_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const T& entry) {
                v.push_back(entry);
            });
            m_visitor->enterArrayInt64(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_UINT64:
        {
            std::vector<std::uint64_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const T& entry) {
                v.push_back(entry);
            });
            m_visitor->enterArrayUInt64(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_FLOAT:
        {
            std::vector<float> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const T& entry) {
                v.push_back(entry);
            });
            m_visitor->enterArrayFloat(field, std::move(v));
        }
        break;
    case MetaTypeId::TYPE_ARRAY_DOUBLE:
        {
            std::vector<double> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v] (const T& entry) {
                v.push_back(entry);
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
                v.push_back(entry);
            });
            m_visitor->enterArrayEnum(field, std::move(v));
        }
        break;
    default:
        std::cout << "array not expected";
        break;
    }
}

void ParserConverter::convertArraytString(const MetaField& field, const std::vector<std::string>& value)
{
    size_t size = value.size();
    switch (field.type)
    {
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
        std::cout << "array not expected";
        break;
    }
}

