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


#include "finalmq/metadata/MetaEnum.h"
#include "finalmq/helpers/Utils.h"

namespace finalmq {


MetaEnum::MetaEnum()
{

}

MetaEnum::MetaEnum(const std::string& typeName, const std::string& description, const std::vector<std::string>& attrs, const std::vector<MetaEnumEntry>& entries)
    : m_typeName(typeName)
    , m_description(description)
    , m_attrs(attrs)
    , m_properties(generateProperties(attrs))
{
    for (size_t i = 0 ; i < entries.size(); ++i)
    {
        addEntry(entries[i]);
    }
}

MetaEnum::MetaEnum(const std::string& typeName, const std::string& description, const std::vector<std::string>& attrs, std::vector<MetaEnumEntry>&& entries)
    : m_typeName(typeName)
    , m_description(description)
    , m_attrs(attrs)
    , m_properties(generateProperties(attrs))
{
    for (size_t i = 0 ; i < entries.size(); ++i)
    {
        addEntry(std::move(entries[i]));
    }
}



const std::string& MetaEnum::getTypeName() const
{
    return m_typeName;
}

void MetaEnum::setDescription(const std::string& description)
{
    m_description = description;
}
const std::string& MetaEnum::getDescription() const
{
    return m_description;
}

const std::vector<std::string>& MetaEnum::getAttributes() const
{
    return m_attrs;
}

const std::unordered_map<std::string, std::string>& MetaEnum::getProperties() const
{
    return m_properties;
}

const MetaEnumEntry* MetaEnum::getEntryById(int id) const
{
    auto it = m_id2Entry.find(id);
    if (it != m_id2Entry.end())
    {
        return it->second.get();
    }
    return nullptr;
}


const MetaEnumEntry* MetaEnum::getEntryByName(const std::string& name) const
{
    auto it1 = m_name2Entry.find(name);
    if (it1 != m_name2Entry.end())
    {
        return it1->second.get();
    }
    auto it2 = m_alias2Entry.find(name);
    if (it2 != m_alias2Entry.end())
    {
        return it2->second.get();
    }
    return nullptr;
}



const MetaEnumEntry* MetaEnum::getEntryByIndex(ssize_t index) const
{
    if (index < static_cast<ssize_t>(m_entries.size()))
    {
        return m_entries[index].get();
    }
    return nullptr;
}


bool MetaEnum::isId(int id) const
{
    auto it = m_id2Entry.find(id);
    return (it != m_id2Entry.end());
}


std::int32_t MetaEnum::getValueByName(const std::string& name) const
{
    std::int32_t value = 0;
    const MetaEnumEntry* entry = MetaEnum::getEntryByName(name);
    if (entry)
    {
        value = entry->id;
    }
    return value;
}



const std::string& MetaEnum::getNameByValue(std::int32_t value) const
{
    const MetaEnumEntry* entry = MetaEnum::getEntryById(value);
    if (entry)
    {
        return entry->name;
    }
    else
    {
        entry = MetaEnum::getEntryById(0);
        if (entry)
        {
            return entry->name;
        }
    }
    static std::string empty;   
    return empty;
}

const std::string& MetaEnum::getAliasByValue(std::int32_t value) const
{
    static const std::string aliasEmpty = "$empty";
    static const std::string empty = "";
    const MetaEnumEntry* entry = MetaEnum::getEntryById(value);
    if (entry)
    {
        if (!entry->alias.empty())
        {
            if (entry->alias != aliasEmpty)
            {
                return entry->alias;
            }
            return empty;
        }
        return entry->name;
    }
    else
    {
        entry = MetaEnum::getEntryById(0);
        if (entry)
        {
            if (!entry->alias.empty())
            {
                if (entry->alias != aliasEmpty)
                {
                    return entry->alias;
                }
                return empty;
            }
            return entry->name;
        }
    }
    static const std::string unknownEnum = "Unknown enum value, default enum is also not defined";
    return unknownEnum;
}



void MetaEnum::addEntry(const MetaEnumEntry& entry)
{
    addEntry(MetaEnumEntry(entry));
}


void MetaEnum::addEntry(MetaEnumEntry&& entry)
{
    if (m_id2Entry.find(entry.id) != m_id2Entry.end())
    {
        // entry already added
        return;
    }

    std::shared_ptr<MetaEnumEntry> e = std::make_shared<MetaEnumEntry>(std::move(entry));
    m_entries.emplace_back(e);
    m_id2Entry.emplace(e->id, e);
    m_name2Entry.emplace(e->name, e);
    m_alias2Entry.emplace(e->alias, e);
}

std::unordered_map<std::string, std::string> MetaEnum::generateProperties(const std::vector<std::string>& attrs)
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
