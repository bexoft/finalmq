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


#include "MetaType.h"
#include "finalmq/helpers/FmqDefines.h"
#include "finalmq/helpers/hybrid_ptr.h"
#include "finalmq/helpers/Utils.h"

#include <string>
#include <memory>
#include <unordered_map>


namespace finalmq {


class MetaEnum;
class MetaStruct;
struct IMetaData;


enum MetaFieldFlags : std::int32_t
{
    METAFLAG_NONE           = 0,
    METAFLAG_PROTO_VARINT   = 1,
    METAFLAG_PROTO_ZIGZAG   = 2,
    METAFLAG_NULLABLE       = 4,    // only for struct
    METAFLAG_ONE_REQUIRED   = 8,    // only for array of struct
    METAFLAG_INDEX          = 16,
};


class MetaField
{
public:
    MetaField(MetaTypeId tId, const std::string& tName, const std::string& n,
        const std::string& desc, int flgs = 0, const std::vector<std::string>& atrs = {}, int ix = -1)
        : typeId(tId)
        , typeName(tName)
        , typeNameWithoutNamespace(removeNamespace(tName))
        , name(n)
        , description(desc)
        , flags(flgs)
        , attrs(atrs)
        , properties(generateProperties(atrs))
        , index(ix)
        , sharedFieldWithoutArray((typeId & MetaTypeId::OFFSET_ARRAY_FLAG) ? std::make_shared<MetaField>(static_cast<MetaTypeId>(typeId & ~MetaTypeId::OFFSET_ARRAY_FLAG), typeName, "", description, flags, attrs, index) : nullptr)
        , fieldWithoutArray((sharedFieldWithoutArray != nullptr) ? sharedFieldWithoutArray.get() : this)
    {
    }

    MetaField(const MetaField& rhs)
        : typeId(rhs.typeId)
        , typeName(rhs.typeName)
        , typeNameWithoutNamespace(rhs.typeNameWithoutNamespace)
        , name(rhs.name)
        , description(rhs.description)
        , flags(rhs.flags)
        , attrs(rhs.attrs)
        , properties(rhs.properties)
        , index(rhs.index)
        , sharedFieldWithoutArray((typeId& MetaTypeId::OFFSET_ARRAY_FLAG) ? std::make_shared<MetaField>(static_cast<MetaTypeId>(typeId & ~MetaTypeId::OFFSET_ARRAY_FLAG), typeName, "", description, flags, attrs, index) : nullptr)
        , fieldWithoutArray((sharedFieldWithoutArray != nullptr) ? sharedFieldWithoutArray.get() : this)
    {
    }

    MetaField(MetaField&& rhs)
        : typeId(rhs.typeId)
        , typeName(rhs.typeName)
        , typeNameWithoutNamespace(rhs.typeNameWithoutNamespace)
        , name(rhs.name)
        , description(rhs.description)
        , flags(rhs.flags)
        , attrs(rhs.attrs)
        , properties(rhs.properties)
        , index(rhs.index)
        , sharedFieldWithoutArray((typeId& MetaTypeId::OFFSET_ARRAY_FLAG) ? std::make_shared<MetaField>(static_cast<MetaTypeId>(typeId & ~MetaTypeId::OFFSET_ARRAY_FLAG), typeName, "", description, flags, attrs, index) : nullptr)
        , fieldWithoutArray((sharedFieldWithoutArray != nullptr) ? sharedFieldWithoutArray.get() : this)
    {
    }

    const MetaField& operator =(MetaField& rhs) = delete;
    const MetaField& operator =(MetaField&& rhs) = delete;

    const std::string& getProperty(const std::string& key, const std::string& defaultValue = {}) const
    {
        const auto it = properties.find(key);
        if (it != properties.end())
        {
            return it->second;
        }
        return defaultValue;
    }

    const MetaTypeId        typeId;                         ///< type id of the parameter
    const std::string       typeName;                       ///< is needed for struct and enum
    const std::string       typeNameWithoutNamespace;       ///< is the typeName, but without the namespace
    const std::string       name{};                         ///< parameter name
    const std::string       description{};                  ///< description of the parameter
    const int               flags;                          ///< flaggs of the parameter
    const std::vector<std::string> attrs;                   ///< attributes of the parameter
    const std::unordered_map<std::string, std::string> properties; ///< properties of the parameter
    const int               index;                          ///< index of field inside struct

    const std::shared_ptr<MetaField> sharedFieldWithoutArray;     ///< in case of an array, this is the MetaField for its entries
    const MetaField* const fieldWithoutArray;     ///< in case of an array, this is the MetaField for its entries

private:
    static std::string removeNamespace(const std::string& typeName)
    {
        size_t pos = typeName.find_last_of('.') + 1;
        return typeName.substr(pos, typeName.size() - pos);
    }

    static std::unordered_map<std::string, std::string> generateProperties(const std::vector<std::string>& attrs)
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

    mutable const MetaEnum*     metaEnum    = nullptr;      ///< cache to find MetaEnum of typeName faster
    mutable const MetaStruct*   metaStruct  = nullptr;      ///< cache to find MetaStruct of typeName faster

    friend class MetaData;
};


}   // namespace finalmq
