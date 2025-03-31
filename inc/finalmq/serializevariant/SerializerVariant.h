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

#include <deque>

#include "finalmq/metadata/MetaStruct.h"
#include "finalmq/serialize/ParserConverter.h"
#include "finalmq/variant/Variant.h"

namespace finalmq
{
class VarValueToVariant;
class ParserProcessDefaultValues;

class SYMBOLEXP SerializerVariant : public ParserConverter
{
public:
    SerializerVariant(Variant& root, bool enumAsString = true, bool skipDefaultValues = false);

private:
    class Internal : public IParserVisitor
    {
    public:
        Internal(SerializerVariant& outer, Variant& root, bool enumAsString);

    private:
        Internal(const Internal&) = delete;
        Internal(Internal&&) = delete;
        const Internal& operator=(const Internal&) = delete;
        const Internal& operator=(Internal&&) = delete;

        // IParserVisitor
        virtual void notifyError(const char* str, const char* message) override;
        virtual void startStruct(const MetaStruct& stru) override;
        virtual void finished() override;

        virtual void enterStruct(const MetaField& field) override;
        virtual void exitStruct(const MetaField& field) override;
        virtual void enterStructNull(const MetaField& field) override;

        virtual void enterArrayStruct(const MetaField& field) override;
        virtual void exitArrayStruct(const MetaField& field) override;

        virtual void enterBool(const MetaField& field, bool value) override;
        virtual void enterInt8(const MetaField& field, std::int8_t value) override;
        virtual void enterUInt8(const MetaField& field, std::uint8_t value) override;
        virtual void enterInt16(const MetaField& field, std::int16_t value) override;
        virtual void enterUInt16(const MetaField& field, std::uint16_t value) override;
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
        virtual void enterJsonString(const MetaField& field, std::string&& value) override;
        virtual void enterJsonString(const MetaField& field, const char* value, ssize_t size) override;
        virtual void enterJsonVariant(const MetaField& field, const Variant& value) override;
        virtual void enterJsonVariantMove(const MetaField& field, Variant&& value) override;

        virtual void enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value) override;
        virtual void enterArrayBool(const MetaField& field, const std::vector<bool>& value) override;
        virtual void enterArrayInt8(const MetaField& field, std::vector<std::int8_t>&& value) override;
        virtual void enterArrayInt8(const MetaField& field, const std::int8_t* value, ssize_t size) override;
        virtual void enterArrayInt16(const MetaField& field, std::vector<std::int16_t>&& value) override;
        virtual void enterArrayInt16(const MetaField& field, const std::int16_t* value, ssize_t size) override;
        virtual void enterArrayUInt16(const MetaField& field, std::vector<std::uint16_t>&& value) override;
        virtual void enterArrayUInt16(const MetaField& field, const std::uint16_t* value, ssize_t size) override;
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

        template<class T>
        inline void add(const MetaField& field, const T& value);
        template<class T>
        void add(const MetaField& field, T&& value);

        Variant& m_root;
        Variant* m_current = nullptr;
        std::deque<Variant*> m_stack{};
        bool m_enumAsString = true;
        std::shared_ptr<VarValueToVariant> m_varValueToVariant{};
        SerializerVariant& m_outer;
    };

    Internal m_internal;
    std::unique_ptr<IParserVisitor> m_parserAbortAndIndex{};
    std::shared_ptr<ParserProcessDefaultValues> m_parserProcessDefaultValues{};
};

} // namespace finalmq
