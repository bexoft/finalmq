#pragma once


#include "json/JsonParser.h"


#include "gmock/gmock.h"

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
    MOCK_METHOD(void, enterString, (const char* value, int size), (override));
    MOCK_METHOD(void, enterString, (std::string&& value), (override));
    MOCK_METHOD(void, enterArray, (), (override));
    MOCK_METHOD(void, exitArray, (), (override));
    MOCK_METHOD(void, enterObject, (), (override));
    MOCK_METHOD(void, exitObject, (), (override));
    MOCK_METHOD(void, enterKey, (const char* key, int size), (override));
    MOCK_METHOD(void, enterKey, (std::string&& key), (override));
    MOCK_METHOD(void, finished, (), (override));
};
