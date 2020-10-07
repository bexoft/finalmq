#pragma once

#include "json/JsonParser.h"
#include "helpers/IZeroCopyBuffer.h"


class JsonBuilder : public IJsonParserVisitor
{
public:
    JsonBuilder(IZeroCopyBuffer& buffer, int maxBlockSize = 1024);
    ~JsonBuilder();

private:
    // IJsonParserVisitor
    virtual void syntaxError(const char* str, const char* message) override;
    virtual void enterNull() override;
    virtual void enterBool(bool value) override;
    virtual void enterInt32(std::int32_t value) override;
    virtual void enterUInt32(std::uint32_t value) override;
    virtual void enterInt64(std::int64_t value) override;
    virtual void enterUInt64(std::uint64_t value) override;
    virtual void enterDouble(double value) override;
    virtual void enterString(const char* value, int size) override;
    virtual void enterString(std::string&& value) override;
    virtual void enterArray() override;
    virtual void exitArray() override;
    virtual void enterObject() override;
    virtual void exitObject() override;
    virtual void enterKey(const char* key, int size) override;
    virtual void enterKey(std::string&& key) override;
    virtual void finished() override;

    void reserveSpace(int space);
    void resizeBuffer();
    void correctComma();
    void escapeString(const char* str, int size);

    IZeroCopyBuffer&        m_zeroCopybuffer;
    int                     m_maxBlockSize = 1024;
    char*                   m_bufferStart = nullptr;
    char*                   m_buffer = nullptr;
    char*                   m_bufferEnd = nullptr;
};
