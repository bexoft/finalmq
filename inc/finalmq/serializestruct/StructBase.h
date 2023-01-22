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

#include "finalmq/metadata/MetaField.h"
#include "finalmq/metadata/MetaEnum.h"
#include "finalmq/metadata/MetaStruct.h"
#include "finalmq/variant/Variant.h"

#include <functional>
#include <cstdint>
#include <vector>
#include <assert.h>


namespace finalmq {


    class StructBase;
    typedef std::shared_ptr<StructBase> StructBasePtr;

    struct IArrayStructAdapter
    {
        virtual ~IArrayStructAdapter() {}
        virtual StructBase* add(char& array) = 0;
        virtual ssize_t size(const char& array) const = 0;
        virtual const StructBase& at(const char& array, ssize_t index) const = 0;
    };

    struct IStructPtrAdapter
    {
        virtual ~IStructPtrAdapter() {}
        virtual StructBasePtr create() const = 0;
    };


    template<class T>
    class ArrayStructAdapter : public IArrayStructAdapter
    {
    private:
        // IArrayStructAdapter
        virtual StructBase* add(char& array) override
        {
            std::vector<T>& vect = reinterpret_cast<std::vector<T>&>(array);
            vect.resize(vect.size() + 1);
            return &vect.back();
        }

        virtual ssize_t size(const char& array) const override
        {
            const std::vector<T>& vect = reinterpret_cast<const std::vector<T>&>(array);
            return vect.size();
        }

        virtual const StructBase& at(const char& array, ssize_t index) const override
        {
            const std::vector<T>& vect = reinterpret_cast<const std::vector<T>&>(array);
            assert(index >= 0 && index < static_cast<ssize_t>(vect.size()));
            return vect[index];
        }
    };

    template<class T>
    class StructPtrAdapter : public IStructPtrAdapter
    {
    private:
        // IStructPtrAdapter
        virtual StructBasePtr create() const override
        {
            return std::make_pair<T>();
        }
    };


    class SYMBOLEXP FieldInfo
    {
    public:
        FieldInfo(int offset, IArrayStructAdapter* arrayStructAdapter = nullptr, IStructPtrAdapter* structPtrAdapter = nullptr);
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
        inline StructBasePtr createStruct() const
        {
            if (m_structPtrAdapter)
            {
                return m_structPtrAdapter->create();
            }
            return nullptr;
        }

    private:
        const MetaField* m_field = nullptr;
        int                     m_offset;
        IArrayStructAdapter* m_arrayStructAdapter = nullptr;
        IStructPtrAdapter* m_structPtrAdapter = nullptr;
    };


    typedef std::function<std::shared_ptr<StructBase>()>    FuncStructBaseFactory;


    class SYMBOLEXP StructInfo
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

        inline const MetaStruct& getMetaStruct() const
        {
            return m_metaStruct;
        }

    private:
        const MetaStruct& m_metaStruct;
        std::vector<FieldInfo>  m_fieldInfos;
    };


    class SYMBOLEXP EnumInfo
    {
    public:
        EnumInfo(const std::string& typeName, const std::string& description, std::vector<MetaEnumEntry>&& entries);
        const MetaEnum& getMetaEnum() const;

    private:
        const MetaEnum& m_metaEnum;
    };

    template <>
    class MetaTypeInfo<Variant>
    {
    public:
        static const int TypeId = MetaTypeId::TYPE_STRUCT;
    };
    template <>
    class MetaTypeInfo<StructBase>
    {
    public:
        static const int TypeId = MetaTypeId::TYPE_STRUCT;
    };
    template <>
    class MetaTypeInfo<StructBasePtr>
    {
    public:
        static const int TypeId = MetaTypeId::TYPE_STRUCT;
    };



    class SYMBOLEXP StructBase
    {
    public:
        StructBase* add(ssize_t index);

        virtual void clear() = 0;

        template<class T>
        T* getData(const FieldInfo& fieldInfo)
        {
            const MetaField* fieldDest = fieldInfo.getField();
            if (fieldDest)
            {
                if ((fieldDest->typeId == MetaTypeInfo<T>::TypeId)
                    || ((MetaTypeInfo<T>::TypeId == MetaTypeInfo<std::int32_t>::TypeId) && (fieldDest->typeId == MetaTypeId::TYPE_ENUM))
                    || ((MetaTypeInfo<T>::TypeId == MetaTypeInfo<std::vector<std::int32_t>>::TypeId) && (fieldDest->typeId == MetaTypeId::TYPE_ARRAY_ENUM)))
                {
                    int offset = fieldInfo.getOffset();
                    return reinterpret_cast<T*>(reinterpret_cast<char*>(this) + offset);
                }
            }
            return nullptr;
        }


        template<class T>
        const T& getValue(const FieldInfo& fieldInfo) const
        {
            int offset = fieldInfo.getOffset();
            const T& data = *reinterpret_cast<T*>(reinterpret_cast<char*>(const_cast<StructBase*>(this)) + offset);
            return data;
        }

        virtual const StructInfo& getStructInfo() const = 0;
        virtual std::shared_ptr<StructBase> clone() const = 0;

    private:
        struct RawData
        {
            std::string type;
            int         contentType = 0;
            std::string data;
        };

    public:
        void setRawData(const std::string& type, int contentType, const char* rawData, ssize_t size)
        {
            if (!m_rawData)
            {
                m_rawData = std::make_shared<RawData>(RawData{ type, contentType, {rawData, rawData + size} });
            }
        }
        const std::string* getRawType() const
        {
            if (m_rawData)
            {
                return &m_rawData->type;
            }
            return nullptr;
        }
        int getRawContentType() const
        {
            if (m_rawData)
            {
                return m_rawData->contentType;
            }
            return 0;
        }
        const std::string* getRawData() const
        {
            if (m_rawData)
            {
                return &m_rawData->data;
            }
            return nullptr;
        }

    private:
        std::shared_ptr<RawData>    m_rawData;
    };

    typedef std::shared_ptr<StructBase> StructBasePtr;


#define OFFSET_STRUCTBASE_TO_PARAM(type, param)         ((int)((long long)(&((type*)1000)->param) - ((long long)(StructBase*)((type*)1000))))
#define OFFSET_STRUCTBASE_TO_STRUCTBASE(type, param)    ((int)((long long)((StructBase*)&((type*)1000)->param) - ((long long)(StructBase*)((type*)1000))))



}   // namespace finalmq

