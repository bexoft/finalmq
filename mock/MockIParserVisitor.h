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


#include "finalmq/serialize/IParserVisitor.h"

#include "gmock/gmock.h"

namespace finalmq {

class MockIParserVisitor : public IParserVisitor
{
public:
    MOCK_METHOD(void, notifyError, (const char* str, const char* message), (override));
    MOCK_METHOD(void, startStruct, (const MetaStruct& stru), (override));
    MOCK_METHOD(void, finished, (), (override));

    MOCK_METHOD(void, enterStruct, (const MetaField& field), (override));
    MOCK_METHOD(void, exitStruct, (const MetaField& field), (override));
    MOCK_METHOD(void, enterStructNull, (const MetaField& field), (override));

    MOCK_METHOD(void, enterArrayStruct, (const MetaField& field), (override));
    MOCK_METHOD(void, exitArrayStruct, (const MetaField& field), (override));

    MOCK_METHOD(void, enterBool, (const MetaField& field, bool value), (override));
    MOCK_METHOD(void, enterInt32, (const MetaField& field, std::int32_t value), (override));
    MOCK_METHOD(void, enterUInt32, (const MetaField& field, std::uint32_t value), (override));
    MOCK_METHOD(void, enterInt64, (const MetaField& field, std::int64_t value), (override));
    MOCK_METHOD(void, enterUInt64, (const MetaField& field, std::uint64_t value), (override));
    MOCK_METHOD(void, enterFloat, (const MetaField& field, float value), (override));
    MOCK_METHOD(void, enterDouble, (const MetaField& field, double value), (override));
    MOCK_METHOD(void, enterString, (const MetaField& field, std::string&& value), (override));
    MOCK_METHOD(void, enterString, (const MetaField& field, const char* value, ssize_t size), (override));
    MOCK_METHOD(void, enterBytes, (const MetaField& field, Bytes&& value), (override));
    MOCK_METHOD(void, enterBytes, (const MetaField& field, const BytesElement* value, ssize_t size), (override));
    MOCK_METHOD(void, enterEnum, (const MetaField& field, std::int32_t value), (override));
    MOCK_METHOD(void, enterEnum, (const MetaField& field, std::string&& value), (override));
    MOCK_METHOD(void, enterEnum, (const MetaField& field, const char* value, ssize_t size), (override));

    MOCK_METHOD(void, enterArrayBoolMove, (const MetaField& field, std::vector<bool>&& value), (override));
    MOCK_METHOD(void, enterArrayBool, (const MetaField& field, const std::vector<bool>& value), (override));
    MOCK_METHOD(void, enterArrayInt32, (const MetaField& field, std::vector<std::int32_t>&& value), (override));
    MOCK_METHOD(void, enterArrayInt32, (const MetaField& field, const std::int32_t* value, ssize_t size), (override));
    MOCK_METHOD(void, enterArrayUInt32, (const MetaField& field, std::vector<std::uint32_t>&& value), (override));
    MOCK_METHOD(void, enterArrayUInt32, (const MetaField& field, const std::uint32_t* value, ssize_t size), (override));
    MOCK_METHOD(void, enterArrayInt64, (const MetaField& field, std::vector<std::int64_t>&& value), (override));
    MOCK_METHOD(void, enterArrayInt64, (const MetaField& field, const std::int64_t* value, ssize_t size), (override));
    MOCK_METHOD(void, enterArrayUInt64, (const MetaField& field, std::vector<std::uint64_t>&& value), (override));
    MOCK_METHOD(void, enterArrayUInt64, (const MetaField& field, const std::uint64_t* value, ssize_t size), (override));
    MOCK_METHOD(void, enterArrayFloat, (const MetaField& field, std::vector<float>&& value), (override));
    MOCK_METHOD(void, enterArrayFloat, (const MetaField& field, const float* value, ssize_t size), (override));
    MOCK_METHOD(void, enterArrayDouble, (const MetaField& field, std::vector<double>&& value), (override));
    MOCK_METHOD(void, enterArrayDouble, (const MetaField& field, const double* value, ssize_t size), (override));
    MOCK_METHOD(void, enterArrayStringMove, (const MetaField& field, std::vector<std::string>&& value), (override));
    MOCK_METHOD(void, enterArrayString, (const MetaField& field, const std::vector<std::string>& value), (override));
    MOCK_METHOD(void, enterArrayBytesMove, (const MetaField& field, std::vector<Bytes>&& value), (override));
    MOCK_METHOD(void, enterArrayBytes, (const MetaField& field, const std::vector<Bytes>& value), (override));
    MOCK_METHOD(void, enterArrayEnum, (const MetaField& field, std::vector<std::int32_t>&& value), (override));
    MOCK_METHOD(void, enterArrayEnum, (const MetaField& field, const std::int32_t* value, ssize_t size), (override));
    MOCK_METHOD(void, enterArrayEnumMove, (const MetaField& field, std::vector<std::string>&& value), (override));
    MOCK_METHOD(void, enterArrayEnum, (const MetaField& field, const std::vector<std::string>& value), (override));
};

}   // namespace finalmq
