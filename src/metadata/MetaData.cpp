
#include "metadata/MetaData.h"

#include <assert.h>
#include <iostream>


// IMetaData

const MetaStruct* MetaData::getStruct(const std::string& typeName) const
{
    auto it = m_name2Struct.find(typeName);
    if (it != m_name2Struct.end())
    {
        return &it->second;
    }
    return nullptr;
}


const MetaEnum* MetaData::getEnum(const std::string& typeName) const
{
    auto it = m_name2Enum.find(typeName);
    if (it != m_name2Enum.end())
    {
        return &it->second;
    }
    return nullptr;
}


const MetaStruct* MetaData::getStruct(const MetaField& field) const
{
    assert(field.typeId == MetaTypeId::TYPE_STRUCT || field.typeId == MetaTypeId::TYPE_ARRAY_STRUCT);
    if (!field.metaStruct)
    {
        const MetaStruct* stru = getStruct(field.typeName);
        field.metaStruct = stru;
        if (!field.metaStruct)
        {
            // struct not found
            std::cout << "struct not found: " << field.typeName << std::endl;
        }
    }
    return field.metaStruct;
}


const MetaField* MetaData::getField(const std::string& typeName, const std::string& fieldName) const
{
    const MetaStruct* stru = getStruct(typeName);
    if (stru)
    {
        const MetaField* fieldValue = stru->getFieldByName(fieldName);
        return fieldValue;
    }
    return nullptr;
}



const MetaEnum* MetaData::getEnum(const MetaField& field) const
{
    assert(field.typeId == MetaTypeId::TYPE_ENUM || field.typeId == MetaTypeId::TYPE_ARRAY_ENUM);
    if (!field.metaEnum)
    {
        const MetaEnum* en = getEnum(field.typeName);
        field.metaEnum = en;
        if (!field.metaEnum)
        {
            // enum not found
            std::cout << "enum not found: " << field.typeName << std::endl;
        }
    }
    return field.metaEnum;
}



const MetaField* MetaData::getArrayField(const MetaField& field) const
{

    assert((int)field.typeId & (int)MetaTypeId::TYPE_ARRAY_FLAG);
    if (!field.fieldWithoutArray)
    {
        field.fieldWithoutArray = std::make_shared<MetaField>(field);
        field.fieldWithoutArray->typeId = (MetaTypeId)((int)field.typeId & ~(int)MetaTypeId::TYPE_ARRAY_FLAG);
        field.fieldWithoutArray->name.clear();
    }
    return field.fieldWithoutArray.get();
}


const MetaField* MetaData::getArrayField(const std::string& typeName, const std::string& fieldName) const
{
    const MetaField* field = getField(typeName, fieldName);
    if (field)
    {
        return getArrayField(*field);
    }
    return nullptr;
}

bool MetaData::isEnumValue(const MetaField& field, std::int32_t value) const
{
    const MetaEnum* en = getEnum(field);
    if (en)
    {
        return en->isId(value);
    }
    return false;
}

std::int32_t MetaData::getEnumValueByName(const MetaField& field, const std::string& name) const
{
    std::int32_t value = 0;
    const MetaEnum* en = getEnum(field);
    if (en)
    {
        value = en->getValueByName(name);
    }
    return value;
}

const std::string& MetaData::getEnumNameByValue(const MetaField& field, std::int32_t value) const
{
    const MetaEnum* en = getEnum(field);
    if (en)
    {
        return en->getNameByValue(value);
    }
    static std::string empty;
    return empty;
}



const MetaStruct& MetaData::addStruct(const MetaStruct& stru)
{
    return addStruct(MetaStruct(stru));
}

const MetaStruct& MetaData::addStruct(MetaStruct&& stru)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    auto it = m_name2Struct.find(stru.getTypeName());
    if (it != m_name2Struct.end())
    {
        // struct already added
        return it->second;
    }

    std::string typeName = stru.getTypeName();
    auto result = m_name2Struct.emplace(typeName, std::move(stru));
    assert(result.second);
    return result.first->second;
}



const MetaEnum& MetaData::addEnum(const MetaEnum& en)
{
    return addEnum(MetaEnum(en));
}

const MetaEnum& MetaData::addEnum(MetaEnum&& en)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    auto it = m_name2Enum.find(en.getTypeName());
    if (it != m_name2Enum.end())
    {
        // enum already added
        return it->second;
    }

    auto result = m_name2Enum.emplace(en.getTypeName(), std::move(en));
    assert(result.second);
    return result.first->second;
}


////////////////////////////////

std::unique_ptr<IMetaData> MetaDataGlobal::m_instance;

IMetaData& MetaDataGlobal::instance()
{
    if (!m_instance)
    {
        m_instance = std::make_unique<MetaData>();
    }
    assert(m_instance);
    return *m_instance.get();
}

void MetaDataGlobal::setInstance(std::unique_ptr<IMetaData>&& instance)
{
    m_instance = std::move(instance);
}

