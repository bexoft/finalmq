#pragma once

#include "metadata/MetaStruct.h"

typedef char                       BytesElement;
typedef std::vector<BytesElement>  Bytes;


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
    virtual void enterString(const MetaField& field, const char* value, int size) = 0;
    virtual void enterBytes(const MetaField& field, Bytes&& value) = 0;
    virtual void enterBytes(const MetaField& field, const BytesElement* value, int size) = 0;
    virtual void enterEnum(const MetaField& field, std::int32_t value) = 0;
    virtual void enterEnum(const MetaField& field, std::string&& value) = 0;
    virtual void enterEnum(const MetaField& field, const char* value, int size) = 0;

    virtual void enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value) = 0;
    virtual void enterArrayBool(const MetaField& field, const std::vector<bool>& value) = 0;
    virtual void enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value) = 0;
    virtual void enterArrayInt32(const MetaField& field, const std::int32_t* value, int size) = 0;
    virtual void enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value) = 0;
    virtual void enterArrayUInt32(const MetaField& field, const std::uint32_t* value, int size) = 0;
    virtual void enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value) = 0;
    virtual void enterArrayInt64(const MetaField& field, const std::int64_t* value, int size) = 0;
    virtual void enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value) = 0;
    virtual void enterArrayUInt64(const MetaField& field, const std::uint64_t* value, int size) = 0;
    virtual void enterArrayFloat(const MetaField& field, std::vector<float>&& value) = 0;
    virtual void enterArrayFloat(const MetaField& field, const float* value, int size) = 0;
    virtual void enterArrayDouble(const MetaField& field, std::vector<double>&& value) = 0;
    virtual void enterArrayDouble(const MetaField& field, const double* value, int size) = 0;
    virtual void enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value) = 0;
    virtual void enterArrayString(const MetaField& field, const std::vector<std::string>& value) = 0;
    virtual void enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value) = 0;
    virtual void enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value) = 0;
    virtual void enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value) = 0;
    virtual void enterArrayEnum(const MetaField& field, const std::int32_t* value, int size) = 0;
    virtual void enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value) = 0;
    virtual void enterArrayEnum(const MetaField& field, const std::vector<std::string>& value) = 0;
};


