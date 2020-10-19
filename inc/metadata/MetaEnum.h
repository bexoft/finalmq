#pragma once


#include <vector>
#include <string>
#include <memory>
#include <unordered_map>


struct MetaEnumEntry
{
public:
    std::string     name;
    int             id = 0;
    std::string     description;
};



class MetaEnum
{
public:
    MetaEnum();
    MetaEnum(const std::string& typeName, const std::vector<MetaEnumEntry>& entries);
    MetaEnum(const std::string& typeName, std::vector<MetaEnumEntry>&& entries);

    void setTypeName(const std::string& typeName);
    const std::string& getTypeName() const;

    const MetaEnumEntry* getEntryById(int id) const;
    const MetaEnumEntry* getEntryByName(const std::string& name) const;
    const MetaEnumEntry* getEntryByIndex(int index) const;
    bool isId(int id) const;
    std::int32_t getValueByName(const std::string& name) const;
    const std::string& getNameByValue(std::int32_t value) const;

    void addEntry(const MetaEnumEntry& entries);
    void addEntry(MetaEnumEntry&& entries);

    int getEntrySize() const
    {
        return m_entries.size();
    }

private:
    std::string                                                             m_typeName;
    std::vector<std::shared_ptr<const MetaEnumEntry>>                       m_entries;
    std::unordered_map<int, std::shared_ptr<const MetaEnumEntry>>           m_id2Entry;
    std::unordered_map<std::string, std::shared_ptr<const MetaEnumEntry>>   m_name2Entry;
};

