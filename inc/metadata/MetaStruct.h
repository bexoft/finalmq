#pragma once

#include "MetaField.h"

#include <vector>
#include <list>
#include <unordered_map>
#include <memory>

class MetaStruct
{
public:
    void setTypeName(const std::string& typeName);
    const std::string& getTypeName() const;

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
    std::vector<std::shared_ptr<const MetaField>>                       m_fields;
    std::unordered_map<std::string, std::shared_ptr<const MetaField>>   m_name2Field;
};
