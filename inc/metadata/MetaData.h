#pragma once

#include "metadata/MetaStruct.h"
#include "metadata/MetaEnum.h"

#include <mutex>
#include <memory>

struct IMetaData
{
    virtual ~IMetaData() {}
    virtual const MetaStruct* getStruct(const std::string& typeName) const = 0;
    virtual const MetaEnum* getEnum(const std::string& typeName) const = 0;
    virtual const MetaStruct* getStruct(const MetaField& field) const = 0;
    virtual const MetaEnum* getEnum(const MetaField& field) const = 0;
    virtual const MetaField* getArrayField(const MetaField& field) const = 0;
    virtual std::int32_t getEnumValueByName(const MetaField& field, const std::string& name) const = 0;
    virtual const std::string& getEnumNameByValue(const MetaField& field, std::int32_t value) const = 0;
    virtual const MetaStruct& addStruct(const MetaStruct& stru) = 0;
    virtual const MetaStruct& addStruct(MetaStruct&& stru) = 0;
    virtual const MetaEnum& addEnum(const MetaEnum& en) = 0;
    virtual const MetaEnum& addEnum(MetaEnum&& en) = 0;
};


class MetaData : public IMetaData
{
public:

private:
    // IMetaData
    virtual const MetaStruct* getStruct(const std::string& typeName) const override;
    virtual const MetaEnum* getEnum(const std::string& typeName) const override;
    virtual const MetaStruct* getStruct(const MetaField& field) const override;
    virtual const MetaEnum* getEnum(const MetaField& field) const override;
    virtual const MetaField* getArrayField(const MetaField& field) const override;
    virtual std::int32_t getEnumValueByName(const MetaField& field, const std::string& name) const override;
    virtual const std::string& getEnumNameByValue(const MetaField& field, std::int32_t value) const override;
    virtual const MetaStruct& addStruct(const MetaStruct& stru) override;
    virtual const MetaStruct& addStruct(MetaStruct&& stru) override;
    virtual const MetaEnum& addEnum(const MetaEnum& en) override;
    virtual const MetaEnum& addEnum(MetaEnum&& en) override;

    std::unordered_map<std::string, MetaStruct> m_name2Struct;
    std::unordered_map<std::string, MetaEnum>   m_name2Enum;
    std::mutex                                  m_mutex;
};


class MetaDataGlobal
{
public:
    static IMetaData& instance();
    static void setInstance(std::unique_ptr<IMetaData>&& instance);

private:
    MetaDataGlobal() = delete;

    static std::unique_ptr<IMetaData> m_instance;
};

