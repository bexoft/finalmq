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


#include "finalmq/metadata/MetaStruct.h"

#include <unordered_map>


namespace finalmq {

static std::string removeNamespace(const std::string& typeName)
{
    size_t pos = typeName.find_last_of('.') + 1;
    return typeName.substr(pos, typeName.size() - pos);
}


MetaStruct::MetaStruct()
    : m_flags(0)
{

}

MetaStruct::MetaStruct(const std::string& typeName, const std::string& description, const std::vector<MetaField>& fields, int flags, const std::vector<std::string>& attrs)
    : m_typeName(typeName)
    , m_typeNameWithoutNamespace(removeNamespace(typeName))
    , m_description(description)
    , m_flags(flags)
    , m_attrs(attrs)
    , m_properties(generateProperties(attrs))
{
    for (size_t i = 0 ; i < fields.size(); ++i)
    {
        addField(fields[i]);
    }
}


const std::string& MetaStruct::getTypeName() const
{
    return m_typeName;
}

const std::string& MetaStruct::getTypeNameWithoutNamespace() const
{
    return m_typeNameWithoutNamespace;
}

const std::string& MetaStruct::getDescription() const
{
    return m_description;
}

int MetaStruct::getFlags() const
{
    return m_flags;
}

const std::vector<std::string>& MetaStruct::getAttributes() const
{
    return m_attrs;
}

const std::unordered_map<std::string, std::string>& MetaStruct::getProperties() const
{
    return m_properties;
}


const MetaField* MetaStruct::getFieldByIndex(ssize_t index) const
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
    if (m_name2Field.find(field.name) != m_name2Field.end())
    {
        // field already added
        return;
    }

    std::shared_ptr<MetaField> f = std::make_shared<MetaField>(MetaField(field.typeId, field.typeName, field.name, 
        field.description, field.flags, field.attrs, static_cast<int>(m_fields.size())));

    m_fields.emplace_back(f);
    m_name2Field.emplace(f->name, f);
}


std::unordered_map<std::string, std::string> MetaStruct::generateProperties(const std::vector<std::string>& attrs)
{
    std::unordered_map<std::string, std::string> properties;

    for (size_t i = 0; i < attrs.size(); ++i)
    {
        const std::string& attr = attrs[i];
        std::vector<std::string> props;
        Utils::split(attr, 0, attr.size(), ',', props);
        for (size_t n = 0; n < props.size(); ++n)
        {
            const std::string& prop = props[n];
            size_t ix = prop.find_first_of(':');
            if (ix != std::string::npos)
            {
                std::string key = prop.substr(0, ix);
                std::string value = prop.substr(ix + 1, prop.size() - ix - 1);
                properties[key] = std::move(value);
            }
            else
            {
                properties[prop] = std::string();
            }
        }
    }

    return properties;
}

}   // namespace finalmq
