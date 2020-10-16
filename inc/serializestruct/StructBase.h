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

    inline const MetaField* getField() const
    {
        return m_field;
    }
    inline int getOffset() const
    {
        return m_offset;
    }

private:
    const MetaField*    m_field = nullptr;
    int                 m_offset;
};



class StructInfo
{
public:
    StructInfo(const std::string& typeName, std::vector<MetaField>&& fields, std::vector<FieldInfo>&& fieldInfos);

    inline const FieldInfo* getField(int index) const
    {
        if (0 <= index && index < static_cast<int>(m_fieldInfos.size()))
        {
            return &m_fieldInfos[index];
        }
        return nullptr;
    }

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
    virtual void clear() = 0;
    virtual StructBase* add(int index);

    template<class T>
    T* getValue(int index, int typeId)
    {
        const StructInfo& structInfo = getStructInfo();
        const FieldInfo* fieldInfo = structInfo.getField(index);
        if (fieldInfo)
        {
            const MetaField* field = fieldInfo->getField();
            if (field)
            {
                if (field->typeId == typeId)
                {
                    int offset = fieldInfo->getOffset();
                    return reinterpret_cast<T*>(reinterpret_cast<char*>(this) + offset);
                }
            }
        }
        return nullptr;
    }

    template<class T>
    const T* getValue(int index, int typeId) const
    {
        return const_cast<StructBase*>(this)->getValue<T>(index, typeId);
    }
private:
    virtual const StructInfo& getStructInfo() const = 0;
};



#define OFFSET_STRUCTBASE_TO_PARAM(type, param)         ((int)((long long)(&((type*)1000)->param) - ((long long)(StructBase*)((type*)1000))))
#define OFFSET_STRUCTBASE_TO_STRUCTBASE(type, param)    ((int)((long long)((StructBase*)&((type*)1000)->param) - ((long long)(StructBase*)((type*)1000))))


