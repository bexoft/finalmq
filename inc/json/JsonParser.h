#pragma once

#include <cstdint>
#include <string>

struct IJsonParserVisitor
{
    virtual ~IJsonParserVisitor() {}
    virtual void syntaxError(const char* str, const char* message) = 0;
    virtual void enterNull() = 0;
    virtual void enterBool(bool value) = 0;
    virtual void enterInt32(std::int32_t value) = 0;
    virtual void enterUInt32(std::uint32_t value) = 0;
    virtual void enterInt64(std::int64_t value) = 0;
    virtual void enterUInt64(std::uint64_t value) = 0;
    virtual void enterDouble(double value) = 0;
    virtual void enterString(const char* value, int size) = 0;
    virtual void enterString(std::string&& value) = 0;
    virtual void enterArray() = 0;
    virtual void exitArray() = 0;
    virtual void enterObject() = 0;
    virtual void exitObject() = 0;
    virtual void enterKey(const char* key, int size) = 0;
    virtual void enterKey(std::string&& key) = 0;
    virtual void finished() = 0;
};


static const int CHECK_ON_ZEROTERM = -1;


class JsonParser
{
public:
    JsonParser(IJsonParserVisitor& visitor);

    const char* parse(const char* str, int size = CHECK_ON_ZEROTERM);

private:
    const char* parseValue(const char* str, int size = CHECK_ON_ZEROTERM);
    inline const char* parseWhiteSpace(const char* str);
    const char* parseObject(const char* str);
    const char* parseArray(const char* str);
    const char* parseNumber(const char* str);
    const char* parseString(const char* str, bool key);
    const char* parseNull(const char* str);
    const char* parseTrue(const char* str);
    const char* parseFalse(const char* str);
    const char* cmpString(const char* str, const char* strCmp);
    const char* parseUEscape(const char* str, std::uint32_t& value);
    inline char getChar(const char* str) const;

    IJsonParserVisitor& m_visitor;
    const char* m_end = nullptr;
};

