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
#include "finalmq/helpers/IZeroCopyBuffer.h"
#include "finalmq/serialize/ParserConverter.h"

#include <deque>

namespace finalmq {

class SYMBOLEXP SerializerProto : public ParserConverter
{
public:
    SerializerProto(IZeroCopyBuffer& buffer, int maxBlockSize = 512);

private:
    enum WireType
    {
        WIRETYPE_VARINT = 0,
        WIRETYPE_FIXED64 = 1,
        WIRETYPE_LENGTH_DELIMITED = 2,
        WIRETYPE_START_GROUP = 3,
        WIRETYPE_END_GROUP = 4,
        WIRETYPE_FIXED32 = 5,
    };

    class Internal : public IParserVisitor
    {
    public:
        Internal(IZeroCopyBuffer& buffer, int maxBlockSize);
    private:
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

        template<bool ignoreZeroLength>
        void serializeString(int id, const char* value, ssize_t size);

        char* serializeStruct(int id);

        void serializeVarintValue(int id, std::uint64_t value);

        void serializeZigZagValue(int id, std::int64_t value);

        inline std::uint64_t zigzag(std::int64_t value);

        template<class T, int WIRETYPE>
        void serializeFixedValue(int id, T value);

        template<class T>
        void serializeArrayFixed(int id, const T* value, ssize_t size);

        void serializeArrayBool(int id, const std::vector<bool>& value);
        void serializeArrayString(int id, const std::vector<std::string>& value);
        void serializeArrayBytes(int id, const std::vector<Bytes>& value);

        template<class T>
        void serializeArrayVarint(int id, const T* value, ssize_t size);

        template<class T>
        void serializeArrayZigZag(int id, const T* value, ssize_t size);

        inline void serializeVarint(std::uint64_t value);
        void reserveSpace(ssize_t space);
        void resizeBuffer();
        ssize_t calculateStructSize(ssize_t& structSize);
        void fillRemainingStruct(ssize_t remainingSize);

        struct StructData
        {
            StructData(char* bstart, char* bsize, char* b, bool ae)
                : bufferStructStart(bstart)
                , bufferStructSize(bsize)
                , buffer(b)
                , arrayEntry(ae)
            {
            }
            char*   bufferStructStart = nullptr;
            char*   bufferStructSize = nullptr;
            char*   buffer = nullptr;
            ssize_t size = 0;
            bool    allocateNextDataBuffer = false;
            bool    arrayParent = false;
            bool    arrayEntry = false;
        };

        IZeroCopyBuffer&        m_zeroCopybuffer;
        ssize_t                 m_maxBlockSize = 512;
        char*                   m_bufferStart = nullptr;
        char*                   m_buffer = nullptr;
        char*                   m_bufferEnd = nullptr;
        bool                    m_arrayParent = false;
        std::deque<StructData>  m_stackStruct;
    };

    Internal    m_internal;

    std::unique_ptr<IParserVisitor>     m_parserAbortAndIndex;
};

}   // namespace finalmq
