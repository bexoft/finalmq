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

#include "metadata/MetaField.h"
#include "metadata/MetaEnum.h"
#include "metadata/MetaStruct.h"

#include <functional>
#include <cstdint>
#include <vector>
#include <assert.h>


namespace finalmq {


class StructBase;

struct IArrayStructAdapter
{
    virtual ~IArrayStructAdapter() {}
    virtual StructBase* add(void* array) = 0;
    virtual ssize_t size(const void* array) const = 0;
    virtual const StructBase& at(const void* array, ssize_t index) const = 0;
};


template<class T>
class ArrayStructAdapter : public IArrayStructAdapter
{
private:
    // IArrayStructAdapter
    virtual StructBase* add(void* array) override
    {
        std::vector<T>& vect = *reinterpret_cast<std::vector<T>*>(array);
        vect.resize(vect.size() + 1);
        return &vect.back();
    }

    virtual ssize_t size(const void* array) const override
    {
        const std::vector<T>& vect = *reinterpret_cast<const std::vector<T>*>(array);
        return vect.size();
    }

    virtual const StructBase& at(const void* array, ssize_t index) const override
    {
        const std::vector<T>& vect = *reinterpret_cast<const std::vector<T>*>(array);
        assert(index >= 0 && index < static_cast<ssize_t>(vect.size()));
        return vect[index];
    }
};



class FieldInfo
{
public:
    FieldInfo(int offset, IArrayStructAdapter* arrayStructAdapter = nullptr);
    void setField(const MetaField* field);

    inline const MetaField* getField() const
    {
        return m_field;
    }
    inline int getOffset() const
    {
        return m_offset;
    }
    inline IArrayStructAdapter* getArrayStructAdapter() const
    {
        return m_arrayStructAdapter;
    }

private:
    const MetaField*        m_field = nullptr;
    int                     m_offset;
    IArrayStructAdapter*    m_arrayStructAdapter = nullptr;
};


typedef std::function<std::shared_ptr<StructBase>()>    FuncStructBaseFactory;


class StructInfo
{
public:
    StructInfo(const std::string& typeName, const std::string& description, FuncStructBaseFactory factory, std::vector<MetaField>&& fields, std::vector<FieldInfo>&& fieldInfos);

    inline const std::string& getTypeName() const
    {
        return m_metaStruct.getTypeName();
    }

    inline const FieldInfo* getField(ssize_t index) const
    {
        if (0 <= index && index < static_cast<ssize_t>(m_fieldInfos.size()))
        {
            return &m_fieldInfos[index];
        }
        return nullptr;
    }

    inline const std::vector<FieldInfo>& getFields() const
    {
        return m_fieldInfos;
    }

private:
    const MetaStruct&       m_metaStruct;
    std::vector<FieldInfo>  m_fieldInfos;
};


class EnumInfo
{
public:
    EnumInfo(const std::string& typeName, const std::string& description, std::vector<MetaEnumEntry>&& entries);
    const MetaEnum& getMetaEnum() const;

private:
    const MetaEnum&   m_metaEnum;
};


class StructBase
{
public:
    StructBase* add(ssize_t index);

    virtual void clear() = 0;

    template<class T>
    T* getData(ssize_t index, int typeId)
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
    const T* getData(ssize_t index, int typeId) const
    {
        return const_cast<StructBase*>(this)->getData<T>(index, typeId);
    }

    template<class T>
    const T& getValue(ssize_t index, int typeId) const
    {
        const T* data = getData<T>(index, typeId);
        if (data)
        {
            return *data;
        }
        static const T defaultValue = T();
        return defaultValue;
    }

    virtual const StructInfo& getStructInfo() const = 0;
};

typedef std::shared_ptr<StructBase> StructBasePtr;


#define OFFSET_STRUCTBASE_TO_PARAM(type, param)         ((int)((long long)(&((type*)1000)->param) - ((long long)(StructBase*)((type*)1000))))
#define OFFSET_STRUCTBASE_TO_STRUCTBASE(type, param)    ((int)((long long)((StructBase*)&((type*)1000)->param) - ((long long)(StructBase*)((type*)1000))))



}   // namespace finalmq

