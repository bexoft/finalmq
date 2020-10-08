#pragma once


#include "serialize/IParserVisitor.h"


#include "gmock/gmock.h"

class MockIParserVisitor : public IParserVisitor
{
public:
    MOCK_METHOD(void, notifyError, (const char* str, const char* message), (override));
    MOCK_METHOD(void, finished, (), (override));

    MOCK_METHOD(void, enterStruct, (const MetaField& field), (override));
    MOCK_METHOD(void, exitStruct, (const MetaField& field), (override));

    MOCK_METHOD(void, enterArrayStruct, (const MetaField& field), (override));
    MOCK_METHOD(void, exitArrayStruct, (const MetaField& field), (override));

    MOCK_METHOD(void, enterBool, (const MetaField& field, bool value), (override));
    MOCK_METHOD(void, enterInt32, (const MetaField& field, std::int32_t value), (override));
    MOCK_METHOD(void, enterUInt32, (const MetaField& field, std::uint32_t value), (override));
    MOCK_METHOD(void, enterInt64, (const MetaField& field, std::int64_t value), (override));
    MOCK_METHOD(void, enterUInt64, (const MetaField& field, std::uint64_t value), (override));
    MOCK_METHOD(void, enterFloat, (const MetaField& field, float value), (override));
    MOCK_METHOD(void, enterDouble, (const MetaField& field, double value), (override));
    MOCK_METHOD(void, enterString, (const MetaField& field, std::string&& value));
    MOCK_METHOD(void, enterString, (const MetaField& field, const char* value, int size));
    MOCK_METHOD(void, enterBytes, (const MetaField& field, Bytes&& value));
    MOCK_METHOD(void, enterBytes, (const MetaField& field, const unsigned char* value, int size));
    MOCK_METHOD(void, enterEnum, (const MetaField& field, std::int32_t value), (override));
    MOCK_METHOD(void, enterEnum, (const MetaField& field, std::string&& value));
    MOCK_METHOD(void, enterEnum, (const MetaField& field, const char* value, int size));

    MOCK_METHOD(void, enterArrayBoolMove, (const MetaField& field, std::vector<bool>&& value), (override));
    MOCK_METHOD(void, enterArrayBool, (const MetaField& field, const std::vector<bool>& value), (override));
    MOCK_METHOD(void, enterArrayInt32, (const MetaField& field, std::vector<std::int32_t>&& value), (override));
    MOCK_METHOD(void, enterArrayInt32, (const MetaField& field, const std::int32_t* value, int size), (override));
    MOCK_METHOD(void, enterArrayUInt32, (const MetaField& field, std::vector<std::uint32_t>&& value), (override));
    MOCK_METHOD(void, enterArrayUInt32, (const MetaField& field, const std::uint32_t* value, int size), (override));
    MOCK_METHOD(void, enterArrayInt64, (const MetaField& field, std::vector<std::int64_t>&& value), (override));
    MOCK_METHOD(void, enterArrayInt64, (const MetaField& field, const std::int64_t* value, int size), (override));
    MOCK_METHOD(void, enterArrayUInt64, (const MetaField& field, std::vector<std::uint64_t>&& value), (override));
    MOCK_METHOD(void, enterArrayUInt64, (const MetaField& field, const std::uint64_t* value, int size), (override));
    MOCK_METHOD(void, enterArrayFloat, (const MetaField& field, std::vector<float>&& value), (override));
    MOCK_METHOD(void, enterArrayFloat, (const MetaField& field, const float* value, int size), (override));
    MOCK_METHOD(void, enterArrayDouble, (const MetaField& field, std::vector<double>&& value), (override));
    MOCK_METHOD(void, enterArrayDouble, (const MetaField& field, const double* value, int size), (override));
    MOCK_METHOD(void, enterArrayStringMove, (const MetaField& field, std::vector<std::string>&& value), (override));
    MOCK_METHOD(void, enterArrayString, (const MetaField& field, const std::vector<std::string>& value), (override));
    MOCK_METHOD(void, enterArrayBytesMove, (const MetaField& field, std::vector<Bytes>&& value), (override));
    MOCK_METHOD(void, enterArrayBytes, (const MetaField& field, const std::vector<Bytes>& value), (override));
    MOCK_METHOD(void, enterArrayEnum, (const MetaField& field, std::vector<std::int32_t>&& value), (override));
    MOCK_METHOD(void, enterArrayEnum, (const MetaField& field, const std::int32_t* value, int size), (override));
    MOCK_METHOD(void, enterArrayEnumMove, (const MetaField& field, std::vector<std::string>&& value), (override));
    MOCK_METHOD(void, enterArrayEnum, (const MetaField& field, const std::vector<std::string>& value), (override));
};

