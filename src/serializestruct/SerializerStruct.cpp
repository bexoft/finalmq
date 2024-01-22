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

#include <algorithm>

#include <assert.h>

#include "finalmq/helpers/ModulenameFinalmq.h"
#include "finalmq/logger/LogStream.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/serialize/ParserProcessDefaultValues.h"
#include "finalmq/serializevariant/VarValueToVariant.h"
#include "finalmq/variant/VariantValueList.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValues.h"

namespace finalmq
{
static const std::string STR_VARVALUE = "finalmq.variant.VarValue";

SerializerStruct::SerializerStruct(StructBase& root)
    : IParserVisitor(), m_root(root)
{
    m_root.clear();
    m_stack.push_back({&m_root, -1});
    m_current = &m_stack.back();
}

void SerializerStruct::setExitNotification(std::function<void()> funcExit)
{
    m_funcExit = std::move(funcExit);
}

// IParserVisitor
void SerializerStruct::notifyError(const char* /*str*/, const char* /*message*/)
{
}

void SerializerStruct::startStruct(const MetaStruct& /*stru*/)
{
    m_wasStartStructCalled = true;
}

void SerializerStruct::finished()
{
}

void SerializerStruct::enterStruct(const MetaField& field)
{
    if (field.typeId != MetaTypeId::TYPE_STRUCT && field.typeId != MetaTypeId::TYPE_ARRAY_STRUCT)
    {
        return;
    }

    if (m_visitor)
    {
        m_visitor->enterStruct(field);
        return;
    }

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
        m_visitor = &m_varValueToVariant->getVisitor();
        m_varValueToVariant->setExitNotification([this, &field]() {
            assert(m_varValueToVariant);
            m_visitor = nullptr;
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
                    if (!(fieldInfoDest->getField()->flags & METAFLAG_NULLABLE))
                    {
                        sub = m_current->structBase->getData<StructBase>(*fieldInfoDest);
                    }
                    else
                    {
                        StructBasePtr* subNullable = m_current->structBase->getData<StructBasePtr>(*fieldInfoDest);
                        if (subNullable)
                        {
                            if (subNullable->get() == nullptr)
                            {
                                *subNullable = fieldInfoDest->createStruct();
                            }
                            sub = subNullable->get();
                        }
                    }
                }
            }
            else
            {
                sub = m_current->structBase->add(m_current->structArrayIndex);
            }
        }
        m_stack.push_back({sub, -1});
        m_current = &m_stack.back();
    }
}

void SerializerStruct::exitStruct(const MetaField& field)
{
    if (field.typeId != MetaTypeId::TYPE_STRUCT && field.typeId != MetaTypeId::TYPE_ARRAY_STRUCT)
    {
        return;
    }

    if (m_visitor)
    {
        m_visitor->exitStruct(field);
        return;
    }

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

void SerializerStruct::enterStructNull(const MetaField& field)
{
    if (field.typeId != MetaTypeId::TYPE_STRUCT && field.typeId != MetaTypeId::TYPE_ARRAY_STRUCT)
    {
        return;
    }

    if (m_visitor)
    {
        m_visitor->enterStructNull(field);
        return;
    }

    assert(!m_stack.empty());
    assert(m_current);

    if (m_current->structBase)
    {
        const FieldInfo* fieldInfoDest = getFieldInfoDest(field);
        if (fieldInfoDest)
        {
            if (fieldInfoDest->getField()->flags & METAFLAG_NULLABLE)
            {
                StructBasePtr* subNullable = m_current->structBase->getData<StructBasePtr>(*fieldInfoDest);
                if (subNullable)
                {
                    *subNullable = nullptr;
                }
            }
        }
    }
}

void SerializerStruct::enterArrayStruct(const MetaField& field)
{
    if (m_visitor)
    {
        m_visitor->enterArrayStruct(field);
        return;
    }

    assert(m_current);
    m_current->structArrayIndex = field.index;
}

void SerializerStruct::exitArrayStruct(const MetaField& field)
{
    if (m_visitor)
    {
        m_visitor->exitArrayStruct(field);
        return;
    }

    assert(m_current);
    m_current->structArrayIndex = -1;
}

template<class T>
void SerializerStruct::setValue(StructBase& structBase, const FieldInfo& fieldInfoDest, const T& value)
{
    T* pval = structBase.getData<T>(fieldInfoDest);
    if (pval)
    {
        *pval = value;
    }
}

template<class T>
void SerializerStruct::setValue(StructBase& structBase, const FieldInfo& fieldInfoDest, T&& value)
{
    T* pval = structBase.getData<T>(fieldInfoDest);
    if (pval)
    {
        *pval = std::move(value);
    }
}

const FieldInfo* SerializerStruct::getFieldInfoDest(const MetaField& field)
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
void SerializerStruct::convertNumber(StructBase& structBase, const FieldInfo& fieldInfoDest, T value)
{
    const MetaField* fieldDest = fieldInfoDest.getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    switch(fieldDest->typeId)
    {
        case MetaTypeId::TYPE_BOOL:
#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
            setValue<bool>(structBase, fieldInfoDest, static_cast<bool>(value));
#ifndef WIN32
#pragma GCC diagnostic pop
#endif
            break;
        case MetaTypeId::TYPE_INT8:
            setValue<std::int8_t>(structBase, fieldInfoDest, static_cast<std::int8_t>(value));
            break;
        case MetaTypeId::TYPE_UINT8:
            setValue<std::uint8_t>(structBase, fieldInfoDest, static_cast<std::uint8_t>(value));
            break;
        case MetaTypeId::TYPE_INT16:
            setValue<std::int16_t>(structBase, fieldInfoDest, static_cast<std::int16_t>(value));
            break;
        case MetaTypeId::TYPE_UINT16:
            setValue<std::uint16_t>(structBase, fieldInfoDest, static_cast<std::uint16_t>(value));
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
#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
            bool v = static_cast<bool>(value);
#ifndef WIN32
#pragma GCC diagnostic pop
#endif
            setValue<std::vector<bool>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT8:
        {
            std::int8_t v = static_cast<std::int8_t>(value);
            setValue<std::vector<std::int8_t>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT16:
        {
            std::int16_t v = static_cast<std::int16_t>(value);
            setValue<std::vector<std::int16_t>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_UINT16:
        {
            std::uint16_t v = static_cast<std::uint16_t>(value);
            setValue<std::vector<std::uint16_t>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT32:
        {
            std::int32_t v = static_cast<std::int32_t>(value);
            setValue<std::vector<std::int32_t>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_UINT32:
        {
            std::uint32_t v = static_cast<std::uint32_t>(value);
            setValue<std::vector<std::uint32_t>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT64:
        {
            std::int64_t v = static_cast<std::int64_t>(value);
            setValue<std::vector<std::int64_t>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_UINT64:
        {
            std::uint64_t v = static_cast<std::uint64_t>(value);
            setValue<std::vector<std::uint64_t>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_FLOAT:
        {
            float v = static_cast<float>(value);
            setValue<std::vector<float>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_DOUBLE:
        {
            double v = static_cast<double>(value);
            setValue<std::vector<double>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_STRING:
        {
            setValue<std::vector<std::string>>(structBase, fieldInfoDest, {std::to_string(value)});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_BYTES:
        {
            std::string str = std::to_string(value);
            Bytes bytes(str.begin(), str.end());
            setValue<std::vector<Bytes>>(structBase, fieldInfoDest, {bytes});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_ENUM:
        {
            std::int32_t v = static_cast<std::int32_t>(value);
            setValue<std::vector<std::int32_t>>(structBase, fieldInfoDest, {v});
        }
        break;
        default:
            streamError << "number not expected";
            break;
    }
}

void SerializerStruct::convertString(StructBase& structBase, const FieldInfo& fieldInfoDest, const char* value, ssize_t size)
{
    const MetaField* fieldDest = fieldInfoDest.getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    std::string str;
    if (size > 0)
    {
        str = std::string(value, value + size);
    }
    switch(fieldDest->typeId)
    {
        case MetaTypeId::TYPE_BOOL:
        {
            bool v = (str.size() == 4 && (memcmp(str.c_str(), "true", 4) == 0));
            setValue<bool>(structBase, fieldInfoDest, v);
        }
        break;
        case MetaTypeId::TYPE_INT8:
        {
            std::int8_t v = static_cast<std::int8_t>(strtol(str.c_str(), nullptr, 10));
            setValue<std::int8_t>(structBase, fieldInfoDest, v);
        }
        break;
        case MetaTypeId::TYPE_UINT8:
        {
            std::uint8_t v = static_cast<std::uint8_t>(strtoul(str.c_str(), nullptr, 10));
            setValue<std::uint8_t>(structBase, fieldInfoDest, v);
        }
        break;
        case MetaTypeId::TYPE_INT16:
        {
            std::int16_t v = static_cast<std::int16_t>(strtol(str.c_str(), nullptr, 10));
            setValue<std::int16_t>(structBase, fieldInfoDest, v);
        }
        break;
        case MetaTypeId::TYPE_UINT16:
        {
            std::uint16_t v = static_cast<std::uint16_t>(strtoul(str.c_str(), nullptr, 10));
            setValue<std::uint16_t>(structBase, fieldInfoDest, v);
        }
        break;
        case MetaTypeId::TYPE_INT32:
        {
            std::int32_t v = static_cast<std::int32_t>(strtol(str.c_str(), nullptr, 10));
            setValue<std::int32_t>(structBase, fieldInfoDest, v);
        }
        break;
        case MetaTypeId::TYPE_UINT32:
        {
            std::uint32_t v = static_cast<std::uint32_t>(strtoul(str.c_str(), nullptr, 10));
            setValue<std::uint32_t>(structBase, fieldInfoDest, v);
        }
        break;
        case MetaTypeId::TYPE_INT64:
        {
            std::int64_t v = strtoll(str.c_str(), nullptr, 10);
            setValue<std::int64_t>(structBase, fieldInfoDest, v);
        }
        break;
        case MetaTypeId::TYPE_UINT64:
        {
            std::uint64_t v = strtoull(str.c_str(), nullptr, 10);
            setValue<std::uint64_t>(structBase, fieldInfoDest, v);
        }
        break;
        case MetaTypeId::TYPE_FLOAT:
        {
            float v = strtof32(str.c_str(), nullptr);
            setValue<float>(structBase, fieldInfoDest, v);
        }
        break;
        case MetaTypeId::TYPE_DOUBLE:
        {
            double v = strtof64(str.c_str(), nullptr);
            setValue<double>(structBase, fieldInfoDest, v);
        }
        break;
        case MetaTypeId::TYPE_STRING:
            setValue<std::string>(structBase, fieldInfoDest, str);
            break;
        case MetaTypeId::TYPE_BYTES:
        {
            Bytes bytes(str.c_str(), str.c_str() + str.size());
            setValue<Bytes>(structBase, fieldInfoDest, std::move(bytes));
        }
        break;
        case MetaTypeId::TYPE_ENUM:
        {
            std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(*fieldDest, str);
            setValue<std::int32_t>(structBase, fieldInfoDest, v);
        }
        break;
        case MetaTypeId::TYPE_ARRAY_BOOL:
        {
            bool v = (str.size() == 4 && (memcmp(str.c_str(), "true", 4) == 0));
            setValue<std::vector<bool>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT8:
        {
            std::int8_t v = static_cast<std::int8_t>(strtol(str.c_str(), nullptr, 10));
            setValue<std::vector<std::int8_t>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT16:
        {
            std::int16_t v = static_cast<std::int16_t>(strtol(str.c_str(), nullptr, 10));
            setValue<std::vector<std::int16_t>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_UINT16:
        {
            std::uint16_t v = static_cast<std::uint16_t>(strtoul(str.c_str(), nullptr, 10));
            setValue<std::vector<std::uint16_t>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT32:
        {
            std::int32_t v = static_cast<std::int32_t>(strtol(str.c_str(), nullptr, 10));
            setValue<std::vector<std::int32_t>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_UINT32:
        {
            std::uint32_t v = static_cast<std::uint32_t>(strtoul(str.c_str(), nullptr, 10));
            setValue<std::vector<std::uint32_t>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT64:
        {
            std::int64_t v = strtoll(str.c_str(), nullptr, 10);
            setValue<std::vector<std::int64_t>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_UINT64:
        {
            std::uint64_t v = strtoull(str.c_str(), nullptr, 10);
            setValue<std::vector<std::uint64_t>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_FLOAT:
        {
            float v = strtof32(str.c_str(), nullptr);
            setValue<std::vector<float>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_DOUBLE:
        {
            double v = strtof64(str.c_str(), nullptr);
            setValue<std::vector<double>>(structBase, fieldInfoDest, {v});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_STRING:
        {
            setValue<std::vector<std::string>>(structBase, fieldInfoDest, {str});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_BYTES:
        {
            Bytes bytes(str.c_str(), str.c_str() + str.size());
            setValue<std::vector<Bytes>>(structBase, fieldInfoDest, {std::move(bytes)});
        }
        break;
        case MetaTypeId::TYPE_ARRAY_ENUM:
        {
            std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(*fieldDest, str);
            setValue<std::vector<std::int32_t>>(structBase, fieldInfoDest, {v});
        }
        break;
        default:
            streamError << "number not expected";
            break;
    }
}

template<class T>
void SerializerStruct::convertArrayNumber(StructBase& structBase, const FieldInfo& fieldInfoDest, const T* value, ssize_t size)
{
    const MetaField* fieldDest = fieldInfoDest.getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    switch(fieldDest->typeId)
    {
        case MetaTypeId::TYPE_BOOL:
            if (size >= 1)
            {
#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
                setValue<bool>(structBase, fieldInfoDest, static_cast<bool>(value[0]));
#ifndef WIN32
#pragma GCC diagnostic pop
#endif
            }
            break;
        case MetaTypeId::TYPE_INT8:
            if (size >= 1)
            {
                setValue<std::int8_t>(structBase, fieldInfoDest, static_cast<std::int8_t>(value[0]));
            }
            break;
        case MetaTypeId::TYPE_UINT8:
            if (size >= 1)
            {
                setValue<std::uint8_t>(structBase, fieldInfoDest, static_cast<std::uint8_t>(value[0]));
            }
            break;
        case MetaTypeId::TYPE_INT16:
            if (size >= 1)
            {
                setValue<std::int16_t>(structBase, fieldInfoDest, static_cast<std::int16_t>(value[0]));
            }
            break;
        case MetaTypeId::TYPE_UINT16:
            if (size >= 1)
            {
                setValue<std::uint16_t>(structBase, fieldInfoDest, static_cast<std::uint16_t>(value[0]));
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
#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
                v.push_back(entry);
#ifndef WIN32
#pragma GCC diagnostic pop
#endif
            });
            setValue<std::vector<bool>>(structBase, fieldInfoDest, std::move(v));
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT8:
        {
            std::vector<std::int8_t> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v](const T& entry) {
                v.push_back(static_cast<std::int8_t>(entry));
            });
            setValue<std::vector<std::int8_t>>(structBase, fieldInfoDest, std::move(v));
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT16:
        {
            std::vector<std::int16_t> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v](const T& entry) {
                v.push_back(static_cast<std::int16_t>(entry));
            });
            setValue<std::vector<std::int16_t>>(structBase, fieldInfoDest, std::move(v));
        }
        break;
        case MetaTypeId::TYPE_ARRAY_UINT16:
        {
            std::vector<std::uint16_t> v;
            v.reserve(size);
            std::for_each(value, value + size, [&v](const T& entry) {
                v.push_back(static_cast<std::uint16_t>(entry));
            });
            setValue<std::vector<std::uint16_t>>(structBase, fieldInfoDest, std::move(v));
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

void SerializerStruct::convertArrayString(StructBase& structBase, const FieldInfo& fieldInfoDest, const std::vector<std::string>& value)
{
    const size_t size = value.size();
    const MetaField* fieldDest = fieldInfoDest.getField();
    if (fieldDest == nullptr)
    {
        return;
    }
    switch(fieldDest->typeId)
    {
        case MetaTypeId::TYPE_BOOL:
            if (size >= 1)
            {
                bool val = (value[0].size() == 4 && (memcmp(value[0].c_str(), "true", 4) == 0));
                setValue<std::vector<bool>>(structBase, fieldInfoDest, {val});
            }
            break;
        case MetaTypeId::TYPE_INT8:
            if (size >= 1)
            {
                setValue<std::int8_t>(structBase, fieldInfoDest, static_cast<std::int8_t>(strtol(value[0].c_str(), nullptr, 10)));
            }
            break;
        case MetaTypeId::TYPE_UINT8:
            if (size >= 1)
            {
                setValue<std::uint8_t>(structBase, fieldInfoDest, static_cast<std::uint8_t>(strtoul(value[0].c_str(), nullptr, 10)));
            }
            break;
        case MetaTypeId::TYPE_INT16:
            if (size >= 1)
            {
                setValue<std::int16_t>(structBase, fieldInfoDest, static_cast<std::int16_t>(strtol(value[0].c_str(), nullptr, 10)));
            }
            break;
        case MetaTypeId::TYPE_UINT16:
            if (size >= 1)
            {
                setValue<std::uint16_t>(structBase, fieldInfoDest, static_cast<std::uint16_t>(strtoul(value[0].c_str(), nullptr, 10)));
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
                setValue<float>(structBase, fieldInfoDest, strtof32(value[0].c_str(), nullptr));
            }
            break;
        case MetaTypeId::TYPE_DOUBLE:
            if (size >= 1)
            {
                setValue<double>(structBase, fieldInfoDest, strtof64(value[0].c_str(), nullptr));
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
                bool val = (entry.size() == 4 && (memcmp(entry.c_str(), "true", 4) == 0)) || (entry.size() >= 1 && (entry[0] == 1));
                v.push_back(val);
            });
            setValue<std::vector<bool>>(structBase, fieldInfoDest, std::move(v));
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT8:
        {
            std::vector<std::int8_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v](const std::string& entry) {
                v.push_back(static_cast<std::int8_t>(strtol(entry.c_str(), nullptr, 10)));
            });
            setValue<std::vector<std::int8_t>>(structBase, fieldInfoDest, std::move(v));
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT16:
        {
            std::vector<std::int16_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v](const std::string& entry) {
                v.push_back(static_cast<std::int16_t>(strtol(entry.c_str(), nullptr, 10)));
            });
            setValue<std::vector<std::int16_t>>(structBase, fieldInfoDest, std::move(v));
        }
        break;
        case MetaTypeId::TYPE_ARRAY_UINT16:
        {
            std::vector<std::uint16_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v](const std::string& entry) {
                v.push_back(static_cast<std::uint16_t>(strtoul(entry.c_str(), nullptr, 10)));
            });
            setValue<std::vector<std::uint16_t>>(structBase, fieldInfoDest, std::move(v));
        }
        break;
        case MetaTypeId::TYPE_ARRAY_INT32:
        {
            std::vector<std::int32_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v](const std::string& entry) {
                v.push_back(static_cast<std::int32_t>(strtol(entry.c_str(), nullptr, 10)));
            });
            setValue<std::vector<std::int32_t>>(structBase, fieldInfoDest, std::move(v));
        }
        break;
        case MetaTypeId::TYPE_ARRAY_UINT32:
        {
            std::vector<std::uint32_t> v;
            v.reserve(size);
            std::for_each(value.begin(), value.end(), [&v](const std::string& entry) {
                v.push_back(static_cast<std::uint32_t>(strtoul(entry.c_str(), nullptr, 10)));
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
void SerializerStruct::setValueNumber(const MetaField& field, T value)
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

void SerializerStruct::setValueString(const MetaField& field, const char* value, ssize_t size)
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
void SerializerStruct::setValueArrayNumber(const MetaField& field, const T* value, ssize_t size)
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
void SerializerStruct::setValueArrayNumber(const MetaField& field, std::vector<T>&& value)
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

void SerializerStruct::setValueArrayString(const MetaField& field, const std::vector<std::string>& value)
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

void SerializerStruct::setValueArrayString(const MetaField& field, std::vector<std::string>&& value)
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

void SerializerStruct::enterBool(const MetaField& field, bool value)
{
    if (m_visitor)
    {
        m_visitor->enterBool(field, value);
        return;
    }

    setValueNumber<bool>(field, value);
}

void SerializerStruct::enterInt8(const MetaField& field, std::int8_t value)
{
    if (m_visitor)
    {
        m_visitor->enterInt8(field, value);
        return;
    }

    setValueNumber<std::int8_t>(field, value);
}

void SerializerStruct::enterUInt8(const MetaField& field, std::uint8_t value)
{
    if (m_visitor)
    {
        m_visitor->enterUInt8(field, value);
        return;
    }

    setValueNumber<std::uint8_t>(field, value);
}

void SerializerStruct::enterInt16(const MetaField& field, std::int16_t value)
{
    if (m_visitor)
    {
        m_visitor->enterInt16(field, value);
        return;
    }

    setValueNumber<std::int16_t>(field, value);
}

void SerializerStruct::enterUInt16(const MetaField& field, std::uint16_t value)
{
    if (m_visitor)
    {
        m_visitor->enterUInt16(field, value);
        return;
    }

    setValueNumber<std::uint16_t>(field, value);
}

void SerializerStruct::enterInt32(const MetaField& field, std::int32_t value)
{
    if (m_visitor)
    {
        m_visitor->enterInt32(field, value);
        return;
    }

    setValueNumber<std::int32_t>(field, value);
}

void SerializerStruct::enterUInt32(const MetaField& field, std::uint32_t value)
{
    if (m_visitor)
    {
        m_visitor->enterUInt32(field, value);
        return;
    }

    setValueNumber<std::uint32_t>(field, value);
}

void SerializerStruct::enterInt64(const MetaField& field, std::int64_t value)
{
    if (m_visitor)
    {
        m_visitor->enterInt64(field, value);
        return;
    }

    setValueNumber<std::int64_t>(field, value);
}

void SerializerStruct::enterUInt64(const MetaField& field, std::uint64_t value)
{
    if (m_visitor)
    {
        m_visitor->enterUInt64(field, value);
        return;
    }

    setValueNumber<std::uint64_t>(field, value);
}

void SerializerStruct::enterFloat(const MetaField& field, float value)
{
    if (m_visitor)
    {
        m_visitor->enterFloat(field, value);
        return;
    }

    setValueNumber<float>(field, value);
}

void SerializerStruct::enterDouble(const MetaField& field, double value)
{
    if (m_visitor)
    {
        m_visitor->enterDouble(field, value);
        return;
    }

    setValueNumber<double>(field, value);
}

void SerializerStruct::enterString(const MetaField& field, std::string&& value)
{
    if (m_visitor)
    {
        m_visitor->enterString(field, std::move(value));
        return;
    }

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

void SerializerStruct::enterString(const MetaField& field, const char* value, ssize_t size)
{
    if (m_visitor)
    {
        m_visitor->enterString(field, value, size);
        return;
    }

    setValueString(field, value, size);
}

void SerializerStruct::enterBytes(const MetaField& field, Bytes&& value)
{
    if (m_visitor)
    {
        m_visitor->enterBytes(field, std::move(value));
        return;
    }

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

void SerializerStruct::enterBytes(const MetaField& field, const BytesElement* value, ssize_t size)
{
    if (m_visitor)
    {
        m_visitor->enterBytes(field, value, size);
        return;
    }

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
        if (size > 0)
        {
            setValue<Bytes>(*structBase, *fieldInfoDest, Bytes(value, value + size));
        }
        else
        {
            setValue<Bytes>(*structBase, *fieldInfoDest, Bytes());
        }
    }
    else
    {
        convertString(*structBase, *fieldInfoDest, value, size);
    }
}

void SerializerStruct::enterEnum(const MetaField& field, std::int32_t value)
{
    if (m_visitor)
    {
        m_visitor->enterEnum(field, value);
        return;
    }

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

void SerializerStruct::enterEnum(const MetaField& field, std::string&& value)
{
    if (m_visitor)
    {
        m_visitor->enterEnum(field, std::move(value));
        return;
    }

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

void SerializerStruct::enterEnum(const MetaField& field, const char* value, ssize_t size)
{
    if (m_visitor)
    {
        m_visitor->enterEnum(field, value, size);
        return;
    }

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

void SerializerStruct::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayBoolMove(field, std::move(value));
        return;
    }

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

void SerializerStruct::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayBool(field, value);
        return;
    }

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

void SerializerStruct::enterArrayInt8(const MetaField& field, std::vector<std::int8_t>&& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayInt8(field, std::move(value));
        return;
    }

    setValueArrayNumber<std::int8_t>(field, std::move(value));
}

void SerializerStruct::enterArrayInt8(const MetaField& field, const std::int8_t* value, ssize_t size)
{
    if (m_visitor)
    {
        m_visitor->enterArrayInt8(field, value, size);
        return;
    }

    setValueArrayNumber<std::int8_t>(field, value, size);
}

void SerializerStruct::enterArrayInt16(const MetaField& field, std::vector<std::int16_t>&& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayInt16(field, std::move(value));
        return;
    }

    setValueArrayNumber<std::int16_t>(field, std::move(value));
}

void SerializerStruct::enterArrayInt16(const MetaField& field, const std::int16_t* value, ssize_t size)
{
    if (m_visitor)
    {
        m_visitor->enterArrayInt16(field, value, size);
        return;
    }

    setValueArrayNumber<std::int16_t>(field, value, size);
}

void SerializerStruct::enterArrayUInt16(const MetaField& field, std::vector<std::uint16_t>&& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayUInt16(field, std::move(value));
        return;
    }

    setValueArrayNumber<std::uint16_t>(field, std::move(value));
}

void SerializerStruct::enterArrayUInt16(const MetaField& field, const std::uint16_t* value, ssize_t size)
{
    if (m_visitor)
    {
        m_visitor->enterArrayUInt16(field, value, size);
        return;
    }

    setValueArrayNumber<std::uint16_t>(field, value, size);
}

void SerializerStruct::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayInt32(field, std::move(value));
        return;
    }

    setValueArrayNumber<std::int32_t>(field, std::move(value));
}

void SerializerStruct::enterArrayInt32(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    if (m_visitor)
    {
        m_visitor->enterArrayInt32(field, value, size);
        return;
    }

    setValueArrayNumber<std::int32_t>(field, value, size);
}

void SerializerStruct::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayUInt32(field, std::move(value));
        return;
    }

    setValueArrayNumber<std::uint32_t>(field, std::move(value));
}

void SerializerStruct::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, ssize_t size)
{
    if (m_visitor)
    {
        m_visitor->enterArrayUInt32(field, value, size);
        return;
    }

    setValueArrayNumber<std::uint32_t>(field, value, size);
}

void SerializerStruct::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayInt64(field, std::move(value));
        return;
    }

    setValueArrayNumber<std::int64_t>(field, std::move(value));
}

void SerializerStruct::enterArrayInt64(const MetaField& field, const std::int64_t* value, ssize_t size)
{
    if (m_visitor)
    {
        m_visitor->enterArrayInt64(field, value, size);
        return;
    }

    setValueArrayNumber<std::int64_t>(field, value, size);
}

void SerializerStruct::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayUInt64(field, std::move(value));
        return;
    }

    setValueArrayNumber<std::uint64_t>(field, std::move(value));
}

void SerializerStruct::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, ssize_t size)
{
    if (m_visitor)
    {
        m_visitor->enterArrayUInt64(field, value, size);
        return;
    }

    setValueArrayNumber<std::uint64_t>(field, value, size);
}

void SerializerStruct::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayFloat(field, std::move(value));
        return;
    }

    setValueArrayNumber<float>(field, std::move(value));
}

void SerializerStruct::enterArrayFloat(const MetaField& field, const float* value, ssize_t size)
{
    if (m_visitor)
    {
        m_visitor->enterArrayFloat(field, value, size);
        return;
    }

    setValueArrayNumber<float>(field, value, size);
}

void SerializerStruct::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayDouble(field, std::move(value));
        return;
    }

    setValueArrayNumber<double>(field, std::move(value));
}

void SerializerStruct::enterArrayDouble(const MetaField& field, const double* value, ssize_t size)
{
    if (m_visitor)
    {
        m_visitor->enterArrayDouble(field, value, size);
        return;
    }

    setValueArrayNumber<double>(field, value, size);
}

void SerializerStruct::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayStringMove(field, std::move(value));
        return;
    }

    setValueArrayString(field, std::move(value));
}

void SerializerStruct::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayString(field, value);
        return;
    }

    setValueArrayString(field, value);
}

void SerializerStruct::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayBytesMove(field, std::move(value));
        return;
    }

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

void SerializerStruct::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayBytes(field, value);
        return;
    }

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

void SerializerStruct::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayEnum(field, std::move(value));
        return;
    }

    setValueArrayNumber<std::int32_t>(field, std::move(value));
}

void SerializerStruct::enterArrayEnum(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    if (m_visitor)
    {
        m_visitor->enterArrayEnum(field, value, size);
        return;
    }

    setValueArrayNumber<std::int32_t>(field, value, size);
}

void SerializerStruct::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayEnumMove(field, std::move(value));
        return;
    }

    setValueArrayString(field, std::move(value));
}

void SerializerStruct::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
{
    if (m_visitor)
    {
        m_visitor->enterArrayEnum(field, value);
        return;
    }

    setValueArrayString(field, value);
}

} // namespace finalmq
