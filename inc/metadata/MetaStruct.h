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

#include "MetaField.h"
#include "helpers/BexDefines.h"

#include <vector>
#include <list>
#include <unordered_map>
#include <memory>


namespace finalmq {


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

    const MetaField* getFieldByIndex(ssize_t index) const;
    const MetaField* getFieldByName(const std::string& name) const;

    void addField(const MetaField& field);
    void addField(MetaField&& field);

    ssize_t getFieldsSize() const
    {
        return m_fields.size();
    }

private:
    std::string                                                         m_typeName;
    std::string                                                         m_description;
    std::vector<std::shared_ptr<const MetaField>>                       m_fields;
    std::unordered_map<std::string, std::shared_ptr<const MetaField>>   m_name2Field;
};

}   // namespace finalmq
