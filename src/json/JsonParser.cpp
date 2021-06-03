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
#include "finalmq/helpers/FmqDefines.h"

#include <string>
#include <climits>
#include <limits>
#include <assert.h>



namespace finalmq {

static const int IGNORE_SIZE = -2;



JsonParser::JsonParser(IJsonParserVisitor& visitor)
    : m_visitor(visitor)
{
}


char JsonParser::getChar(const char* str) const
{
    return ((str < m_end) ? *str : 0);
}



const char* JsonParser::parseWhiteSpace(const char* str)
{
    char c;
    while ((c = this->getChar(str)) != 0)
    {
        switch (c)
        {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            str++;
            break;
        default:
            return str;
            break;
        }
    }
    return str;
}


const char* JsonParser::parse(const char* str, ssize_t size)
{
    const char* ret = parseValue(str, size);
    m_visitor.finished();
    return ret;
}



const char* JsonParser::parseValue(const char* str, ssize_t size)
{
    if (size >= CHECK_ON_ZEROTERM)
    {
        if (size == CHECK_ON_ZEROTERM)
        {
            m_end = (char*)nullptr - 1;
        }
        else
        {
            m_end = str + size;
        }
    }
    str = parseWhiteSpace(str);
    char c = getChar(str);
    if (c == 0)
    {
        m_visitor.syntaxError(str, "value expected");
        return nullptr;
    }
    switch (c)
    {
    // string
    case '\"':
        str = parseString(str, false);
        break;
    // object
    case '{':
        str = parseObject(str);
        break;
    case '[':
        str = parseArray(str);
        break;
    case 'n':
        str = parseNull(str);
        break;
    case 't':
        str = parseTrue(str);
        break;
    case 'f':
        str = parseFalse(str);
        break;
    default:
        str = parseNumber(str);
        break;
    }
    if (str)
    {
        str = parseWhiteSpace(str);
        return str;
    }
    return nullptr;
}



const char* JsonParser::cmpString(const char* str, const char* strCmp)
{
    char c;
    while ((c = *strCmp))
    {
        if (getChar(str) != c)
        {
            std::string message;
            message += c;
            message += " expected";
            m_visitor.syntaxError(str,  message.c_str());
            return nullptr;
        }
        str++;
        strCmp++;
    }

    return str;
}


const char* JsonParser::parseNull(const char* str)
{
    str = cmpString(str, "null");
    if (str)
    {
        m_visitor.enterNull();
    }
    return str;
}

const char* JsonParser::parseTrue(const char* str)
{
    str = cmpString(str, "true");
    if (str)
    {
        m_visitor.enterBool(true);
    }
    return str;
}

const char* JsonParser::parseFalse(const char* str)
{
    str = cmpString(str, "false");
    if (str)
    {
        m_visitor.enterBool(false);
    }
    return str;
}




const char* JsonParser::parseNumber(const char* str)
{
    const char* first = str;
    char c = getChar(str);
    bool isNegative = false;
    if (c == '-')
    {
        isNegative = true;
        str++;
        c = getChar(str);
        if (c >= '0' && c <= '9')
        {
            str++;
        }
        else
        {
            m_visitor.syntaxError(str,  "digit expected");
            return nullptr;
        }
    }
    else if ((c >= '0' && c <= '9') || (c == '-'))
    {
        str++;
    }
    else
    {
        m_visitor.syntaxError(str,  "digit expected");
        return nullptr;
    }

    bool isFloat = false;
    while ((c = getChar(str)) != 0)
    {
        if ((c >= '0' && c <= '9') || (c == '+') || (c == '-') || (c == 'e') || (c == 'E'))
        {
            str++;
        }
        else if (c == '.')
        {
            str++;
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
        if (res != str)
        {
            m_visitor.syntaxError(res,  "wrong number format");
            return nullptr;
        }
        m_visitor.enterDouble(value);
    }
    else if (isNegative)
    {
        long long value = strtoll(first, &res, 10);
        if (res != str)
        {
            m_visitor.syntaxError(res,  "wrong number format");
            return nullptr;
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
        if (res != str)
        {
            m_visitor.syntaxError(res,  "wrong number format");
            return nullptr;
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
    str = res;

    return str;
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


const char* JsonParser::parseUEscape(const char* str, std::uint32_t& value)
{
    value = 0;
    for (int i = 0; i < 4; ++i)
    {
        std::int32_t v = getHexDigit(getChar(str));
        if (v == -1)
        {
            m_visitor.syntaxError(str, "invalid u escape");
            return nullptr;
        }
        v <<= 12 - 4*i;
        value += v;
        str++;
    }
    return str;
}


const char* JsonParser::parseString(const char* str, bool key)
{
    // skip '"'
    str++;

    const char* strBegin = str;

    // try fast parse. it is fast when there is no escape character in the string
    char c;
    while ((c = getChar(str)) != 0)
    {
        if (c == '\"')
        {
            ssize_t size = str - strBegin;
            if (key)
            {
                m_visitor.enterKey(strBegin, size);
            }
            else
            {
                m_visitor.enterString(strBegin, size);
            }
            str++;
            return str;
        }
        else if (c == '\\')
        {
            break;
        }
        str++;
    }

    if (c == 0)
    {
        m_visitor.syntaxError(str, "'\"' expected");
        return nullptr;
    }

    // fast parse was not possible, go ahead with escaping
    std::string dest(strBegin, str);
    while ((c = getChar(str)) != 0)
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
            str++;
            return str;
        }
        else if (c == '\\')
        {
            str++;
            if ((c = getChar(str)) != 0)
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
                        str++;
                        std::uint32_t num = 0;
                        str = parseUEscape(str, num);
                        if (str == nullptr)
                        {
                            return nullptr;
                        }
                        if (num >= 0xD800 && num <= 0xDBFF)
                        {
                            if (getChar(str) != '\\')
                            {
                                m_visitor.syntaxError(str, "'\\' expected");
                                return nullptr;
                            }
                            str++;
                            if (getChar(str) != 'u')
                            {
                                m_visitor.syntaxError(str, "'u' expected");
                                return nullptr;
                            }
                            str++;
                            std::uint32_t num2 = 0;
                            str = parseUEscape(str, num2);
                            if (str == nullptr)
                            {
                                return nullptr;
                            }
                            if (num2 < 0xDC00 || num2 > 0xDFFF)
                            {
                                m_visitor.syntaxError(str, "wrong utf16 value");
                                return nullptr;
                            }
                            //num += num2 << 16;
                            num = (((num - 0xD800) << 10) | (num2 - 0xDC00)) + 0x10000;
                        }
                        else if (num > 0xDBFF && num <= 0xDFFF)
                        {
                            m_visitor.syntaxError(str, "wrong utf16 valueh");
                            return nullptr;
                        }
                        str--;

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
        str++;
    }
    m_visitor.syntaxError(str, "'\"' expected");
    return nullptr;
}


const char* JsonParser::parseArray(const char* str)
{
    m_visitor.enterArray();

    // skip '['
    str++;

    while (getChar(str) != 0)
    {
        str = parseWhiteSpace(str);
        if (getChar(str) == ']')
        {
            str++;
            m_visitor.exitArray();
            return str;
        }
        str = parseValue(str, IGNORE_SIZE);
        if (str == nullptr)
        {
            return nullptr;
        }
        char c = getChar(str);
        if (c != ',' && c != ']')
        {
            m_visitor.syntaxError(str, "',' or ']' expected");
            return nullptr;
        }
        if (c == ',')
        {
            str++;
        }
    }
    m_visitor.syntaxError(str, "',' or ']' expected");
    return nullptr;
}


const char* JsonParser::parseObject(const char* str)
{
    m_visitor.enterObject();
    // skip '{'
    str++;
    str = parseWhiteSpace(str);

    while (getChar(str) != 0)
    {
        str = parseWhiteSpace(str);
        char c = getChar(str);
        if (c == '}')
        {
            str++;
            m_visitor.exitObject();
            return str;
        }
        if (c != '\"')
        {
            m_visitor.syntaxError(str, "'\"' for key expected");
            return nullptr;
        }
        str = parseString(str, true);
        if (str == nullptr)
        {
            return nullptr;
        }
        str = parseWhiteSpace(str);

        if (getChar(str) != ':')
        {
            m_visitor.syntaxError(str, "':' expected");
            return nullptr;
        }

        str++;
        str = parseValue(str, IGNORE_SIZE);
        if (str == nullptr)
        {
            return nullptr;
        }
        c = getChar(str);
        if (c != ',' && c != '}')
        {
            m_visitor.syntaxError(str, "',' or '}' expected");
            return nullptr;
        }
        if (c == ',')
        {
            str++;
        }
    }
    m_visitor.syntaxError(str, "',' or '}' expected");
    return nullptr;
}

}   // namespace finalmq
