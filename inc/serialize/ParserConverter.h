#pragma once

#include "metadata/MetaStruct.h"
#include "serialize/IParserVisitor.h"



class ParserConverter : public IParserVisitor
{
public:
    ParserConverter(IParserVisitor* visitor = nullptr);
    void setVisitor(IParserVisitor& visitor);

private:
    // IParserVisitor
    virtual void notifyError(const char* str, const char* message) override;
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
    virtual void enterString(const MetaField& field, const char* value, int size) override;
    virtual void enterBytes(const MetaField& field, Bytes&& value) override;
    virtual void enterBytes(const MetaField& field, const BytesElement* value, int size) override;
    virtual void enterEnum(const MetaField& field, std::int32_t value) override;
    virtual void enterEnum(const MetaField& field, std::string&& value) override;
    virtual void enterEnum(const MetaField& field, const char* value, int size) override;

    virtual void enterArrayBoolMove(const MetaField& field, std::vector<bool>&& value) override;
    virtual void enterArrayBool(const MetaField& field, const std::vector<bool>& value) override;
    virtual void enterArrayInt32(const MetaField& field, std::vector<std::int32_t>&& value) override;
    virtual void enterArrayInt32(const MetaField& field, const std::int32_t* value, int size) override;
    virtual void enterArrayUInt32(const MetaField& field, std::vector<std::uint32_t>&& value) override;
    virtual void enterArrayUInt32(const MetaField& field, const std::uint32_t* value, int size) override;
    virtual void enterArrayInt64(const MetaField& field, std::vector<std::int64_t>&& value) override;
    virtual void enterArrayInt64(const MetaField& field, const std::int64_t* value, int size) override;
    virtual void enterArrayUInt64(const MetaField& field, std::vector<std::uint64_t>&& value) override;
    virtual void enterArrayUInt64(const MetaField& field, const std::uint64_t* value, int size) override;
    virtual void enterArrayFloat(const MetaField& field, std::vector<float>&& value) override;
    virtual void enterArrayFloat(const MetaField& field, const float* value, int size) override;
    virtual void enterArrayDouble(const MetaField& field, std::vector<double>&& value) override;
    virtual void enterArrayDouble(const MetaField& field, const double* value, int size) override;
    virtual void enterArrayStringMove(const MetaField& field, std::vector<std::string>&& value) override;
    virtual void enterArrayString(const MetaField& field, const std::vector<std::string>& value) override;
    virtual void enterArrayBytesMove(const MetaField& field, std::vector<Bytes>&& value) override;
    virtual void enterArrayBytes(const MetaField& field, const std::vector<Bytes>& value) override;
    virtual void enterArrayEnum(const MetaField& field, std::vector<std::int32_t>&& value) override;
    virtual void enterArrayEnum(const MetaField& field, const std::int32_t* value, int size) override;
    virtual void enterArrayEnumMove(const MetaField& field, std::vector<std::string>&& value) override;
    virtual void enterArrayEnum(const MetaField& field, const std::vector<std::string>& value) override;

    template<class T>
    void convertNumber(const MetaField& field, T value);
    template<class T>
    void convertArraytNumber(const MetaField& field, const T* value, int size);
    template<class T>
    void convertArraytNumber(const MetaField& field, const std::vector<T>& value);

    void convertString(const MetaField& field, const char* value, int size);
    void convertArraytString(const MetaField& field, const std::vector<std::string>& value);

    IParserVisitor*     m_visitor = nullptr;
};
