
#include "metadata/MetaStruct.h"



MetaStruct::MetaStruct()
{

}

MetaStruct::MetaStruct(const std::string& typeName, const std::string& description, const std::vector<MetaField>& fields)
    : m_typeName(typeName)
    , m_description(description)
{
    for (size_t i = 0 ; i < fields.size(); ++i)
    {
        addField(fields[i]);
    }
}

MetaStruct::MetaStruct(const std::string& typeName, const std::string& description, std::vector<MetaField>&& fields)
    : m_typeName(typeName)
    , m_description(description)
{
    for (size_t i = 0 ; i < fields.size(); ++i)
    {
        addField(std::move(fields[i]));
    }
}



void MetaStruct::setTypeName(const std::string& typeName)
{
    m_typeName = typeName;
}

const std::string& MetaStruct::getTypeName() const
{
    return m_typeName;
}

void MetaStruct::setDescription(const std::string& description)
{
    m_description = description;
}

const std::string& MetaStruct::getDescription() const
{
    return m_description;
}


const MetaField* MetaStruct::getFieldByIndex(int index) const
{
    if (index >= 0 && index < static_cast<int>(m_fields.size()))
    {
        return m_fields[index].get();
    }
    return nullptr;
}


const MetaField* MetaStruct::getFieldByName(const std::string& name) const
{
    auto it = m_name2Field.find(name);
    if (it != m_name2Field.end())
    {
        return it->second.get();
    }
    return nullptr;
}


void MetaStruct::addField(const MetaField& field)
{
    addField(MetaField(field));
}


void MetaStruct::addField(MetaField&& field)
{
    if (m_name2Field.find(field.name) != m_name2Field.end())
    {
        // field already added
        return;
    }

    field.index = m_fields.size();

    std::shared_ptr<MetaField> f = std::make_shared<MetaField>(std::move(field));
    m_fields.emplace_back(f);
    m_name2Field.emplace(f->name, f);
}
