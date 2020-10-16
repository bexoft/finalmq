
#include "serializestruct/SerializerStruct.h"
#include "serialize/ParserProcessDefaultValues.h"
#include "metadata/MetaData.h"
#include "variant/VariantValueStruct.h"
#include "variant/VariantValueList.h"
#include "variant/VariantValues.h"

#include <assert.h>
#include <algorithm>




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
void SerializerStruct::Internal::notifyError(const char* str, const char* message)
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
        m_stack.push_back(&m_root);
        m_current = &m_root;
    }
    else
    {
        assert(m_current);
        StructBase* sub = m_current->getValue<StructBase>(field.index, field.typeId);
        if (sub)
        {
            m_stack.push_back(sub);
            m_current = sub;
        }
    }
}

void SerializerStruct::Internal::exitStruct(const MetaField& field)
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


void SerializerStruct::Internal::enterArrayStruct(const MetaField& field)
{
//    if (!m_stack.empty())
//    {
//        assert(m_current);
//        if (m_current->getType() == TYPE_STRUCT)
//        {
//            m_current->add(field.name, VariantList());
//            VariantStruct* stru = *m_current;
//            assert(stru);
//            m_stack.push_back(&stru->back().second);
//        }
//        else
//        {
//            assert(m_current->getType() == TYPE_ARRAY_STRUCT);
//            m_current->add(VariantList());
//            VariantList* list = *m_current;
//            assert(list);
//            m_stack.push_back(&list->back());
//        }
//        m_current = m_stack.back();
//    }
}

void SerializerStruct::Internal::exitArrayStruct(const MetaField& field)
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
void SerializerStruct::Internal::setValue(const MetaField& field, const T& value)
{
    if (m_current)
    {
        T* pval = m_current->getValue<T>(field.index, field.typeId);
        if (pval)
        {
            *pval = value;
        }
    }
}

template <class T>
void SerializerStruct::Internal::setValue(const MetaField& field, T&& value)
{
    if (m_current)
    {
        T* pval = m_current->getValue<T>(field.index, field.typeId);
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
//    if (m_enumAsString)
//    {
//        add(field, std::string(value, size));
//    }
//    else
//    {
//        std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(field, std::string(value, size));
//        add(field, v);
//    }
}

void SerializerStruct::Internal::enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayBool(const MetaField& field, const std::vector<bool>& value)
{
//    add(field, value);
}

void SerializerStruct::Internal::enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayInt32(const MetaField& field, const std::int32_t* value, int size)
{
//    add(field, std::vector<std::int32_t>(value, value + size));
}

void SerializerStruct::Internal::enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayUInt32(const MetaField& field, const std::uint32_t* value, int size)
{
//    add(field, std::vector<std::uint32_t>(value, value + size));
}

void SerializerStruct::Internal::enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayInt64(const MetaField& field, const std::int64_t* value, int size)
{
//    add(field, std::vector<std::int64_t>(value, value + size));
}

void SerializerStruct::Internal::enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayUInt64(const MetaField& field, const std::uint64_t* value, int size)
{
//    add(field, std::vector<std::uint64_t>(value, value + size));
}

void SerializerStruct::Internal::enterArrayFloat(const MetaField& field, std::vector<float>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayFloat(const MetaField& field, const float* value, int size)
{
//    add(field, std::vector<float>(value, value + size));
}

void SerializerStruct::Internal::enterArrayDouble(const MetaField& field, std::vector<double>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayDouble(const MetaField& field, const double* value, int size)
{
//    add(field, std::vector<double>(value, value + size));
}

void SerializerStruct::Internal::enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayString(const MetaField& field, const std::vector<std::string>& value)
{
//    add(field, value);
}

void SerializerStruct::Internal::enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value)
{
    setValue(field, std::move(value));
}

void SerializerStruct::Internal::enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value)
{
//    add(field, value);
}

void SerializerStruct::Internal::enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value)
{
//    assert(field.typeId == MetaTypeId::TYPE_ARRAY_ENUM);

//    if (m_enumAsString)
//    {
//        std::vector<std::string> enums;
//        enums.reserve(value.size());
//        std::for_each(value.begin(), value.end(), [this, &field, &enums] (std::int32_t entry) {
//            const std::string& name = MetaDataGlobal::instance().getEnumNameByValue(field, entry);
//            enums.push_back(name);
//        });
//        add(field, std::move(enums));
//    }
//    else
//    {
//        add(field, std::move(value));
//    }
}

void SerializerStruct::Internal::enterArrayEnum(const MetaField& field, const std::int32_t* value, int size)
{
//    assert(field.typeId == MetaTypeId::TYPE_ARRAY_ENUM);

//    if (m_enumAsString)
//    {
//        std::vector<std::string> enums;
//        enums.reserve(size);
//        std::for_each(value, value + size, [this, &field, &enums] (std::int32_t entry) {
//            const std::string& name = MetaDataGlobal::instance().getEnumNameByValue(field, entry);
//            enums.push_back(name);
//        });
//        add(field, std::move(enums));
//    }
//    else
//    {
//        add(field, std::vector<std::int32_t>(value, value + size));
//    }
}

void SerializerStruct::Internal::enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value)
{
//    if (m_enumAsString)
//    {
//        add(field, std::move(value));
//    }
//    else
//    {
//        std::vector<std::int32_t> enums;
//        enums.reserve(value.size());
//        std::for_each(value.begin(), value.end(), [this, &field, &enums] (const std::string& entry) {
//            std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(field, entry);
//            enums.push_back(v);
//        });
//    }
}

void SerializerStruct::Internal::enterArrayEnum(const MetaField& field, const std::vector<std::string>& value)
{
//    assert(field.typeId == MetaTypeId::TYPE_ARRAY_ENUM);

//    if (m_enumAsString)
//    {
//        add(field, value);
//    }
//    else
//    {
//        std::vector<std::int32_t> enums;
//        enums.reserve(value.size());
//        std::for_each(value.begin(), value.end(), [this, &field, &enums] (const std::string& entry) {
//            std::int32_t v = MetaDataGlobal::instance().getEnumValueByName(field, entry);
//            enums.push_back(v);
//        });
//    }
}
