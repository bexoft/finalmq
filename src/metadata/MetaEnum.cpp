
#include "metadata/MetaEnum.h"



MetaEnum::MetaEnum()
{

}

MetaEnum::MetaEnum(const std::string& typeName, const std::string& description, const std::vector<MetaEnumEntry>& entries)
    : m_typeName(typeName)
    , m_description(description)
{
    for (size_t i = 0 ; i < entries.size(); ++i)
    {
        addEntry(entries[i]);
    }
}

MetaEnum::MetaEnum(const std::string& typeName, const std::string& description, std::vector<MetaEnumEntry>&& entries)
    : m_typeName(typeName)
    , m_description(description)
{
    for (size_t i = 0 ; i < entries.size(); ++i)
    {
        addEntry(std::move(entries[i]));
    }
}




void MetaEnum::setTypeName(const std::string& typeName)
{
    m_typeName = typeName;
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
    auto it = m_name2Entry.find(name);
    if (it != m_name2Entry.end())
    {
        return it->second.get();
    }
    return nullptr;
}



const MetaEnumEntry* MetaEnum::getEntryByIndex(int index) const
{
    if (index < static_cast<int>(m_entries.size()))
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
}

