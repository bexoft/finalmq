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
#include "finalmq/serialize/ParserConverter.h"
#include "StructBase.h"

#include <deque>

namespace finalmq {

class VarValueToVariant;

class SYMBOLEXP SerializerStruct : public ParserConverter
{
public:
    SerializerStruct(StructBase& root);
    void setExitNotification(std::function<void()> funcExit);

private:
    class Internal : public IParserVisitor
    {
    public:
        Internal(SerializerStruct& outer, StructBase& root);
        void setExitNotification(std::function<void()> funcExit);
    private:
        // IParserVisitor
        virtual void notifyError(const char* str, const char* message) override;
        virtual void startStruct(const MetaStruct& stru) override;
        virtual void finished() override;

        virtual void enterStruct(const MetaField& field) override;
        virtual void exitStruct(const MetaField& field) override;

        virtual void enterArrayStruct(const MetaField& field) override;
        virtual void exitArrayStruct(const MetaField& field) override;

        virtual void enterBool(const MetaField& field, bool value) override;
        virtual void enterInt32(const MetaField& field, std::int32_t value) override;
        virtual void enterUInt32(const MetaField& field, std::uint32_t value) override;
        virtual void enterInt64(const MetaField& field, std::int64_t value) override;
        virtual void enterUInt64(const MetaField& field, std::uint64_t value) override;
        virtual void enterFloat(const MetaField& field, float value) override;
        virtual void enterDouble(const MetaField& field, double value) override;
        virtual void enterString(const MetaField& field, std::string&& value) override;
        virtual void enterString(const MetaField& field, const char* value, ssize_t size) override;
        virtual void enterBytes(const MetaField& field, Bytes&& value) override;
        virtual void enterBytes(const MetaField& field, const BytesElement* value, ssize_t size) override;
        virtual void enterEnum(const MetaField& field, std::int32_t value) override;
        virtual void enterEnum(const MetaField& field, std::string&& value) override;
        virtual void enterEnum(const MetaField& field, const char* value, ssize_t size) override;

        virtual void enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value) override;
        virtual void enterArrayBool(const MetaField& field, const std::vector<bool>& value) override;
        virtual void enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value) override;
        virtual void enterArrayInt32(const MetaField& field, const std::int32_t* value, ssize_t size) override;
        virtual void enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value) override;
        virtual void enterArrayUInt32(const MetaField& field, const std::uint32_t* value, ssize_t size) override;
        virtual void enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value) override;
        virtual void enterArrayInt64(const MetaField& field, const std::int64_t* value, ssize_t size) override;
        virtual void enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value) override;
        virtual void enterArrayUInt64(const MetaField& field, const std::uint64_t* value, ssize_t size) override;
        virtual void enterArrayFloat(const MetaField& field, std::vector<float>&& value) override;
        virtual void enterArrayFloat(const MetaField& field, const float* value, ssize_t size) override;
        virtual void enterArrayDouble(const MetaField& field, std::vector<double>&& value) override;
        virtual void enterArrayDouble(const MetaField& field, const double* value, ssize_t size) override;
        virtual void enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value) override;
        virtual void enterArrayString(const MetaField& field, const std::vector<std::string>& value) override;
        virtual void enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value) override;
        virtual void enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value) override;
        virtual void enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value) override;
        virtual void enterArrayEnum(const MetaField& field, const std::int32_t* value, ssize_t size) override;
        virtual void enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value) override;
        virtual void enterArrayEnum(const MetaField& field, const std::vector<std::string>& value) override;

        template <class T>
        void setValue(StructBase& structBase, const FieldInfo& fieldInfoDest, const T& value);

        template <class T>
        void setValue(StructBase& structBase, const FieldInfo& fieldInfoDest, T&& value);

        template<class T>
        void convertNumber(StructBase& structBase, const FieldInfo& fieldInfoDest, T value);
            
        void convertString(StructBase& structBase, const FieldInfo& fieldInfoDest, const char* value, ssize_t size);
            
        template<class T>
        void convertArrayNumber(StructBase& structBase, const FieldInfo& fieldInfoDest, const T* value, ssize_t size);
            
        void convertArrayString(StructBase& structBase, const FieldInfo& fieldInfoDest, const std::vector<std::string>& value);
            
        template<class T>
        void setValueNumber(const MetaField& field, T value);

        void setValueString(const MetaField& field, const char* value, ssize_t size);
            
        template<class T>
        void setValueArrayNumber(const MetaField& field, const T* value, ssize_t size);
        template<class T>
        void setValueArrayNumber(const MetaField& field, std::vector<T>&& value);

        void setValueArrayString(const MetaField& field, const std::vector<std::string>& value);
        void setValueArrayString(const MetaField& field, const std::vector<std::string>&& value);
        
        const FieldInfo* getFieldInfoDest(const MetaField& field);

        struct StackEntry
        {
            StructBase* structBase = nullptr;
            ssize_t     structArrayIndex = -1;
        };

        StructBase&                     m_root;
        StackEntry*                     m_current = nullptr;
        std::deque<StackEntry>          m_stack;
        std::function<void()>           m_funcExit;
        std::shared_ptr<VarValueToVariant> m_varValueToVariant;
        bool                            m_wasStartStructCalled = false;
        SerializerStruct&               m_outer;
        Variant                         m_variantDummy;
    };

    Internal                            m_internal;
};

}   // namespace finalmq
