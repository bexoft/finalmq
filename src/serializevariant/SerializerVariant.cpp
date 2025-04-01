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


#include "finalmq/serializevariant/SerializerVariant.h"
#include "finalmq/serializevariant/VarValueToVariant.h"
#include "finalmq/serialize/ParserAbortAndIndex.h"
#include "finalmq/serialize/ParserProcessDefaultValues.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValueList.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/metadataserialize/variant.fmq.h"
#include "finalmq/jsonvariant/JsonToVariant.h"

#include <assert.h>
#include <algorithm>


namespace finalmq {


static const std::string STR_VARVALUE = "finalmq.variant.VarValue";


SerializerVariant::SerializerVariant(Variant& root, bool enumAsString, bool skipDefaultValues)
    : ParserConverter()
    , m_internal(*this, root, enumAsString)
    , m_parserProcessDefaultValues()
{
    m_parserAbortAndIndex = std::make_unique<ParserAbortAndIndex>(&m_internal);
    m_parserProcessDefaultValues = std::make_unique<ParserProcessDefaultValues>(skipDefaultValues, m_parserAbortAndIndex.get());
    ParserConverter::setVisitor(*m_parserProcessDefaultValues);
}



SerializerVariant::Internal::Internal(SerializerVariant& outer, Variant& root, bool enumAsString)
    : m_root(root)
    , m_enumAsString(enumAsString)
    , m_outer(outer)
{
}


// IParserVisitor
void SerializerVariant::Internal::notifyError(const char* /*str*/, const char* /*message*/)
{
}

void SerializerVariant::Internal::startStruct(const MetaStruct& /*stru*/)
{
    m_root = Variant();
    m_root = VariantStruct();
    m_stack.push_back(&m_root);
    m_current = m_stack.back();
}

void SerializerVariant::Internal::finished()
{
}


void SerializerVariant::Internal::enterStruct(const MetaField& field)
{
    assert(m_current);

    if (field.typeName == STR_VARVALUE)
    {
        m_varValueToVariant = nullptr;
        m_current->add(field.name, Variant());
        Variant* variant = m_current->getVariant(field.name);
        assert(variant);
        m_varValueToVariant = std::make_shared<VarValueToVariant>(*variant);
        m_outer.ParserConverter::setVisitor(m_varValueToVariant->getVisitor());
        m_outer.m_parserProcessDefaultValues->setVisitor(m_varValueToVariant->getVisitor());
        m_varValueToVariant->setExitNotification([this]() {
            assert(m_varValueToVariant);
            m_outer.ParserConverter::setVisitor(*m_outer.m_parserProcessDefaultValues);
            m_outer.m_parserProcessDefaultValues->resetVarValueActive();
            m_outer.m_parserProcessDefaultValues->setVisitor(*this);
            m_varValueToVariant->convert();
            m_varValueToVariant->setExitNotification(nullptr);
        });
    }
    else
    {
        if (m_current->getType() == TYPE_STRUCT)
        {
            m_current->add(field.name, VariantStruct());
            VariantStruct* stru = *m_current;
            assert(stru);
            m_stack.push_back(&stru->back().second);
        }
        else if (m_current->getType() == VARTYPE_LIST)
        {
            m_current->add(VariantStruct());
            VariantList* list = *m_current;
            assert(list);
            m_stack.push_back(&list->back());
        }
        else
        {
            m_stack.push_back(nullptr);
        }
        m_current = m_stack.back();
    }
}

void SerializerVariant::Internal::exitStruct(const MetaField& /*field*/)
{
    if (!m_stack.empty())
    {
        m_stack.pop_back();
        if (!m_stack.empty())
        {
            m_current = m_stack.back();
        }
        else
        {
            m_current = nullptr;
        }
    }
}

void SerializerVariant::Internal::enterStructNull(const MetaField& field)
{
    assert(m_current);

    if (m_current->getType() == TYPE_STRUCT)
    {
        m_current->add(field.name, Variant());
    }
    else if (m_current->getType() == VARTYPE_LIST)
    {
        m_current->add(Variant());
    }
}


void SerializerVariant::Internal::enterArrayStruct(const MetaField& field)
{
    if (!m_stack.empty())
    {
        assert(m_current);
        if (m_current->getType() == TYPE_STRUCT)
        {
            m_current->add(field.name, VariantList());
            VariantStruct* stru = *m_current;
            assert(stru);
            m_stack.push_back(&stru->back().second);
        }
        else
        {
            assert(m_current->getType() == VARTYPE_LIST);
            m_current->add(VariantList());
            VariantList* list = *m_current;
            assert(list);
            m_stack.push_back(&list->back());
        }
        m_current = m_stack.back();
    }
}

void SerializerVariant::Internal::exitArrayStruct(const MetaField& /*field*/)
{
    if (!m_stack.empty())
    {
        m_stack.pop_back();
        if (!m_stack.empty())
        {
            m_current = m_stack.back();
        }
        else
        {
            m_current = nullptr;
        }
    }
}


template <class T>
void SerializerVariant::Internal::add(const MetaField& field, const T& value)
{
    if (m_current)
    {
        if (m_current->getType() == TYPE_STRUCT)
        {
            m_current->add(field.name, value);
        }
        else
        {
            assert(m_current->getType() == VARTYPE_LIST);
            m_current->add(value);
        }
    }
}

template <class T>
void SerializerVariant::Internal::add(const MetaField& field, T&& value)
{
    if (m_current)
    {
        if (m_current->getType() == TYPE_STRUCT)
        {
            m_current->add(field.name, std::move(value));
        }
        else
        {
            assert(m_current->getType() == VARTYPE_LIST);
            m_current->add(std::move(value));
        }
    }
}


void SerializerVariant::Internal::enterBool(const MetaField& field, bool value)
{
    add(field, value);
}

void SerializerVariant::Internal::enterInt8(const MetaField& field, std::int8_t value)
{
    add(field, value);
}

void SerializerVariant::Internal::enterUInt8(const MetaField& field, std::uint8_t value)
{
    add(field, value);
}

void SerializerVariant::Internal::enterInt16(const MetaField& field, std::int16_t value)
{
    add(field, value);
}

void SerializerVariant::Internal::enterUInt16(const MetaField& field, std::uint16_t value)
{
    add(field, value);
}

void SerializerVariant::Internal::enterInt32(const MetaField& field, std::int32_t value)
{
    add(field, value);
}

void SerializerVariant::Internal::enterUInt32(const MetaField& field, std::uint32_t value)
{
    add(field, value);
}

void SerializerVariant::Internal::enterInt64(const MetaField& field, std::int64_t value)
{
    add(field, value);
}

void SerializerVariant::Internal::enterUInt64(const MetaField& field, std::uint64_t value)
{
    add(field, value);
}

void SerializerVariant::Internal::enterFloat(const MetaField& field, float value)
{
    add(field, value);
}

void SerializerVariant::Internal::enterDouble(const MetaField& field, double value)
{
    add(field, value);
}

void SerializerVariant::Internal::enterString(const MetaField& field, std::string&& value)
{
    add(field, std::move(value));
}

void SerializerVariant::Internal::enterString(const MetaField& field, const char* value, ssize_t size)
{
    add(field, std::string(value, size));
}

void SerializerVariant::Internal::enterBytes(const MetaField& field, Bytes&& value)
{
    add(field, std::move(value));
}

void SerializerVariant::Internal::enterBytes(const MetaField& field, const BytesElement* value, ssize_t size)
{
    add(field, Bytes(value, value + size));
}

void SerializerVariant::Internal::enterEnum(const MetaField& field, std::int32_t value)
{
    if (m_enumAsString)
    {
        const std::string& name = MetaDataGlobal::instance().getEnumAliasByValue(field, value);
        add(field, name);
    }
    else
    {
        add(field, value);
    }
}


void SerializerVariant::Internal::enterEnum(const MetaField& field, std::string&& value)
{
    if (m_enumAsString)
    {
        add(field, std::move(value));
    }
    else
    {
        std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(field, value);
        add(field, v);
    }
}

void SerializerVariant::Internal::enterEnum(const MetaField& field, const char* value, ssize_t size)
{
    if (m_enumAsString)
    {
        add(field, std::string(value, size));
    }
    else
    {
        std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(field, std::string(value, size));
        add(field, v);
    }
}


void SerializerVariant::Internal::enterJsonString(const MetaField& field, std::string&& value)
{
    enterJsonString(field, value.c_str(), value.size());
}
void SerializerVariant::Internal::enterJsonString(const MetaField& field, const char* value, ssize_t size)
{
    Variant variant;
    if (field.typeId == MetaTypeId::TYPE_JSON)
    {
        JsonToVariant jsonToVariant(variant);
        jsonToVariant.parse(value, size);
    }
    add(field, std::move(variant));
}
void SerializerVariant::Internal::enterJsonVariant(const MetaField& field, const Variant& value)
{
    add(field, value);
}
void SerializerVariant::Internal::enterJsonVariantMove(const MetaField& field, Variant&& value)
{
    add(field, std::move(value));
}


void SerializerVariant::Internal::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
    add(field, std::move(value));
}

void SerializerVariant::Internal::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
{
    add(field, value);
}

void SerializerVariant::Internal::enterArrayInt8(const MetaField& field, std::vector<std::int8_t>&& value)
{
    add(field, std::move(value));
}

void SerializerVariant::Internal::enterArrayInt8(const MetaField& field, const std::int8_t* value, ssize_t size)
{
    add(field, std::vector<std::int8_t>(value, value + size));
}

void SerializerVariant::Internal::enterArrayInt16(const MetaField& field, std::vector<std::int16_t>&& value)
{
    add(field, std::move(value));
}

void SerializerVariant::Internal::enterArrayInt16(const MetaField& field, const std::int16_t* value, ssize_t size)
{
    add(field, std::vector<std::int16_t>(value, value + size));
}

void SerializerVariant::Internal::enterArrayUInt16(const MetaField& field, std::vector<std::uint16_t>&& value)
{
    add(field, std::move(value));
}

void SerializerVariant::Internal::enterArrayUInt16(const MetaField& field, const std::uint16_t* value, ssize_t size)
{
    add(field, std::vector<std::uint16_t>(value, value + size));
}

void SerializerVariant::Internal::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
{
    add(field, std::move(value));
}

void SerializerVariant::Internal::enterArrayInt32(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    add(field, std::vector<std::int32_t>(value, value + size));
}

void SerializerVariant::Internal::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
{
    add(field, std::move(value));
}

void SerializerVariant::Internal::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, ssize_t size)
{
    add(field, std::vector<std::uint32_t>(value, value + size));
}

void SerializerVariant::Internal::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
{
    add(field, std::move(value));
}

void SerializerVariant::Internal::enterArrayInt64(const MetaField& field, const std::int64_t* value, ssize_t size)
{
    add(field, std::vector<std::int64_t>(value, value + size));
}

void SerializerVariant::Internal::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
{
    add(field, std::move(value));
}

void SerializerVariant::Internal::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, ssize_t size)
{
    add(field, std::vector<std::uint64_t>(value, value + size));
}

void SerializerVariant::Internal::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
{
    add(field, std::move(value));
}

void SerializerVariant::Internal::enterArrayFloat(const MetaField& field, const float* value, ssize_t size)
{
    add(field, std::vector<float>(value, value + size));
}

void SerializerVariant::Internal::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
{
    add(field, std::move(value));
}

void SerializerVariant::Internal::enterArrayDouble(const MetaField& field, const double* value, ssize_t size)
{
    add(field, std::vector<double>(value, value + size));
}

void SerializerVariant::Internal::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
{
    add(field, std::move(value));
}

void SerializerVariant::Internal::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
{
    add(field, value);
}

void SerializerVariant::Internal::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
{
    add(field, std::move(value));
}

void SerializerVariant::Internal::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
    add(field, value);
}

void SerializerVariant::Internal::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_ENUM);

    if (m_enumAsString)
    {
        std::vector<std::string> enums;
        enums.reserve(value.size());
        std::for_each(value.begin(), value.end(), [&field, &enums] (std::int32_t entry) {
            const std::string& name = MetaDataGlobal::instance().getEnumAliasByValue(field, entry);
            enums.push_back(name);
        });
        add(field, std::move(enums));
    }
    else
    {
        add(field, std::move(value));
    }
}

void SerializerVariant::Internal::enterArrayEnum(const MetaField& field, const std::int32_t* value, ssize_t size)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_ENUM);

    if (m_enumAsString)
    {
        std::vector<std::string> enums;
        enums.reserve(size);
        std::for_each(value, value + size, [&field, &enums] (std::int32_t entry) {
            const std::string& name = MetaDataGlobal::instance().getEnumAliasByValue(field, entry);
            enums.push_back(name);
        });
        add(field, std::move(enums));
    }
    else
    {
        add(field, std::vector<std::int32_t>(value, value + size));
    }
}

void SerializerVariant::Internal::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
{
    if (m_enumAsString)
    {
        add(field, std::move(value));
    }
    else
    {
        std::vector<std::int32_t> enums;
        enums.reserve(value.size());
        std::for_each(value.begin(), value.end(), [&field, &enums] (const std::string& entry) {
            std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(field, entry);
            enums.push_back(v);
        });
        add(field, std::move(enums));
    }
}

void SerializerVariant::Internal::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
{
    assert(field.typeId == MetaTypeId::TYPE_ARRAY_ENUM);

    if (m_enumAsString)
    {
        add(field, value);
    }
    else
    {
        std::vector<std::int32_t> enums;
        enums.reserve(value.size());
        std::for_each(value.begin(), value.end(), [&field, &enums] (const std::string& entry) {
            std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(field, entry);
            enums.push_back(v);
        });
        add(field, std::move(enums));
    }
}

}   // namespace finalmq
