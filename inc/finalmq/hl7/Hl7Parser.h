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

struct IHl7ParserVisitor
{
    virtual ~IHl7ParserVisitor() {}
    virtual void syntaxError(const char* str, const char* message) = 0;
    virtual void enterStruct() = 0;
    virtual void exitStruct() = 0;
    virtual void enterArray() = 0;
    virtual void exitArray() = 0;
    virtual void enterNull() = 0;
    virtual void enterEmpty() = 0;
    virtual void enterInt64(std::int64_t value) = 0;
    virtual void enterUInt64(std::uint64_t value) = 0;
    virtual void enterDouble(double value) = 0;
    virtual void enterString(const char* value, ssize_t size) = 0;
    virtual void enterString(std::string&& value) = 0;
    virtual void finished() = 0;
};


static const ssize_t CHECK_ON_ZEROTERM = -1;

/*
class SYMBOLEXP JsonParser
{
public:
    JsonParser(IJsonParserVisitor& visitor);

    const char* parse(const char* str, ssize_t size = CHECK_ON_ZEROTERM);

private:
    const char* parseValue(const char* str);
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
*/
}   // namespace finalmq
