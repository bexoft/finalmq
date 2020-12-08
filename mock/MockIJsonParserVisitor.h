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


#include "json/JsonParser.h"

#include "gmock/gmock.h"

namespace finalmq {

class MockIJsonParserVisitor : public IJsonParserVisitor
{
public:
    MOCK_METHOD(void, syntaxError, (const char* str, const char* message), (override));
    MOCK_METHOD(void, enterNull, (), (override));
    MOCK_METHOD(void, enterBool, (bool value), (override));
    MOCK_METHOD(void, enterInt32, (std::int32_t value), (override));
    MOCK_METHOD(void, enterUInt32, (std::uint32_t value), (override));
    MOCK_METHOD(void, enterInt64, (std::int64_t value), (override));
    MOCK_METHOD(void, enterUInt64, (std::uint64_t value), (override));
    MOCK_METHOD(void, enterDouble, (double value), (override));
    MOCK_METHOD(void, enterString, (const char* value, ssize_t size), (override));
    MOCK_METHOD(void, enterString, (std::string&& value), (override));
    MOCK_METHOD(void, enterArray, (), (override));
    MOCK_METHOD(void, exitArray, (), (override));
    MOCK_METHOD(void, enterObject, (), (override));
    MOCK_METHOD(void, exitObject, (), (override));
    MOCK_METHOD(void, enterKey, (const char* key, ssize_t size), (override));
    MOCK_METHOD(void, enterKey, (std::string&& key), (override));
    MOCK_METHOD(void, finished, (), (override));
};

}   // namespace finalmq
