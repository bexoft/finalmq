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

#include "finalmq/metadata/MetaStruct.h"
#include "finalmq/metadata/MetaEnum.h"

#include <mutex>
#include <memory>


namespace finalmq {


struct IMetaData
{
    virtual ~IMetaData() {}
    virtual const MetaStruct* getStruct(const std::string& typeName) const = 0;
    virtual const MetaEnum* getEnum(const std::string& typeName) const = 0;
    virtual const MetaStruct* getStruct(const MetaField& field) const = 0;
    virtual const MetaField* getField(const std::string& typeName, const std::string& fieldName) const = 0;
    virtual const MetaEnum* getEnum(const MetaField& field) const = 0;
    virtual const MetaField* getArrayField(const MetaField& field) const = 0;
    virtual const MetaField* getArrayField(const std::string& typeName, const std::string& fieldName) const = 0;
    virtual bool isEnumValue(const MetaField& field, std::int32_t value) const = 0;
    virtual std::int32_t getEnumValueByName(const MetaField& field, const std::string& name) const = 0;
    virtual const std::string& getEnumNameByValue(const MetaField& field, std::int32_t value) const = 0;
    virtual const MetaStruct& addStruct(const MetaStruct& stru) = 0;
    virtual const MetaStruct& addStruct(MetaStruct&& stru) = 0;
    virtual const MetaEnum& addEnum(const MetaEnum& en) = 0;
    virtual const MetaEnum& addEnum(MetaEnum&& en) = 0;
    virtual const std::unordered_map<std::string, MetaStruct> getAllStructs() const = 0;
    virtual const std::unordered_map<std::string, MetaEnum> getAllEnums() const = 0;
};


class SYMBOLEXP MetaData : public IMetaData
{
public:

private:
    // IMetaData
    virtual const MetaStruct* getStruct(const std::string& typeName) const override;
    virtual const MetaEnum* getEnum(const std::string& typeName) const override;
    virtual const MetaStruct* getStruct(const MetaField& field) const override;
    virtual const MetaField* getField(const std::string& typeName, const std::string& fieldName) const override;
    virtual const MetaEnum* getEnum(const MetaField& field) const override;
    virtual const MetaField* getArrayField(const MetaField& field) const override;
    virtual const MetaField* getArrayField(const std::string& typeName, const std::string& fieldName) const override;
    virtual bool isEnumValue(const MetaField& field, std::int32_t value) const override;
    virtual std::int32_t getEnumValueByName(const MetaField& field, const std::string& name) const override;
    virtual const std::string& getEnumNameByValue(const MetaField& field, std::int32_t value) const override;
    virtual const MetaStruct& addStruct(const MetaStruct& stru) override;
    virtual const MetaStruct& addStruct(MetaStruct&& stru) override;
    virtual const MetaEnum& addEnum(const MetaEnum& en) override;
    virtual const MetaEnum& addEnum(MetaEnum&& en) override;
    virtual const std::unordered_map<std::string, MetaStruct> getAllStructs() const override;
    virtual const std::unordered_map<std::string, MetaEnum> getAllEnums() const override;

    std::unordered_map<std::string, MetaStruct> m_name2Struct;
    std::unordered_map<std::string, MetaEnum>   m_name2Enum;
    mutable std::mutex                          m_mutex;
};


class SYMBOLEXP MetaDataGlobal
{
public:
    inline static IMetaData& instance()
    {
        if (!m_instance)
        {
            m_instance = std::make_unique<MetaData>();
        }
        return *m_instance.get();
    }

    static void setInstance(std::unique_ptr<IMetaData>&& instance);

private:
    MetaDataGlobal() = delete;

    static std::unique_ptr<IMetaData> m_instance;
};

}   // namespace finalmq
