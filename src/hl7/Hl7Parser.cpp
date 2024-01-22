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

#include "finalmq/hl7/Hl7Parser.h"

#include <climits>
#include <limits>
#include <string>

#include <assert.h>

#include "finalmq/helpers/FmqDefines.h"

namespace finalmq
{
static const char SEGMENT_END = 0x0D; // '\r'

Hl7Parser::Hl7Parser()
{
}

char Hl7Parser::getChar(const char* str) const
{
    return ((str < m_end) ? *str : 0);
}

bool Hl7Parser::startParse(const char* str, ssize_t size)
{
    m_str = str;
    assert(size >= CHECK_ON_ZEROTERM);
    if (size >= CHECK_ON_ZEROTERM)
    {
        if (size == CHECK_ON_ZEROTERM)
        {
            size = strlen(str);
        }
        m_end = str + size;

        skipControlCharacters();

        if (size < 8)
        {
            return false;
        }

        if (m_str[0] == 'M' && m_str[1] == 'S' && m_str[2] == 'H')
        {
            m_delimiterField[0] = SEGMENT_END;  // segment delimiter '\r', 0x0D
            m_delimiterField[1] = m_str[3 + 0]; // |
            m_delimiterField[2] = m_str[3 + 1]; // ^
            m_delimiterField[3] = m_str[3 + 4]; // &
            m_delimiterRepeat = m_str[3 + 2];   // ~
            m_escape = m_str[3 + 3];            // '\\'
        }
        else
        {
            return false;
        }
    }
    return true;
}

int Hl7Parser::isDelimiter(char c) const
{
    if (c == 0)
    {
        return -1;
    }
    int i;
    for (i = 0; i < LAYER_MAX; ++i)
    {
        if (c == m_delimiterField[i])
        {
            break;
        }
    }

    return i;
}

void Hl7Parser::getSegmentId(std::string& token) const
{
    token.clear();
    int i = 0;
    while (true)
    {
        char c = getChar(m_str + i);
        if (isDelimiter(c) == LAYER_MAX)
        {
            token += c;
        }
        else
        {
            break;
        }
        ++i;
    }
}

int Hl7Parser::isNextFieldFilled(int level, bool& filled)
{
    filled = false;
    int l = level;
    char c = this->getChar(m_str);
    if (c == m_delimiterRepeat)
    {
        filled = true;
    }
    else
    {
        l = isDelimiter(c);
        if (l > level)
        {
            filled = true;
            l = level;
        }
    }
    if (!filled)
    {
        ++m_str;
    }

    assert(l <= level);
    return l;
}

int Hl7Parser::parseToken(int level, std::string& token, bool& isarray)
{
    token.clear();
    isarray = false;
    int l = level;
    if (m_waitForDeleimiterField == 1)
    {
        token = std::string(m_str - 1, m_str);
    }
    else if (m_waitForDeleimiterField == 2)
    {
        if (m_str + 4 < m_end)
        {
            token = std::string(m_str, m_str + 4);
            m_str += 4 + 1;
        }
    }
    else
    {
        const char* start = m_str;
        while (true)
        {
            char c = this->getChar(m_str);
            if (c == m_delimiterRepeat)
            {
                token = deEscape(start, m_str);
                ++m_str;
                isarray = true;
                l = 1; // array is only on level = 1
                break;
            }
            else
            {
                l = isDelimiter(c);
                if (l < LAYER_MAX)
                {
                    token = deEscape(start, m_str);
                    if (l != -1)
                    {
                        ++m_str;
                    }
                    break;
                }
            }
            ++m_str;
        }
        if (l > level)
        {
            l = parseTillEndOfStructIntern(level, true, isarray);
        }
    }

    if (m_waitForDeleimiterField < 3)
    {
        ++m_waitForDeleimiterField;
    }

    assert(l <= level);
    return l;
}

int Hl7Parser::parseTokenArray(int level, std::vector<std::string>& array)
{
    array.clear();
    const char* start = m_str;
    int l = 0;
    bool filled = false;
    while (true)
    {
        char c = this->getChar(m_str);
        if (c == m_delimiterRepeat)
        {
            array.emplace_back(deEscape(start, m_str));
            ++m_str;
            start = m_str;
            filled = true;
        }
        else
        {
            l = isDelimiter(c);
            if (l < LAYER_MAX)
            {
                if (filled || (l > 1)) // array/repeated (~) is on layer 1 possible
                {
                    array.emplace_back(deEscape(start, m_str));
                }
                if (l != -1)
                {
                    ++m_str;
                }
                if (l <= level)
                {
                    break;
                }
            }
            if (l > level && l < LAYER_MAX)
            {
                bool isarray;
                l = parseTillEndOfStructIntern(level, true, isarray);
                if (l <= level && !isarray)
                {
                    break;
                }
                start = m_str;
            }
            else
            {
                ++m_str;
            }
            filled = true;
        }
    }
    assert(l <= level);
    return l;
}

int Hl7Parser::parseTillEndOfStruct(int level)
{
    bool isarrayDummy;
    return parseTillEndOfStructIntern(level, false, isarrayDummy);
}

int Hl7Parser::parseTillEndOfStructIntern(int level, bool stopOnArray, bool& isarray)
{
    isarray = false;
    char c;
    while ((c = this->getChar(m_str)) != 0)
    {
        int l = isDelimiter(c);
        if (l <= level)
        {
            if (l != -1)
            {
                ++m_str;
            }
            return l;
        }
        else if (stopOnArray && c == m_delimiterRepeat)
        {
            isarray = true;
            ++m_str;
            return 1;
        }
        ++m_str;
    }
    return -1;
}

const char* Hl7Parser::getCurrentPosition() const
{
    return m_str;
}

void Hl7Parser::skipControlCharacters()
{
    while (true)
    {
        char c = this->getChar(m_str);
        if (c < 0x20 && c != 0)
        {
            ++m_str;
        }
        else
        {
            break;
        }
    }
}

static unsigned char hex2char(char c)
{
    unsigned char num = 0;
    if ('0' <= c && c <= '9')
    {
        num = static_cast<unsigned char>(c - '0');
    }
    else if ('a' <= c && c <= 'f')
    {
        num = static_cast<unsigned char>((c - 'a') + 10);
    }
    else if ('A' <= c && c <= 'F')
    {
        num = static_cast<unsigned char>((c - 'A') + 10);
    }
    else
    {
        num = static_cast<unsigned char>(0xff);
    }
    return num;
}

std::string Hl7Parser::deEscape(const char* start, const char* end) const
{
    std::string dest;
    const char* src = start;
    while (src < end)
    {
        char c = *src;
        if (c == m_escape)
        {
            ++src;
            if (src >= end)
            {
                return dest;
            }
            c = *src;
            switch(c)
            {
                case 'E':
                    dest += m_escape;
                    ++src;
                    break;
                case 'F':
                    dest += m_delimiterField[1]; // |
                    ++src;
                    break;
                case 'R':
                    dest += m_delimiterRepeat; // ~
                    ++src;
                    break;
                case 'S':
                    dest += m_delimiterField[2]; // ^
                    ++src;
                    break;
                case 'T':
                    dest += m_delimiterField[3]; // &
                    ++src;
                    break;
                case 'X':
                    while (true)
                    {
                        ++src;
                        if (src >= end)
                        {
                            break;
                        }
                        c = *src;
                        if (c == '\\')
                        {
                            break;
                        }
                        unsigned char num = hex2char(c);
                        if (num == static_cast<unsigned char>(0xff))
                        {
                            return dest;
                        }
                        unsigned char d = num;
                        ++src;
                        if (src >= end)
                        {
                            break;
                        }
                        c = *src;
                        num = hex2char(c);
                        if (num == static_cast<unsigned char>(0xff))
                        {
                            return dest;
                        }
                        d = static_cast<unsigned char>(d << 4);
                        d |= num;
                        dest += d;
                    }
                    break;
                default:
                    break;
            }
        }
        else
        {
            dest += c;
        }
        ++src;
    }

    return dest;
}

} // namespace finalmq
