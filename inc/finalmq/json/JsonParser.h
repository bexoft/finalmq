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

#include "finalmq/helpers/FmqDefines.h"

#include <cstdint>
#include <string>

namespace finalmq {

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
    virtual void enterString(const char* value, ssize_t size) = 0;
    virtual void enterString(std::string&& value) = 0;
    virtual void enterArray() = 0;
    virtual void exitArray() = 0;
    virtual void enterObject() = 0;
    virtual void exitObject() = 0;
    virtual void enterKey(const char* key, ssize_t size) = 0;
    virtual void enterKey(std::string&& key) = 0;
    virtual void finished() = 0;
};



class SYMBOLEXP JsonParser
{
public:
    JsonParser(IJsonParserVisitor& visitor);

    const char* parse(const char* str, ssize_t size = CHECK_ON_ZEROTERM);
    const char* getCurrentPosition() const;

private:
    void parseValue();
    inline void parseWhiteSpace();
    void parseObject();
    void parseArray();
    void parseNumber();
    void parseString(bool key);
    void parseNull();
    void parseTrue();
    void parseFalse();
    void cmpString(const char* strCmp);
    void parseUEscape(std::uint32_t& value);
    inline char getChar(const char* str) const;

    IJsonParserVisitor& m_visitor;
    const char* m_str = nullptr;
    const char* m_end = nullptr;
};

}   // namespace finalmq
