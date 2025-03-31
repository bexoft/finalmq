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

#include "finalmq/json/JsonParser.h"

#include <climits>
#include <limits>
#include <string>

#include <assert.h>

#include "finalmq/helpers/FmqDefines.h"

namespace finalmq
{
JsonParser::JsonParser(IJsonParserVisitor& visitor)
    : m_visitor(visitor)
{
}

char JsonParser::getChar(const char* str) const
{
    return ((str < m_end) ? *str : 0);
}

void JsonParser::parseWhiteSpace()
{
    char c;
    while ((c = this->getChar(m_str)) != 0)
    {
        switch(c)
        {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                m_str++;
                break;
            default:
                return;
                break;
        }
    }
}

const char* JsonParser::parse(const char* str, ssize_t size)
{
    if (size >= CHECK_ON_ZEROTERM)
    {
        if (size == CHECK_ON_ZEROTERM)
        {
            size = strlen(str);
        }

        m_end = str + size;
    }
    m_str = str;
    parseValue();
    m_visitor.finished();
    return m_str;
}

const char* JsonParser::getCurrentPosition() const
{
    return m_str;
}


void JsonParser::parseValue()
{
    parseWhiteSpace();
    char c = getChar(m_str);
    if (c == 0)
    {
        m_visitor.syntaxError(m_str, "value expected");
        m_str = nullptr;
        return;
    }
    switch(c)
    {
        // string
        case '\"':
            parseString(false);
            break;
        // object
        case '{':
            parseObject();
            break;
        case '[':
            parseArray();
            break;
        case 'n':
            parseNull();
            break;
        case 't':
            parseTrue();
            break;
        case 'f':
            parseFalse();
            break;
        default:
            parseNumber();
            break;
    }
    if (m_str)
    {
        parseWhiteSpace();
    }
}

void JsonParser::cmpString(const char* strCmp)
{
    char c;
    while ((c = *strCmp))
    {
        if (getChar(m_str) != c)
        {
            std::string message;
            message += c;
            message += " expected";
            m_visitor.syntaxError(m_str, message.c_str());
            m_str = nullptr;
            return;
        }
        m_str++;
        strCmp++;
    }
}

void JsonParser::parseNull()
{
    cmpString("null");
    if (m_str)
    {
        m_visitor.enterNull();
    }
}

void JsonParser::parseTrue()
{
    cmpString("true");
    if (m_str)
    {
        m_visitor.enterBool(true);
    }
}

void JsonParser::parseFalse()
{
    cmpString("false");
    if (m_str)
    {
        m_visitor.enterBool(false);
    }
}

void JsonParser::parseNumber()
{
    const char* first = m_str;
    char c = getChar(m_str);
    bool isNegative = false;
    if (c == '-')
    {
        isNegative = true;
        m_str++;
        c = getChar(m_str);
        if (c >= '0' && c <= '9')
        {
            m_str++;
        }
        else
        {
            m_visitor.syntaxError(m_str, "digit expected");
            m_str = nullptr;
            return;
        }
    }
    else if ((c >= '0' && c <= '9') || (c == '-'))
    {
        m_str++;
    }
    else
    {
        m_visitor.syntaxError(m_str, "digit expected");
        m_str = nullptr;
        return;
    }

    bool isFloat = false;
    while ((c = getChar(m_str)) != 0)
    {
        if ((c >= '0' && c <= '9') || (c == '+') || (c == '-'))
        {
            m_str++;
        }
        else if ((c == '.') || (c == 'e') || (c == 'E'))
        {
            m_str++;
            isFloat = true;
        }
        else
        {
            break;
        }
    }

    char* res = nullptr;
    if (isFloat)
    {
        double value = strtof64(first, &res);
        if (res != m_str)
        {
            m_visitor.syntaxError(res, "wrong number format");
            m_str = nullptr;
            return;
        }
        m_visitor.enterDouble(value);
    }
    else if (isNegative)
    {
        long long value = strtoll(first, &res, 10);
        if (res != m_str)
        {
            m_visitor.syntaxError(res, "wrong number format");
            m_str = nullptr;
            return;
        }
        assert(value < 0);
        if (value >= INT_MIN)
        {
            m_visitor.enterInt32(static_cast<std::int32_t>(value));
        }
        else
        {
            m_visitor.enterInt64(value);
        }
    }
    else
    {
        unsigned long long value = strtoull(first, &res, 10);
        if (res != m_str)
        {
            m_visitor.syntaxError(res, "wrong number format");
            m_str = nullptr;
            return;
        }
        if (value <= INT_MAX)
        {
            m_visitor.enterUInt32(static_cast<std::uint32_t>(value));
        }
        else
        {
            m_visitor.enterUInt64(value);
        }
    }

    assert(res);
    m_str = res;
}

static std::int32_t getHexDigit(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    else if (c >= 'a' && c <= 'f')
    {
        return 10 + c - 'a';
    }
    else if (c >= 'A' && c <= 'F')
    {
        return 10 + c - 'A';
    }
    return -1;
}

void JsonParser::parseUEscape(std::uint32_t& value)
{
    value = 0;
    for (int i = 0; i < 4; ++i)
    {
        std::int32_t v = getHexDigit(getChar(m_str));
        if (v == -1)
        {
            m_visitor.syntaxError(m_str, "invalid u escape");
            m_str = nullptr;
            return;
        }
        v <<= 12 - 4 * i;
        value += v;
        m_str++;
    }
}

void JsonParser::parseString(bool key)
{
    // skip '"'
    m_str++;

    const char* strBegin = m_str;

    // try fast parse. it is fast when there is no escape character in the string
    char c;
    while ((c = getChar(m_str)) != 0)
    {
        if (c == '\"')
        {
            ssize_t size = m_str - strBegin;
            if (key)
            {
                m_visitor.enterKey(strBegin, size);
            }
            else
            {
                m_visitor.enterString(strBegin, size);
            }
            m_str++;
            return;
        }
        else if (c == '\\')
        {
            break;
        }
        m_str++;
    }

    if (c == 0)
    {
        m_visitor.syntaxError(m_str, "'\"' expected");
        m_str = nullptr;
        return;
    }

    // fast parse was not possible, go ahead with escaping
    std::string dest(strBegin, m_str);
    while ((c = getChar(m_str)) != 0)
    {
        if (c == '\"')
        {
            if (key)
            {
                m_visitor.enterKey(std::move(dest));
            }
            else
            {
                m_visitor.enterString(std::move(dest));
            }
            m_str++;
            return;
        }
        else if (c == '\\')
        {
            m_str++;
            if ((c = getChar(m_str)) != 0)
            {
                switch(c)
                {
                    case '\"':
                    case '\\':
                    case '/':
                        dest += c;
                        break;
                    case 'b':
                        dest += '\b';
                        break;
                    case 'f':
                        dest += '\f';
                        break;
                    case 'n':
                        dest += '\n';
                        break;
                    case 'r':
                        dest += '\r';
                        break;
                    case 't':
                        dest += '\t';
                        break;
                    case 'u':
                    {
                        m_str++;
                        std::uint32_t num = 0;
                        parseUEscape(num);
                        if (m_str == nullptr)
                        {
                            return;
                        }
                        if (num >= 0xD800 && num <= 0xDBFF)
                        {
                            if (getChar(m_str) != '\\')
                            {
                                m_visitor.syntaxError(m_str, "'\\' expected");
                                m_str = nullptr;
                                return;
                            }
                            m_str++;
                            if (getChar(m_str) != 'u')
                            {
                                m_visitor.syntaxError(m_str, "'u' expected");
                                m_str = nullptr;
                                return;
                            }
                            m_str++;
                            std::uint32_t num2 = 0;
                            parseUEscape(num2);
                            if (m_str == nullptr)
                            {
                                return;
                            }
                            if (num2 < 0xDC00 || num2 > 0xDFFF)
                            {
                                m_visitor.syntaxError(m_str, "wrong utf16 value");
                                m_str = nullptr;
                                return;
                            }
                            //num += num2 << 16;
                            num = (((num - 0xD800) << 10) | (num2 - 0xDC00)) + 0x10000;
                        }
                        else if (num > 0xDBFF && num <= 0xDFFF)
                        {
                            m_visitor.syntaxError(m_str, "wrong utf16 valueh");
                            m_str = nullptr;
                            return;
                        }
                        m_str--;

                        if (num <= 0x7F)
                        {
                            dest += static_cast<char>(num & 0xff);
                        }
                        else if (num <= 0x7FF)
                        {
                            dest += static_cast<char>(0xC0 | ((num >> 6) & 0xFF));
                            dest += static_cast<char>(0x80 | ((num & 0x3F)));
                        }
                        else if (num <= 0xFFFF)
                        {
                            dest += static_cast<char>(0xE0 | ((num >> 12) & 0xFF));
                            dest += static_cast<char>(0x80 | ((num >> 6) & 0x3F));
                            dest += static_cast<char>(0x80 | (num & 0x3F));
                        }
                        else
                        {
                            assert(num <= 0x10FFFF);
                            dest += static_cast<char>(0xF0 | ((num >> 18) & 0xFF));
                            dest += static_cast<char>(0x80 | ((num >> 12) & 0x3F));
                            dest += static_cast<char>(0x80 | ((num >> 6) & 0x3F));
                            dest += static_cast<char>(0x80 | (num & 0x3F));
                        }
                    }
                    break;
                    default:
                        dest += '\\';
                        dest += c;
                        break;
                }
            }
        }
        else
        {
            dest += c;
        }
        m_str++;
    }
    m_visitor.syntaxError(m_str, "'\"' expected");
    m_str = nullptr;
}

void JsonParser::parseArray()
{
    m_visitor.enterArray();

    // skip '['
    m_str++;

    while (getChar(m_str) != 0)
    {
        parseWhiteSpace();
        if (getChar(m_str) == ']')
        {
            m_str++;
            m_visitor.exitArray();
            return;
        }
        parseValue();
        if (m_str == nullptr)
        {
            return;
        }
        char c = getChar(m_str);
        if (c != ',' && c != ']')
        {
            m_visitor.syntaxError(m_str, "',' or ']' expected");
            m_str = nullptr;
            return;
        }
        if (c == ',')
        {
            m_str++;
        }
    }
    m_visitor.syntaxError(m_str, "',' or ']' expected");
    m_str = nullptr;
}

void JsonParser::parseObject()
{
    m_visitor.enterObject();
    // skip '{'
    m_str++;
    parseWhiteSpace();

    while (getChar(m_str) != 0)
    {
        parseWhiteSpace();
        char c = getChar(m_str);
        if (c == '}')
        {
            m_str++;
            m_visitor.exitObject();
            return;
        }
        if (c != '\"')
        {
            m_visitor.syntaxError(m_str, "'\"' for key expected");
            m_str = nullptr;
            return;
        }
        parseString(true);
        if (m_str == nullptr)
        {
            return;
        }
        parseWhiteSpace();

        if (getChar(m_str) != ':')
        {
            m_visitor.syntaxError(m_str, "':' expected");
            m_str = nullptr;
            return;
        }

        m_str++;
        parseValue();
        if (m_str == nullptr)
        {
            return;
        }
        c = getChar(m_str);
        if (c != ',' && c != '}')
        {
            m_visitor.syntaxError(m_str, "',' or '}' expected");
            m_str = nullptr;
            return;
        }
        if (c == ',')
        {
            m_str++;
        }
    }
    m_visitor.syntaxError(m_str, "',' or '}' expected");
    m_str = nullptr;
}

} // namespace finalmq
