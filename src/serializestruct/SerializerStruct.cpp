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


#include "serializestruct/SerializerStruct.h"
#include "serialize/ParserProcessDefaultValues.h"
#include "metadata/MetaData.h"
#include "variant/VariantValueStruct.h"
#include "variant/VariantValueList.h"
#include "variant/VariantValues.h"

#include <assert.h>
#include <algorithm>


namespace finalmq {


SerializerStruct::SerializerStruct(StructBase& root)
    : ParserConverter()
    , m_internal(root)
{
    setVisitor(m_internal);
}



SerializerStruct::Internal::Internal(StructBase& root)
    : m_root(root)
{
}


// IParserVisitor
void SerializerStruct::Internal::notifyError(const char* /*str*/, const char* /*message*/)
{
}

void SerializerStruct::Internal::finished()
{
}


void SerializerStruct::Internal::enterStruct(const MetaField& field)
{
    if (m_stack.empty())
    {
        m_root.clear();
        m_stack.push_back({&m_root, -1});
        m_current = &m_stack.back();
    }
    else
    {
        assert(m_current);
        StructBase* sub = nullptr;
        if (m_current->structBase)
        {
            if (m_current->structArrayIndex == -1)
            {
                sub = m_current->structBase->getData<StructBase>(field.index, field.typeId);
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
void SerializerStruct::Internal::setValue(const MetaField& field, const T& value)
{
    assert(m_current);
    if (m_current->structBase)
    {
        T* pval = m_current->structBase->getData<T>(field.index, field.typeId);
        if (pval)
        {
            *pval = value;
        }
    }
}

template <class T>
void SerializerStruct::Internal::setValue(const MetaField& field, T&& value)
{
    assert(m_current);
    if (m_current->structBase)
    {
        T* pval = m_current->structBase->getData<T>(field.index, field.typeId);
        if (pval)
        {
            *pval = std::move(value);
        }
    }
}


void SerializerStruct::Internal::enterBool(const MetaField& field, bool value)
{
    setValue<bool>(field, value);
}

void SerializerStruct::Internal::enterInt32(const MetaField& field, std::int32_t value)
{
    setValue<std::int32_t>(field, value);
}

void SerializerStruct::Internal::enterUInt32(const MetaField& field, std::uint32_t value)
{
    setValue<std::uint32_t>(field, value);
}

void SerializerStruct::Internal::enterInt64(const MetaField& field, std::int64_t value)
{
    setValue<std::int64_t>(field, value);
}

void SerializerStruct::Internal::enterUInt64(const MetaField& field, std::uint64_t value)
{
    setValue<std::uint64_t>(field, value);
}

void SerializerStruct::Internal::enterFloat(const MetaField& field, float value)
{
    setValue<float>(field, value);
}

void SerializerStruct::Internal::enterDouble(const MetaField& field, double value)
{
    setValue<double>(field, value);
}

void SerializerStruct::Internal::enterString(const MetaField& field, std::string&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterString(const MetaField& field, const char* value, int size)
{
    setValue(field, std::string(value, size));
}

void SerializerStruct::Internal::enterBytes(const MetaField& field, Bytes&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterBytes(const MetaField& field, const BytesElement* value, int size)
{
    setValue(field, Bytes(value, value + size));
}

void SerializerStruct::Internal::enterEnum(const MetaField& field, std::int32_t value)
{
    bool validEnum = MetaDataGlobal::instance().isEnumValue(field, value);
    setValue<std::int32_t>(field, validEnum ? value : 0);
}


void SerializerStruct::Internal::enterEnum(const MetaField& field, std::string&& value)
{
    std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(field, value);
    setValue<std::int32_t>(field, v);
}

void SerializerStruct::Internal::enterEnum(const MetaField& field, const char* value, int size)
{
    std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(field, std::string(value, size));
    setValue<std::int32_t>(field, v);
}

void SerializerStruct::Internal::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
{
    setValue(field, value);
}

void SerializerStruct::Internal::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayInt32(const MetaField& field, const std::int32_t* value, int size)
{
    setValue(field, std::vector<std::int32_t>(value, value + size));
}

void SerializerStruct::Internal::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, int size)
{
    setValue(field, std::vector<std::uint32_t>(value, value + size));
}

void SerializerStruct::Internal::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayInt64(const MetaField& field, const std::int64_t* value, int size)
{
    setValue(field, std::vector<std::int64_t>(value, value + size));
}

void SerializerStruct::Internal::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, int size)
{
    setValue(field, std::vector<std::uint64_t>(value, value + size));
}

void SerializerStruct::Internal::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayFloat(const MetaField& field, const float* value, int size)
{
    setValue(field, std::vector<float>(value, value + size));
}

void SerializerStruct::Internal::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayDouble(const MetaField& field, const double* value, int size)
{
    setValue(field, std::vector<double>(value, value + size));
}

void SerializerStruct::Internal::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
{
    setValue(field, value);
}

void SerializerStruct::Internal::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
    setValue(field, value);
}

void SerializerStruct::Internal::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
    enterArrayEnum(field, value.data(), value.size());
}

void SerializerStruct::Internal::enterArrayEnum(const MetaField& field, const std::int32_t* value, int size)
{
    std::vector<std::int32_t> enums;
    enums.reserve(size);

    std::for_each(value, value + size, [this, &field, &enums] (const std::int32_t& entry) {
        bool validEnum = MetaDataGlobal::instance().isEnumValue(field, entry);
        enums.push_back(validEnum ? entry : 0);
    });
    setValue(field, std::move(enums));
}

void SerializerStruct::Internal::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
{
    enterArrayEnum(field, value);
}

void SerializerStruct::Internal::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
{
    std::vector<std::int32_t> enums;
    enums.reserve(value.size());
    std::for_each(value.begin(), value.end(), [this, &field, &enums] (const std::string& entry) {
        std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(field, entry);
        enums.push_back(v);
    });
    setValue(field, std::move(enums));
}

} // namespace finalmq
