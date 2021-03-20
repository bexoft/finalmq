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



namespace finalmq {

struct IParserVisitor
{
    virtual ~IParserVisitor() {}

    virtual void notifyError(const char* str, const char* message) = 0;
    virtual void finished() = 0;

    virtual void enterStruct(const MetaField& field) = 0;
    virtual void exitStruct(const MetaField& field) = 0;

    virtual void enterArrayStruct(const MetaField& field) = 0;
    virtual void exitArrayStruct(const MetaField& field) = 0;

    virtual void enterBool(const MetaField& field, bool value) = 0;
    virtual void enterInt32(const MetaField& field, std::int32_t value) = 0;
    virtual void enterUInt32(const MetaField& field, std::uint32_t value) = 0;
    virtual void enterInt64(const MetaField& field, std::int64_t value) = 0;
    virtual void enterUInt64(const MetaField& field, std::uint64_t value) = 0;
    virtual void enterFloat(const MetaField& field, float value) = 0;
    virtual void enterDouble(const MetaField& field, double value) = 0;
    virtual void enterString(const MetaField& field, std::string&& value) = 0;
    virtual void enterString(const MetaField& field, const char* value, ssize_t size) = 0;
    virtual void enterBytes(const MetaField& field, Bytes&& value) = 0;
    virtual void enterBytes(const MetaField& field, const BytesElement* value, ssize_t size) = 0;
    virtual void enterEnum(const MetaField& field, std::int32_t value) = 0;
    virtual void enterEnum(const MetaField& field, std::string&& value) = 0;
    virtual void enterEnum(const MetaField& field, const char* value, ssize_t size) = 0;

    virtual void enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value) = 0;
    virtual void enterArrayBool(const MetaField& field, const std::vector<bool>& value) = 0;
    virtual void enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value) = 0;
    virtual void enterArrayInt32(const MetaField& field, const std::int32_t* value, ssize_t size) = 0;
    virtual void enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value) = 0;
    virtual void enterArrayUInt32(const MetaField& field, const std::uint32_t* value, ssize_t size) = 0;
    virtual void enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value) = 0;
    virtual void enterArrayInt64(const MetaField& field, const std::int64_t* value, ssize_t size) = 0;
    virtual void enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value) = 0;
    virtual void enterArrayUInt64(const MetaField& field, const std::uint64_t* value, ssize_t size) = 0;
    virtual void enterArrayFloat(const MetaField& field, std::vector<float>&& value) = 0;
    virtual void enterArrayFloat(const MetaField& field, const float* value, ssize_t size) = 0;
    virtual void enterArrayDouble(const MetaField& field, std::vector<double>&& value) = 0;
    virtual void enterArrayDouble(const MetaField& field, const double* value, ssize_t size) = 0;
    virtual void enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value) = 0;
    virtual void enterArrayString(const MetaField& field, const std::vector<std::string>& value) = 0;
    virtual void enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value) = 0;
    virtual void enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value) = 0;
    virtual void enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value) = 0;
    virtual void enterArrayEnum(const MetaField& field, const std::int32_t* value, ssize_t size) = 0;
    virtual void enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value) = 0;
    virtual void enterArrayEnum(const MetaField& field, const std::vector<std::string>& value) = 0;
};

}   // namespace finalmq
