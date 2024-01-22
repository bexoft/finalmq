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

#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

#include "MetaField.h"
#include "finalmq/helpers/FmqDefines.h"

namespace finalmq
{
enum MetaStructFlags : std::int32_t
{
    METASTRUCTFLAG_NONE = 0,
    METASTRUCTFLAG_HL7_SEGMENT = 1,
    METASTRUCTFLAG_CHOICE = 2,
};

class SYMBOLEXP MetaStruct
{
public:
    MetaStruct();
    MetaStruct(const std::string& typeName, const std::string& description, const std::vector<MetaField>& fields, int flags = 0, const std::vector<std::string>& attrs = {});

    const std::string& getTypeName() const;
    const std::string& getTypeNameWithoutNamespace() const;
    const std::string& getDescription() const;
    int getFlags() const;
    const std::vector<std::string>& getAttributes() const;
    const std::unordered_map<std::string, std::string>& getProperties() const;

    const MetaField* getFieldByIndex(ssize_t index) const;
    const MetaField* getFieldByName(const std::string& name) const;

    void addField(const MetaField& field);

    ssize_t getFieldsSize() const
    {
        return m_fields.size();
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

    const std::string m_typeName{};
    const std::string m_typeNameWithoutNamespace{};
    const std::string m_description{};
    std::vector<std::shared_ptr<const MetaField>> m_fields{};
    const int m_flags{};
    const std::vector<std::string> m_attrs{};
    const std::unordered_map<std::string, std::string> m_properties{};
    std::unordered_map<std::string, std::shared_ptr<const MetaField>> m_name2Field{};
};

} // namespace finalmq
