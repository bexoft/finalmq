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

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "finalmq/helpers/FmqDefines.h"

namespace finalmq
{
struct MetaEnumEntry
{
public:
    std::string name{};
    int id{0};
    std::string description{};
    std::string alias{};
};

class SYMBOLEXP MetaEnum
{
public:
    MetaEnum();
    MetaEnum(const std::string& typeName, const std::string& description, const std::vector<std::string>& attrs, const std::vector<MetaEnumEntry>& entries);
    MetaEnum(const std::string& typeName, const std::string& description, const std::vector<std::string>& attrs, std::vector<MetaEnumEntry>&& entries);

    const std::string& getTypeName() const;
    void setDescription(const std::string& description);
    const std::string& getDescription() const;
    const std::vector<std::string>& getAttributes() const;
    const std::unordered_map<std::string, std::string>& getProperties() const;

    const MetaEnumEntry* getEntryById(int id) const;
    const MetaEnumEntry* getEntryByName(const std::string& name) const;
    const MetaEnumEntry* getEntryByIndex(ssize_t index) const;
    bool isId(int id) const;
    std::int32_t getValueByName(const std::string& name) const;
    const std::string& getNameByValue(std::int32_t value) const;
    const std::string& getAliasByValue(std::int32_t value) const;

    void addEntry(const MetaEnumEntry& entries);
    void addEntry(MetaEnumEntry&& entries);

    ssize_t getEntrySize() const
    {
        return m_entries.size();
    }

    const std::string& getProperty(const std::string& key, const std::string& defaultValue = {}) const
    {
        const auto it = m_properties.find(key);
        if (it != m_properties.end())
        {
            return it->second;
        }
        return defaultValue;
    }

private:
    static std::unordered_map<std::string, std::string> generateProperties(const std::vector<std::string>& attrs);

    std::string m_typeName{};
    std::string m_description{};
    std::vector<std::shared_ptr<const MetaEnumEntry>> m_entries{};
    const std::vector<std::string> m_attrs{};
    const std::unordered_map<std::string, std::string> m_properties{};
    std::unordered_map<int, std::shared_ptr<const MetaEnumEntry>> m_id2Entry{};
    std::unordered_map<std::string, std::shared_ptr<const MetaEnumEntry>> m_name2Entry{};
    std::unordered_map<std::string, std::shared_ptr<const MetaEnumEntry>> m_alias2Entry{};
};

} // namespace finalmq
