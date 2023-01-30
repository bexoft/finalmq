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

namespace finalmq {


MetaStruct::MetaStruct()
    : m_flags(0)
{

}

MetaStruct::MetaStruct(const std::string& typeName, const std::string& description, const std::vector<MetaField>& fields, int flags)
    : m_typeName(typeName)
    , m_description(description)
    , m_flags(flags)
{
    for (size_t i = 0 ; i < fields.size(); ++i)
    {
        addField(fields[i]);
    }
}

MetaStruct::MetaStruct(const std::string& typeName, const std::string& description, std::vector<MetaField>&& fields, int flags)
    : m_typeName(typeName)
    , m_description(description)
    , m_flags(flags)
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

void MetaStruct::setFlags(int flags)
{
    m_flags = flags;
}

int MetaStruct::getFlags() const
{
    return m_flags;
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
    addField(MetaField(field));
}


void MetaStruct::addField(MetaField&& field)
{
    if (m_name2Field.find(field.name) != m_name2Field.end())
    {
        // field already added
        return;
    }

    field.index = static_cast<int>(m_fields.size());

    std::shared_ptr<MetaField> f = std::make_shared<MetaField>(std::move(field));
    m_fields.emplace_back(f);
    m_name2Field.emplace(f->name, f);
}

}   // namespace finalmq
