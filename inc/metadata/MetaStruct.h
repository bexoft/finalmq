#pragma once

#include "MetaField.h"

#include <vector>
#include <list>
#include <unordered_map>
#include <memory>

class MetaStruct
{
public:
    MetaStruct();
    MetaStruct(const std::string& typeName, const std::string& description, const std::vector<MetaField>& fields);
    MetaStruct(const std::string& typeName, const std::string& description, std::vector<MetaField>&& fields);

    void setTypeName(const std::string& typeName);
    const std::string& getTypeName() const;
    void setDescription(const std::string& description);
    const std::string& getDescription() const;

    const MetaField* getFieldByIndex(int index) const;
    const MetaField* getFieldByName(const std::string& name) const;

    void addField(const MetaField& field);
    void addField(MetaField&& field);

    int getFieldsSize() const
    {
        return m_fields.size();
    }

private:
    std::string                                                         m_typeName;
    std::string                                                         m_description;
    std::vector<std::shared_ptr<const MetaField>>                       m_fields;
    std::unordered_map<std::string, std::shared_ptr<const MetaField>>   m_name2Field;
};
