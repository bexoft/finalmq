#pragma once

#include "metadata/MetaField.h"
#include "metadata/MetaEnum.h"

#include <cstdint>
#include <vector>



class FieldInfo
{
public:
    FieldInfo(int offset);
    void setField(const MetaField* field);
    const MetaField* getField() const;

private:
    const MetaField*    m_field = nullptr;
    int                 m_offset;
};



class StructInfo
{
public:
    StructInfo(const std::string& typeName, std::vector<MetaField>&& fields, std::vector<FieldInfo>&& fieldInfos);

private:
    std::vector<FieldInfo> m_fieldInfos;
};


class EnumInfo
{
public:
    EnumInfo(const std::string& typeName, std::vector<MetaEnumEntry>&& entries);
    const MetaEnum& getMetaEnum() const;

private:
    const MetaEnum&   m_metaEnum;
};


class StructBase
{
public:

private:
};



#define OFFSET_STRUCTBASE_TO_PARAM(type, param)         ((int)((long long)(&((type*)1000)->param) - ((long long)(StructBase*)((type*)1000))))
#define OFFSET_STRUCTBASE_TO_STRUCTBASE(type, param)    ((int)((long long)((StructBase*)&((type*)1000)->param) - ((long long)(StructBase*)((type*)1000))))


