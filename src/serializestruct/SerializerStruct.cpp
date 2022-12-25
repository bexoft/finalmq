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


#include "finalmq/serializestruct/SerializerStruct.h"
#include "finalmq/serializevariant/VarValueToVariant.h"
#include "finalmq/serialize/ParserProcessDefaultValues.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValueList.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/logger/LogStream.h"
#include "finalmq/helpers/ModulenameFinalmq.h"

#include <assert.h>
#include <algorithm>


namespace finalmq {


static const std::string STR_VARVALUE = "finalmq.variant.VarValue";


SerializerStruct::SerializerStruct(StructBase& root)
    : ParserConverter()
    , m_internal(*this, root)
{
    setVisitor(m_internal);
}


void SerializerStruct::setExitNotification(std::function<void()> funcExit)
{
    m_internal.setExitNotification(std::move(funcExit));
}



SerializerStruct::Internal::Internal(SerializerStruct& outer, StructBase& root)
    : m_root(root)
    , m_outer(outer)
{
    m_root.clear();
    m_stack.push_back({ &m_root, -1 });
    m_current = &m_stack.back();
}

void SerializerStruct::Internal::setExitNotification(std::function<void()> funcExit)
{
    m_funcExit = std::move(funcExit);
}


// IParserVisitor
void SerializerStruct::Internal::notifyError(const char* /*str*/, const char* /*message*/)
{
}


void SerializerStruct::Internal::startStruct(const MetaStruct& /*stru*/)
{
    m_wasStartStructCalled = true;
}

void SerializerStruct::Internal::finished()
{
}


void SerializerStruct::Internal::enterStruct(const MetaField& field)
{
    assert(!m_stack.empty());
    assert(m_current);

    if (m_wasStartStructCalled && field.typeName == STR_VARVALUE)
    {
        m_varValueToVariant = nullptr;
        Variant* variant = &m_variantDummy;
        const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
        if (fieldInfoDest)
        {
            variant = m_current->structBase->getData<Variant>(*fieldInfoDest);
        }
        assert(variant);
        m_varValueToVariant = std::make_shared<VarValueToVariant>(*variant);
        m_outer.ParserConverter::setVisitor(m_varValueToVariant->getVisitor());
        m_varValueToVariant->setExitNotification([this, &field]() {
            assert(m_varValueToVariant);
            m_outer.ParserConverter::setVisitor(*this);
            m_varValueToVariant->convert();
            m_varValueToVariant->setExitNotification(nullptr);
        });
    }
    else
    {
        StructBase* sub = nullptr;
        if (m_current->structBase)
        {
            if (m_current->structArrayIndex == -1)
            {
                const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
                if (fieldInfoDest)
                {
                    sub = m_current->structBase->getData<StructBase>(*fieldInfoDest);
                }
            }
            else
            {
                sub = m_current->structBase->add(m_current->structArrayIndex);
            }
        }
        m_stack.push_back({ sub, -1 });
        m_current = &m_stack.back();
    }
}

void SerializerStruct::Internal::exitStruct(const MetaField& /*field*/)
{
    if (!m_stack.empty())
    {
        m_stack.pop_back();
        if (!m_stack.empty())
        {
            m_current = &m_stack.back();
        }
        else
        {
            m_current = nullptr;
            if (m_funcExit)
            {
                m_funcExit();
            }
        }
    }
}


void SerializerStruct::Internal::enterArrayStruct(const MetaField& field)
{
    assert(m_current);
    m_current->structArrayIndex = field.index;
}

void SerializerStruct::Internal::exitArrayStruct(const MetaField& /*field*/)
{
    assert(m_current);
    m_current->structArrayIndex = -1;
}



template <class T>
void SerializerStruct::Internal::setValue(StructBase& structBase, const FieldInfo& fieldInfoDest, const T& value)
{
    T* pval = structBase.getData<T>(fieldInfoDest);
    if (pval)
    {
        *pval = value;
    }
}

template <class T>
void SerializerStruct::Internal::setValue(StructBase& structBase, const FieldInfo& fieldInfoDest, T&& value)
{
    T* pval = structBase.getData<T>(fieldInfoDest);
    if (pval)
    {
        *pval = std::move(value);
    }
}


const FieldInfo* SerializerStruct::Internal::getFieldInfoDest(const MetaField& field)
{
    assert(m_current);
    StructBase* structBase = m_current->structBase;
    if (structBase)
    {
        const FieldInfo* fieldInfoDest = structBase->getStructInfo().getField(field.index);
        return fieldInfoDest;
    }
    return nullptr;
}


template<class T>
void SerializerStruct::Internal::convertNumber(StructBase& structBase, const FieldInfo& fieldInfoDest, T value)
{
    const MetaField* fieldDest = fieldInfoDest.getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    switch (fieldDest->typeId)
    {
    case MetaTypeId::TYPE_BOOL:
        setValue<bool>(structBase, fieldInfoDest, static_cast<bool>(value));
        break;
    case MetaTypeId::TYPE_INT32:
        setValue<std::int32_t>(structBase, fieldInfoDest, static_cast<std::int32_t>(value));
        break;
    case MetaTypeId::TYPE_UINT32:
        setValue<std::uint32_t>(structBase, fieldInfoDest, static_cast<std::uint32_t>(value));
        break;
    case MetaTypeId::TYPE_INT64:
        setValue<std::int64_t>(structBase, fieldInfoDest, static_cast<std::int64_t>(value));
        break;
    case MetaTypeId::TYPE_UINT64:
        setValue<std::uint64_t>(structBase, fieldInfoDest, static_cast<std::uint64_t>(value));
        break;
    case MetaTypeId::TYPE_FLOAT:
        setValue<float>(structBase, fieldInfoDest, static_cast<float>(value));
        break;
    case MetaTypeId::TYPE_DOUBLE:
        setValue<double>(structBase, fieldInfoDest, static_cast<double>(value));
        break;
    case MetaTypeId::TYPE_STRING:
        setValue<std::string>(structBase, fieldInfoDest, std::to_string(value));
        break;
    case MetaTypeId::TYPE_BYTES:
    {
        std::string str = std::to_string(value);
        Bytes bytes(str.begin(), str.end());
        setValue<Bytes>(structBase, fieldInfoDest, std::move(bytes));
    }
    break;
    case MetaTypeId::TYPE_ENUM:
    {
        bool validEnum = MetaDataGlobal::instance().isEnumValue(*fieldDest, static_cast<std::int32_t>(value));
        setValue<std::int32_t>(structBase, fieldInfoDest, static_cast<std::int32_t>(validEnum ? value : 0));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_BOOL:
    {
        bool v = static_cast<bool>(value);
        setValue<std::vector<bool>>(structBase, fieldInfoDest, { v });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_INT32:
    {
        std::int32_t v = static_cast<std::int32_t>(value);
        setValue<std::vector<std::int32_t>>(structBase, fieldInfoDest, { v });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_UINT32:
    {
        std::uint32_t v = static_cast<std::uint32_t>(value);
        setValue<std::vector<std::uint32_t>>(structBase, fieldInfoDest, { v });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_INT64:
    {
        std::int64_t v = static_cast<std::int64_t>(value);
        setValue<std::vector<std::int64_t>>(structBase, fieldInfoDest, { v });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_UINT64:
    {
        std::uint64_t v = static_cast<std::uint64_t>(value);
        setValue<std::vector<std::uint64_t>>(structBase, fieldInfoDest, { v });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_FLOAT:
    {
        float v = static_cast<float>(value);
        setValue<std::vector<float>>(structBase, fieldInfoDest, { v });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_DOUBLE:
    {
        double v = static_cast<double>(value);
        setValue<std::vector<double>>(structBase, fieldInfoDest, { v });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_STRING:
    {
        setValue<std::vector<std::string>>(structBase, fieldInfoDest, { std::to_string(value) });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_BYTES:
    {
        std::string str = std::to_string(value);
        Bytes bytes(str.begin(), str.end());
        setValue<std::vector<Bytes>>(structBase, fieldInfoDest, { bytes });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_ENUM:
    {
        std::int32_t v = static_cast<std::int32_t>(value);
        setValue<std::vector<std::int32_t>>(structBase, fieldInfoDest, { v });
    }
    break;
    default:
        streamError << "number not expected";
        break;
    }
}


void SerializerStruct::Internal::convertString(StructBase& structBase, const FieldInfo& fieldInfoDest, const char* value, ssize_t size)
{
    const MetaField* fieldDest = fieldInfoDest.getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    switch (fieldDest->typeId)
    {
    case MetaTypeId::TYPE_BOOL:
    {
        bool v = (size == 4 && (memcmp(value, "true", 4) == 0));
        setValue<bool>(structBase, fieldInfoDest, v);
    }
    break;
    case MetaTypeId::TYPE_INT32:
    {
        std::int32_t v = strtol(value, nullptr, 10);
        setValue<std::int32_t>(structBase, fieldInfoDest, v);
    }
    break;
    case MetaTypeId::TYPE_UINT32:
    {
        std::uint32_t v = strtoul(value, nullptr, 10);
        setValue<std::uint32_t>(structBase, fieldInfoDest, v);
    }
    break;
    case MetaTypeId::TYPE_INT64:
    {
        std::int64_t v = strtoll(value, nullptr, 10);
        setValue<std::int64_t>(structBase, fieldInfoDest, v);
    }
    break;
    case MetaTypeId::TYPE_UINT64:
    {
        std::uint64_t v = strtoull(value, nullptr, 10);
        setValue<std::uint64_t>(structBase, fieldInfoDest, v);
    }
    break;
    case MetaTypeId::TYPE_FLOAT:
    {
        float v = strtof32(value, nullptr);
        setValue<float>(structBase, fieldInfoDest, v);
    }
    break;
    case MetaTypeId::TYPE_DOUBLE:
    {
        double v = strtof64(value, nullptr);
        setValue<double>(structBase, fieldInfoDest, v);
    }
    break;
    case MetaTypeId::TYPE_STRING:
        setValue<std::string>(structBase, fieldInfoDest, std::string(value, size));
        break;
    case MetaTypeId::TYPE_BYTES:
    {
        Bytes bytes(value, value + size);
        setValue<Bytes>(structBase, fieldInfoDest, std::move(bytes));
    }
    break;
    case MetaTypeId::TYPE_ENUM:
    {
        std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(*fieldDest, std::string(value, size));
        setValue<std::int32_t>(structBase, fieldInfoDest, v);
    }
    break;
    case MetaTypeId::TYPE_ARRAY_BOOL:
    {
        bool v = (size == 4 && (memcmp(value, "true", 4) == 0));
        setValue<std::vector<bool>>(structBase, fieldInfoDest, { v });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_INT32:
    {
        std::int32_t v = strtol(value, nullptr, 10);
        setValue< std::vector<std::int32_t>>(structBase, fieldInfoDest, { v });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_UINT32:
    {
        std::uint32_t v = strtoul(value, nullptr, 10);
        setValue< std::vector<std::uint32_t>>(structBase, fieldInfoDest, { v });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_INT64:
    {
        std::int64_t v = strtoll(value, nullptr, 10);
        setValue< std::vector<std::int64_t>>(structBase, fieldInfoDest, { v });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_UINT64:
    {
        std::uint64_t v = strtoull(value, nullptr, 10);
        setValue< std::vector<std::uint64_t>>(structBase, fieldInfoDest, { v });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_FLOAT:
    {
        float v = strtof32(value, nullptr);
        setValue<std::vector<float>>(structBase, fieldInfoDest, { v });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_DOUBLE:
    {
        double v = strtof64(value, nullptr);
        setValue<std::vector<double>>(structBase, fieldInfoDest, { v });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_STRING:
    {
        setValue<std::vector<std::string>>(structBase, fieldInfoDest, { std::string(value, size) });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_BYTES:
    {
        Bytes bytes(value, value + size);
        setValue<std::vector<Bytes>>(structBase, fieldInfoDest, { std::move(bytes) });
    }
    break;
    case MetaTypeId::TYPE_ARRAY_ENUM:
    {
        std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(*fieldDest, std::string(value, size));
        setValue<std::vector<std::int32_t>>(structBase, fieldInfoDest, { v });
    }
    break;
    default:
        streamError << "number not expected";
        break;
    }
}

template<class T>
void SerializerStruct::Internal::convertArrayNumber(StructBase& structBase, const FieldInfo& fieldInfoDest, const T* value, ssize_t size)
{
    const MetaField* fieldDest = fieldInfoDest.getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    switch (fieldDest->typeId)
    {
    case MetaTypeId::TYPE_BOOL:
        if (size >= 1)
        {
            setValue<bool>(structBase, fieldInfoDest, static_cast<bool>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_INT32:
        if (size >= 1)
        {
            setValue<std::int32_t>(structBase, fieldInfoDest, static_cast<std::int32_t>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_UINT32:
        if (size >= 1)
        {
            setValue<std::uint32_t>(structBase, fieldInfoDest, static_cast<std::uint32_t>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_INT64:
        if (size >= 1)
        {
            setValue<std::int64_t>(structBase, fieldInfoDest, static_cast<std::int64_t>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_UINT64:
        if (size >= 1)
        {
            setValue<std::uint64_t>(structBase, fieldInfoDest, static_cast<std::uint64_t>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_FLOAT:
        if (size >= 1)
        {
            setValue<float>(structBase, fieldInfoDest, static_cast<float>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_DOUBLE:
        if (size >= 1)
        {
            setValue<double>(structBase, fieldInfoDest, static_cast<double>(value[0]));
        }
        break;
    case MetaTypeId::TYPE_STRING:
        if (size >= 1)
        {
            setValue<std::string>(structBase, fieldInfoDest, std::to_string(value[0]));
        }
        break;
    case MetaTypeId::TYPE_BYTES:
    {
        if (size >= 1)
        {
            std::string str = std::to_string(value[0]);
            Bytes bytes(str.begin(), str.end());
            setValue<Bytes>(structBase, fieldInfoDest, std::move(bytes));
        }
    }
    break;
    case MetaTypeId::TYPE_ENUM:
    {
        if (size >= 1)
        {
            bool validEnum = MetaDataGlobal::instance().isEnumValue(*fieldDest, static_cast<std::int32_t>(value[0]));
            setValue<std::int32_t>(structBase, fieldInfoDest, static_cast<std::int32_t>(validEnum ? value[0] : 0));
        }
    }
    break;
    case MetaTypeId::TYPE_ARRAY_BOOL:
    {
        std::vector<bool> v;
        v.reserve(size);
        std::for_each(value, value + size, [&v](const T& entry) {
            v.push_back(entry);
            });
        setValue<std::vector<bool>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_INT32:
    {
        std::vector<std::int32_t> v;
        v.reserve(size);
        std::for_each(value, value + size, [&v](const T& entry) {
            v.push_back(static_cast<std::int32_t>(entry));
            });
        setValue<std::vector<std::int32_t>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_UINT32:
    {
        std::vector<std::uint32_t> v;
        v.reserve(size);
        std::for_each(value, value + size, [&v](const T& entry) {
            v.push_back(static_cast<std::uint32_t>(entry));
            });
        setValue<std::vector<std::uint32_t>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_INT64:
    {
        std::vector<std::int64_t> v;
        v.reserve(size);
        std::for_each(value, value + size, [&v](const T& entry) {
            v.push_back(static_cast<std::int64_t>(entry));
            });
        setValue<std::vector<std::int64_t>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_UINT64:
    {
        std::vector<std::uint64_t> v;
        v.reserve(size);
        std::for_each(value, value + size, [&v](const T& entry) {
            v.push_back(static_cast<std::uint64_t>(entry));
            });
        setValue<std::vector<std::uint64_t>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_FLOAT:
    {
        std::vector<float> v;
        v.reserve(size);
        std::for_each(value, value + size, [&v](const T& entry) {
            v.push_back(static_cast<float>(entry));
            });
        setValue<std::vector<float>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_DOUBLE:
    {
        std::vector<double> v;
        v.reserve(size);
        std::for_each(value, value + size, [&v](const T& entry) {
            v.push_back(static_cast<double>(entry));
            });
        setValue<std::vector<double>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_STRING:
    {
        std::vector<std::string> v;
        v.reserve(size);
        std::for_each(value, value + size, [&v](const T& entry) {
            v.push_back(std::to_string(entry));
            });
        setValue<std::vector<std::string>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_BYTES:
    {
        std::vector<Bytes> v;
        v.reserve(size);
        std::for_each(value, value + size, [&v](const T& entry) {
            std::string str = std::to_string(entry);
            Bytes bytes(str.begin(), str.end());
            v.push_back(bytes);
        });
        setValue<std::vector<Bytes>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_ENUM:
    {
        std::vector<std::int32_t> v;
        v.reserve(size);
        std::for_each(value, value + size, [&v, fieldDest](const T& entry) {
            bool validEnum = MetaDataGlobal::instance().isEnumValue(*fieldDest, static_cast<std::int32_t>(entry));
            v.push_back(static_cast<std::int32_t>(validEnum ? entry : 0));
            });
        setValue<std::vector<std::int32_t>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    default:
        streamError << "array not expected";
        break;
    }
}

void SerializerStruct::Internal::convertArrayString(StructBase& structBase, const FieldInfo& fieldInfoDest, const std::vector<std::string>& value)
{
    const size_t size = value.size();
    const MetaField* fieldDest = fieldInfoDest.getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    switch (fieldDest->typeId)
    {
    case MetaTypeId::TYPE_BOOL:
        if (size >= 1)
        {
            bool val = (value[0].size() == 4 && (memcmp(value[0].c_str(), "true", 4) == 0));
            setValue<std::vector<bool>>(structBase, fieldInfoDest, { val });
        }
        break;
    case MetaTypeId::TYPE_INT32:
        if (size >= 1)
        {
            setValue<std::int32_t>(structBase, fieldInfoDest, static_cast<std::int32_t>(strtol(value[0].c_str(), nullptr, 10)));
        }
        break;
    case MetaTypeId::TYPE_UINT32:
        if (size >= 1)
        {
            setValue<std::uint32_t>(structBase, fieldInfoDest, static_cast<std::uint32_t>(strtoul(value[0].c_str(), nullptr, 10)));
        }
        break;
    case MetaTypeId::TYPE_INT64:
        if (size >= 1)
        {
            setValue<std::int64_t>(structBase, fieldInfoDest, static_cast<std::int64_t>(strtoll(value[0].c_str(), nullptr, 10)));
        }
        break;
    case MetaTypeId::TYPE_UINT64:
        if (size >= 1)
        {
            setValue<std::uint64_t>(structBase, fieldInfoDest, static_cast<std::uint64_t>(strtoull(value[0].c_str(), nullptr, 10)));
        }
        break;
    case MetaTypeId::TYPE_FLOAT:
        if (size >= 1)
        {
            setValue<float>(structBase, fieldInfoDest, static_cast<float>(strtof32(value[0].c_str(), nullptr)));
        }
        break;
    case MetaTypeId::TYPE_DOUBLE:
        if (size >= 1)
        {
            setValue<double>(structBase, fieldInfoDest, static_cast<double>(strtof64(value[0].c_str(), nullptr)));
        }
        break;
    case MetaTypeId::TYPE_STRING:
        if (size >= 1)
        {
            setValue<std::string>(structBase, fieldInfoDest, value[0]);
        }
        break;
    case MetaTypeId::TYPE_BYTES:
    {
        if (size >= 1)
        {
            Bytes bytes(value[0].begin(), value[0].end());
            setValue<Bytes>(structBase, fieldInfoDest, std::move(bytes));
        }
    }
    break;
    case MetaTypeId::TYPE_ENUM:
    {
        if (size >= 1)
        {
            std::int32_t enumValue = MetaDataGlobal::instance().getEnumValueByName(*fieldDest, value[0]);
            setValue<std::int32_t>(structBase, fieldInfoDest, enumValue);
        }
    }
    break;
    case MetaTypeId::TYPE_ARRAY_BOOL:
    {
        std::vector<bool> v;
        v.reserve(size);
        std::for_each(value.begin(), value.end(), [&v](const std::string& entry) {
            bool val = (entry.size() == 4 && (memcmp(entry.c_str(), "true", 4) == 0));
            v.push_back(val);
            });
        setValue<std::vector<bool>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_INT32:
    {
        std::vector<std::int32_t> v;
        v.reserve(size);
        std::for_each(value.begin(), value.end(), [&v](const std::string& entry) {
            v.push_back(strtol(entry.c_str(), nullptr, 10));
            });
        setValue<std::vector<std::int32_t>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_UINT32:
    {
        std::vector<std::uint32_t> v;
        v.reserve(size);
        std::for_each(value.begin(), value.end(), [&v](const std::string& entry) {
            v.push_back(strtoul(entry.c_str(), nullptr, 10));
            });
        setValue<std::vector<std::uint32_t>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_INT64:
    {
        std::vector<std::int64_t> v;
        v.reserve(size);
        std::for_each(value.begin(), value.end(), [&v](const std::string& entry) {
            v.push_back(strtoll(entry.c_str(), nullptr, 10));
            });
        setValue<std::vector<std::int64_t>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_UINT64:
    {
        std::vector<std::uint64_t> v;
        v.reserve(size);
        std::for_each(value.begin(), value.end(), [&v](const std::string& entry) {
            v.push_back(strtoull(entry.c_str(), nullptr, 10));
            });
        setValue<std::vector<std::uint64_t>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_FLOAT:
    {
        std::vector<float> v;
        v.reserve(size);
        std::for_each(value.begin(), value.end(), [&v](const std::string& entry) {
            v.push_back(strtof32(entry.c_str(), nullptr));
            });
        setValue<std::vector<float>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_DOUBLE:
    {
        std::vector<double> v;
        v.reserve(size);
        std::for_each(value.begin(), value.end(), [&v](const std::string& entry) {
            v.push_back(strtof64(entry.c_str(), nullptr));
            });
        setValue<std::vector<double>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_STRING:
        setValue<std::vector<std::string>>(structBase, fieldInfoDest, value);
        break;
    case MetaTypeId::TYPE_ARRAY_BYTES:
    {
        std::vector<Bytes> v;
        v.reserve(size);
        std::for_each(value.begin(), value.end(), [&v](const std::string& entry) {
            std::string str(entry.begin(), entry.end());
            Bytes bytes(str.begin(), str.end());
            v.push_back(bytes);
            });
        setValue<std::vector<Bytes>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    case MetaTypeId::TYPE_ARRAY_ENUM:
    {
        std::vector<std::int32_t> v;
        v.reserve(size);
        std::for_each(value.begin(), value.end(), [&v, fieldDest](const std::string& entry) {
            std::int32_t enumValue = MetaDataGlobal::instance().getEnumValueByName(*fieldDest, entry);
            v.push_back(enumValue);
        });
        setValue<std::vector<std::int32_t>>(structBase, fieldInfoDest, std::move(v));
    }
    break;
    default:
        streamError << "array not expected";
        break;
    }
}


template<class T>
void SerializerStruct::Internal::setValueNumber(const MetaField& field, T value)
{
    const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
    if (fieldInfoDest == nullptr)
    {
        return;
    }
    const MetaField* fieldDest = fieldInfoDest->getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    StructBase* structBase = m_current->structBase;
    if (structBase == nullptr)
    {
        return;
    }

    if (fieldDest->typeId == MetaTypeInfo<T>::TypeId)
    {
        setValue<T>(*structBase, *fieldInfoDest, value);
    }
    else
    {
        convertNumber(*structBase, *fieldInfoDest, value);
    }
}

void SerializerStruct::Internal::setValueString(const MetaField& field, const char* value, ssize_t size)
{
    const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
    if (fieldInfoDest == nullptr)
    {
        return;
    }
    const MetaField* fieldDest = fieldInfoDest->getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    StructBase* structBase = m_current->structBase;
    if (structBase == nullptr)
    {
        return;
    }

    if (fieldDest->typeId == MetaTypeId::TYPE_STRING)
    {
        setValue<std::string>(*structBase, *fieldInfoDest, std::string(value, size));
    }
    else
    {
        convertString(*structBase, *fieldInfoDest, value, size);
    }
}

template<class T>
void SerializerStruct::Internal::setValueArrayNumber(const MetaField& field, const T* value, ssize_t size)
{
    const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
    if (fieldInfoDest == nullptr)
    {
        return;
    }
    const MetaField* fieldDest = fieldInfoDest->getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    StructBase* structBase = m_current->structBase;
    if (structBase == nullptr)
    {
        return;
    }

    if (fieldDest->typeId == MetaTypeInfo<T>::TypeId)
    {
        setValue<std::vector<T>>(*structBase, *fieldInfoDest, std::vector<T>(value, value + size));
    }
    else
    {
        convertArrayNumber(*structBase, *fieldInfoDest, value, size);
    }
}

template<class T>
void SerializerStruct::Internal::setValueArrayNumber(const MetaField& field, std::vector<T>&& value)
{
    const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
    if (fieldInfoDest == nullptr)
    {
        return;
    }
    const MetaField* fieldDest = fieldInfoDest->getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    StructBase* structBase = m_current->structBase;
    if (structBase == nullptr)
    {
        return;
    }

    if (fieldDest->typeId == MetaTypeInfo<T>::TypeId)
    {
        setValue<std::vector<T>>(*structBase, *fieldInfoDest, std::move(value));
    }
    else
    {
        convertArrayNumber(*structBase, *fieldInfoDest, value.data(), value.size());
    }
}


void SerializerStruct::Internal::setValueArrayString(const MetaField& field, const std::vector<std::string>& value)
{
    const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
    if (fieldInfoDest == nullptr)
    {
        return;
    }
    const MetaField* fieldDest = fieldInfoDest->getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    StructBase* structBase = m_current->structBase;
    if (structBase == nullptr)
    {
        return;
    }

    if (fieldDest->typeId == MetaTypeId::TYPE_ARRAY_STRING)
    {
        setValue<std::vector<std::string>>(*structBase, *fieldInfoDest, value);
    }
    else
    {
        convertArrayString(*structBase, *fieldInfoDest, value);
    }
}

void SerializerStruct::Internal::setValueArrayString(const MetaField& field, const std::vector<std::string>&& value)
{
    const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
    if (fieldInfoDest == nullptr)
    {
        return;
    }
    const MetaField* fieldDest = fieldInfoDest->getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    StructBase* structBase = m_current->structBase;
    if (structBase == nullptr)
    {
        return;
    }

    if (fieldDest->typeId == MetaTypeId::TYPE_ARRAY_STRING)
    {
        setValue<std::vector<std::string>>(*structBase, *fieldInfoDest, std::move(value));
    }
    else
    {
        convertArrayString(*structBase, *fieldInfoDest, value);
    }
}



void SerializerStruct::Internal::enterBool(const MetaField& field, bool value)
{
    setValueNumber<bool>(field, value);
}

void SerializerStruct::Internal::enterInt32(const MetaField& field, std::int32_t value)
{
    setValueNumber<std::int32_t>(field, value);
}

void SerializerStruct::Internal::enterUInt32(const MetaField& field, std::uint32_t value)
{
    setValueNumber<std::uint32_t>(field, value);
}

void SerializerStruct::Internal::enterInt64(const MetaField& field, std::int64_t value)
{
    setValueNumber<std::int64_t>(field, value);
}

void SerializerStruct::Internal::enterUInt64(const MetaField& field, std::uint64_t value)
{
    setValueNumber<std::uint64_t>(field, value);
}

void SerializerStruct::Internal::enterFloat(const MetaField& field, float value)
{
    setValueNumber<float>(field, value);
}

void SerializerStruct::Internal::enterDouble(const MetaField& field, double value)
{
    setValueNumber<double>(field, value);
}

void SerializerStruct::Internal::enterString(const MetaField& field, std::string&& value)
{
    const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
    if (fieldInfoDest == nullptr)
    {
        return;
    }
    const MetaField* fieldDest = fieldInfoDest->getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    StructBase* structBase = m_current->structBase;
    if (structBase == nullptr)
    {
        return;
    }

    if (fieldDest->typeId == MetaTypeId::TYPE_STRING)
    {
        setValue<std::string>(*structBase, *fieldInfoDest, std::move(value));
    }
    else
    {
        convertString(*structBase, *fieldInfoDest, value.c_str(), value.size());
    }
}

void SerializerStruct::Internal::enterString(const MetaField& field, const char* value, ssize_t size)
{
    setValueString(field, value, size);
}

void SerializerStruct::Internal::enterBytes(const MetaField& field, Bytes&& value)
{
    const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
    if (fieldInfoDest == nullptr)
    {
        return;
    }
    const MetaField* fieldDest = fieldInfoDest->getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    StructBase* structBase = m_current->structBase;
    if (structBase == nullptr)
    {
        return;
    }

    if (fieldDest->typeId == MetaTypeId::TYPE_BYTES)
    {
        setValue<Bytes>(*structBase, *fieldInfoDest, std::move(value));
    }
    else
    {
        convertString(*structBase, *fieldInfoDest, value.data(), value.size());
    }
}

void SerializerStruct::Internal::enterBytes(const MetaField& field, const BytesElement* value, ssize_t size)
{
    const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
    if (fieldInfoDest == nullptr)
    {
        return;
    }
    const MetaField* fieldDest = fieldInfoDest->getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    StructBase* structBase = m_current->structBase;
    if (structBase == nullptr)
    {
        return;
    }

    if (fieldDest->typeId == MetaTypeId::TYPE_BYTES)
    {
        setValue<Bytes>(*structBase, *fieldInfoDest, Bytes(value, value + size));
    }
    else
    {
        convertString(*structBase, *fieldInfoDest, value, size);
    }
}

void SerializerStruct::Internal::enterEnum(const MetaField& field, std::int32_t value)
{
    const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
    if (fieldInfoDest == nullptr)
    {
        return;
    }
    const MetaField* fieldDest = fieldInfoDest->getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    StructBase* structBase = m_current->structBase;
    if (structBase == nullptr)
    {
        return;
    }

    if (fieldDest->typeId == MetaTypeId::TYPE_ENUM)
    {
        bool validEnum = MetaDataGlobal::instance().isEnumValue(*fieldDest, value);
        setValue<std::int32_t>(*structBase, *fieldInfoDest, validEnum ? value : 0);
    }
    else
    {
        convertNumber(*structBase, *fieldInfoDest, value);
    }
}


void SerializerStruct::Internal::enterEnum(const MetaField& field, std::string&& value)
{
    const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
    if (fieldInfoDest == nullptr)
    {
        return;
    }
    const MetaField* fieldDest = fieldInfoDest->getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    StructBase* structBase = m_current->structBase;
    if (structBase == nullptr)
    {
        return;
    }

    if (fieldDest->typeId == MetaTypeId::TYPE_ENUM)
    {
        std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(*fieldDest, value);
        setValue<std::int32_t>(*structBase, *fieldInfoDest, v);
    }
    else
    {
        convertString(*structBase, *fieldInfoDest, value.c_str(), value.size());
    }
}

void SerializerStruct::Internal::enterEnum(const MetaField& field, const char* value, ssize_t size)
{
    const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
    if (fieldInfoDest == nullptr)
    {
        return;
    }
    const MetaField* fieldDest = fieldInfoDest->getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    StructBase* structBase = m_current->structBase;
    if (structBase == nullptr)
    {
        return;
    }

    if (fieldDest->typeId == MetaTypeId::TYPE_ENUM)
    {
        std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(*fieldDest, std::string(value, size));
        setValue<std::int32_t>(*structBase, *fieldInfoDest, v);
    }
    else
    {
        convertString(*structBase, *fieldInfoDest, value, size);
    }
}

void SerializerStruct::Internal::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
    const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
    if (fieldInfoDest == nullptr)
    {
        return;
    }
    const MetaField* fieldDest = fieldInfoDest->getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    StructBase* structBase = m_current->structBase;
    if (structBase == nullptr)
    {
        return;
    }

    if (fieldDest->typeId == MetaTypeInfo<bool>::TypeId)
    {
        setValue<std::vector<bool>>(*structBase, *fieldInfoDest, std::move(value));
    }
    else
    {
        std::vector<std::int32_t> v;
        v.reserve(value.size());
        for (size_t i = 0; i < value.size(); ++i)
        {
            v.push_back(static_cast<std::int32_t>(value[i]));
        }
        convertArrayNumber(*structBase, *fieldInfoDest, v.data(), v.size());
    }
}

void SerializerStruct::Internal::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
{
    const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
    if (fieldInfoDest == nullptr)
    {
        return;
    }
    const MetaField* fieldDest = fieldInfoDest->getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    StructBase* structBase = m_current->structBase;
    if (structBase == nullptr)
    {
        return;
    }

    if (fieldDest->typeId == MetaTypeInfo<bool>::TypeId)
    {
        setValue<std::vector<bool>>(*structBase, *fieldInfoDest, value);
    }
    else
    {
        std::vector<std::int32_t> v;
        v.reserve(value.size());
        for (size_t i = 0; i < value.size(); ++i)
        {
            v.push_back(static_cast<std::int32_t>(value[i]));
        }
        convertArrayNumber(*structBase, *fieldInfoDest, v.data(), v.size());
    }
}

void SerializerStruct::Internal::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
{
    setValueArrayNumber<std::int32_t>(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayInt32(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    setValueArrayNumber<std::int32_t>(field, value, size);
}

void SerializerStruct::Internal::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
{
    setValueArrayNumber<std::uint32_t>(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, ssize_t size)
{
    setValueArrayNumber<std::uint32_t>(field, value, size);
}

void SerializerStruct::Internal::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
{
    setValueArrayNumber<std::int64_t>(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayInt64(const MetaField& field, const std::int64_t* value, ssize_t size)
{
    setValueArrayNumber<std::int64_t>(field, value, size);
}

void SerializerStruct::Internal::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
{
    setValueArrayNumber<std::uint64_t>(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, ssize_t size)
{
    setValueArrayNumber<std::uint64_t>(field, value, size);
}

void SerializerStruct::Internal::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
{
    setValueArrayNumber<float>(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayFloat(const MetaField& field, const float* value, ssize_t size)
{
    setValueArrayNumber<float>(field, value, size);
}

void SerializerStruct::Internal::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
{
    setValueArrayNumber<double>(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayDouble(const MetaField& field, const double* value, ssize_t size)
{
    setValueArrayNumber<double>(field, value, size);
}

void SerializerStruct::Internal::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
{
    setValueArrayString(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
{
    setValueArrayString(field, value);
}

void SerializerStruct::Internal::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
{
    const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
    if (fieldInfoDest == nullptr)
    {
        return;
    }
    const MetaField* fieldDest = fieldInfoDest->getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    StructBase* structBase = m_current->structBase;
    if (structBase == nullptr)
    {
        return;
    }

    if (fieldDest->typeId == MetaTypeId::TYPE_ARRAY_BYTES)
    {
        setValue<std::vector<Bytes>>(*structBase, *fieldInfoDest, std::move(value));
    }
    else
    {
        std::vector<std::string> strings;
        strings.reserve(value.size());
        for (size_t i = 0; i < value.size(); ++i)
        {
            strings.push_back(std::string(value[i].data(), value[i].size()));
        }
        convertArrayString(*structBase, *fieldInfoDest, strings);
    }
}

void SerializerStruct::Internal::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
    const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
    if (fieldInfoDest == nullptr)
    {
        return;
    }
    const MetaField* fieldDest = fieldInfoDest->getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    StructBase* structBase = m_current->structBase;
    if (structBase == nullptr)
    {
        return;
    }

    if (fieldDest->typeId == MetaTypeId::TYPE_ARRAY_BYTES)
    {
        setValue<std::vector<Bytes>>(*structBase, *fieldInfoDest, value);
    }
    else
    {
        std::vector<std::string> strings;
        strings.reserve(value.size());
        for (size_t i = 0; i < value.size(); ++i)
        {
            strings.push_back(std::string(value[i].data(), value[i].size()));
        }
        convertArrayString(*structBase, *fieldInfoDest, strings);
    }
}

void SerializerStruct::Internal::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
    setValueArrayNumber<std::int32_t>(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayEnum(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    setValueArrayNumber<std::int32_t>(field, value, size);
}

void SerializerStruct::Internal::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
{
    setValueArrayString(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
{
    setValueArrayString(field, value);
}

} // namespace finalmq
