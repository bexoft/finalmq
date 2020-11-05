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


#include "metadata/MetaData.h"
#include "logger/LogStream.h"
#include "helpers/ModulenameFinalmq.h"

#include <assert.h>
#include <iostream>


namespace finalmq {


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
            streamError << "struct not found: " << field.typeName;
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
            streamError << "enum not found: " << field.typeName;
        }
    }
    return field.metaEnum;
}



const MetaField* MetaData::getArrayField(const MetaField& field) const
{

    assert((int)field.typeId & (int)MetaTypeId::OFFSET_ARRAY_FLAG);
    if (!field.fieldWithoutArray)
    {
        field.fieldWithoutArray = std::make_shared<MetaField>(field);
        field.fieldWithoutArray->typeId = (MetaTypeId)((int)field.typeId & ~(int)MetaTypeId::OFFSET_ARRAY_FLAG);
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


const std::unordered_map<std::string, MetaStruct>& MetaData::getAllStructs() const
{
    return m_name2Struct;
}

const std::unordered_map<std::string, MetaEnum>& MetaData::getAllEnums() const
{
    return m_name2Enum;
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

}   // namespace finalmq
