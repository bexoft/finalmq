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


#include "finalmq/json/JsonBuilder.h"
#include "finalmq/conversions/itoa.h"
#include "finalmq/conversions/dtoa.h"
#include "finalmq/logger/LogStream.h"
#include "finalmq/helpers/ModulenameFinalmq.h"

#include <assert.h>
#include <string.h>
#include <iostream>


namespace finalmq {


JsonBuilder::JsonBuilder(IZeroCopyBuffer& buffer, int maxBlockSize)
    : m_zeroCopybuffer(buffer)
    , m_maxBlockSize(maxBlockSize)
{
}

JsonBuilder::~JsonBuilder()
{
    finished();
}


void JsonBuilder::reserveSpace(ssize_t space)
{
    ssize_t sizeRemaining = m_bufferEnd - m_buffer;
    if (sizeRemaining < space)
    {
        if (m_buffer != nullptr)
        {
            ssize_t size = m_buffer - m_bufferStart;
            assert(size >= 0);
            m_zeroCopybuffer.downsizeLastBuffer(size);
        }

        const ssize_t sizeRemainingZeroCopyBuffer = m_zeroCopybuffer.getRemainingSize();
        ssize_t sizeNew = m_maxBlockSize;
        if (space <= sizeRemainingZeroCopyBuffer)
        {
            sizeNew = sizeRemainingZeroCopyBuffer;
        }
        sizeNew = std::max(sizeNew, space);
        char* bufferStartNew = m_zeroCopybuffer.addBuffer(sizeNew);
        m_bufferStart = bufferStartNew;
        m_bufferEnd = m_bufferStart + sizeNew;
        m_buffer = m_bufferStart;
    }
}

void JsonBuilder::resizeBuffer()
{
    if (m_buffer != nullptr)
    {
        ssize_t size = m_buffer - m_bufferStart;
        assert(size >= 0);
        m_zeroCopybuffer.downsizeLastBuffer(size);
        m_bufferStart = nullptr;
        m_bufferEnd = nullptr;
        m_buffer = nullptr;
    }
}



// IJsonParserVisitor
void JsonBuilder::syntaxError(const char* /*str*/, const char* /*message*/)
{
    assert(false);
}

void JsonBuilder::enterNull()
{
    reserveSpace(5);
    assert(m_buffer);
    memcpy(m_buffer, "null,", 5);
    m_buffer += 5;
}

void JsonBuilder::enterBool(bool value)
{
    reserveSpace(6);
    assert(m_buffer);
    if (value)
    {
        memcpy(m_buffer, "true,", 5);
        m_buffer += 5;
    }
    else
    {
        memcpy(m_buffer, "false,", 6);
        m_buffer += 6;
    }
}

void JsonBuilder::enterInt32(std::int32_t value)
{
    reserveSpace(20);   // 11 + 1 is enough: 10 digits + 1 minus + 1 comma
    assert(m_buffer);
    m_buffer = rapidjson::i32toa(value, m_buffer);
    *m_buffer = ',';
    ++m_buffer;
}

void JsonBuilder::enterUInt32(std::uint32_t value)
{
    reserveSpace(20);   // 10 + 1 is enough: 10 digits + 1 comma
    assert(m_buffer);
    m_buffer = rapidjson::u32toa(value, m_buffer);
    *m_buffer = ',';
    ++m_buffer;
}

void JsonBuilder::enterInt64(std::int64_t value)
{
    reserveSpace(30);   // 21 + 1 is enough: 20 digits + 1 minus + 1 comma
    assert(m_buffer);
    m_buffer = rapidjson::i64toa(value, m_buffer);
    *m_buffer = ',';
    ++m_buffer;
}

void JsonBuilder::enterUInt64(std::uint64_t value)
{
    reserveSpace(30);   // 20 + 1 is enough: 20 digits + 1 comma
    assert(m_buffer);
    m_buffer = rapidjson::u64toa(value, m_buffer);
    *m_buffer = ',';
    ++m_buffer;
}

void JsonBuilder::enterDouble(double value)
{
    reserveSpace(50);
    assert(m_buffer);
    m_buffer = rapidjson::dtoa(value, m_buffer);
    *m_buffer = ',';
    ++m_buffer;
}

void JsonBuilder::enterString(const char* value, ssize_t size)
{
    reserveSpace(size*6 + 2 + 1); // string*6 + 2" + comma
    *m_buffer = '\"';
    ++m_buffer;
    escapeString(value, size);
    *m_buffer = '\"';
    ++m_buffer;
    *m_buffer = ',';
    ++m_buffer;
}

void JsonBuilder::enterString(std::string&& value)
{
    reserveSpace(value.size()*6 + 2 + 1); // string*6 + 2" + comma
    *m_buffer = '\"';
    ++m_buffer;
    escapeString(value.c_str(), value.size());
    *m_buffer = '\"';
    ++m_buffer;
    *m_buffer = ',';
    ++m_buffer;
}

void JsonBuilder::enterArray()
{
    reserveSpace(1);
    *m_buffer = '[';
    ++m_buffer;
}

void JsonBuilder::exitArray()
{
    correctComma();
    reserveSpace(2);
    *m_buffer = ']';
    ++m_buffer;
    *m_buffer = ',';
    ++m_buffer;
}

void JsonBuilder::enterObject()
{
    reserveSpace(1);
    *m_buffer = '{';
    ++m_buffer;
}

void JsonBuilder::exitObject()
{
    correctComma();
    reserveSpace(2);
    *m_buffer = '}';
    ++m_buffer;
    *m_buffer = ',';
    ++m_buffer;
}




void JsonBuilder::enterKey(const char* key, ssize_t size)
{
    reserveSpace(size*6 + 2 + 1); // string*6 + 2" + :
    *m_buffer = '\"';
    ++m_buffer;
    escapeString(key, size);
    *m_buffer = '\"';
    ++m_buffer;
    *m_buffer = ':';
    ++m_buffer;
}

void JsonBuilder::enterKey(std::string&& key)
{
    reserveSpace(key.size()*6 + 2 + 1); // string*6 + 2" + :
    *m_buffer = '\"';
    ++m_buffer;
    escapeString(key.c_str(), key.size());
    *m_buffer = '\"';
    ++m_buffer;
    *m_buffer = ':';
    ++m_buffer;
}




void JsonBuilder::finished()
{
    correctComma();
    resizeBuffer();
}


void JsonBuilder::correctComma()
{
    if (m_buffer)
    {
        ssize_t size = m_buffer - m_bufferStart;
        assert(size >= 0);
        if (size >= 1)
        {
            if (*(m_buffer - 1) == ',')
            {
                --m_buffer;
            }
        }
    }
}



static unsigned char getRange(unsigned char c)
{
    // Referring to DFA of http://bjoern.hoehrmann.de/utf-8/decoder/dfa/
    // With new mapping 1 -> 0x10, 7 -> 0x20, 9 -> 0x40, such that AND operation can test multiple types.
    static const unsigned char type[] =
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
        0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
        0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
        0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
        8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,
    };
    return type[c];
}




static bool decodeUtf8(const char*& str, unsigned int& codepoint)
{
#define RAPIDJSON_COPY() c = *str; str++; codepoint = (codepoint << 6) | (c & 0x3Fu)
#define RAPIDJSON_TRANS(mask) result &= ((getRange(c) & mask) != 0)
#define RAPIDJSON_TAIL() RAPIDJSON_COPY(); RAPIDJSON_TRANS(0x70)

    unsigned char c = static_cast<unsigned char>(*str);
    str++;
    if (!(c & 0x80))
    {
        codepoint = c;
        return true;
    }

    unsigned char type = getRange(c);
    if (type >= 32)
    {
        codepoint = 0;
    }
    else
    {
        codepoint = (0xFFu >> type) & c;
    }

    bool result = true;
    switch (type)
    {
    case 2: RAPIDJSON_TAIL(); return result;
    case 3: RAPIDJSON_TAIL(); RAPIDJSON_TAIL(); return result;
    case 4: RAPIDJSON_COPY(); RAPIDJSON_TRANS(0x50); RAPIDJSON_TAIL(); return result;
    case 5: RAPIDJSON_COPY(); RAPIDJSON_TRANS(0x10); RAPIDJSON_TAIL(); RAPIDJSON_TAIL(); return result;
    case 6: RAPIDJSON_TAIL(); RAPIDJSON_TAIL(); RAPIDJSON_TAIL(); return result;
    case 10: RAPIDJSON_COPY(); RAPIDJSON_TRANS(0x20); RAPIDJSON_TAIL(); return result;
    case 11: RAPIDJSON_COPY(); RAPIDJSON_TRANS(0x60); RAPIDJSON_TAIL(); RAPIDJSON_TAIL(); return result;
    default: return false;
    }
#undef RAPIDJSON_COPY
#undef RAPIDJSON_TRANS
#undef RAPIDJSON_TAIL
}






void JsonBuilder::escapeString(const char* str, ssize_t size)
{
    static const char hexDigits[16]{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

    static const char escape[0x20] =
    {
            //0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
            'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'b', 't', 'n', 'u', 'f', 'r', 'u', 'u', // 00
            'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', // 10
    };

    const char* end = str + size;


    while (str < end)
    {
        unsigned char c = static_cast<unsigned char>(*str);
        if (c < 0x20)
        {
            *m_buffer = '\\';
            m_buffer++;
            char e = escape[c];
            *m_buffer = e;
            m_buffer++;
            if (e == 'u')
            {
                *m_buffer = '0';
                m_buffer++;
                *m_buffer = '0';
                m_buffer++;
                *m_buffer = hexDigits[(c >> 4) & 0x0f];
                m_buffer++;
                *m_buffer = hexDigits[(c & 0x0f)];
                m_buffer++;
            }
            str++;
        }
        else if (c < 0x80)
        {
            if (c == '\\' || c == '\"')
            {
                *m_buffer = '\\';
                m_buffer++;
            }
            *m_buffer = c;
            m_buffer++;
            str++;
        }
        else
        {
            unsigned int codepoint;
            bool ok = decodeUtf8(str, codepoint);
            if (!ok)
            {
                streamError << "wrong utf8 format";
            }
            *m_buffer = '\\';
            m_buffer++;
            *m_buffer = 'u';
            m_buffer++;
            if (codepoint <= 0xD7FF || (codepoint >= 0xE000 && codepoint <= 0xFFFF)) {
                *m_buffer = hexDigits[((codepoint >> 12) & 0x0f)];
                m_buffer++;
                *m_buffer = hexDigits[((codepoint >>  8) & 0x0f)];
                m_buffer++;
                *m_buffer = hexDigits[((codepoint >>  4) & 0x0f)];
                m_buffer++;
                *m_buffer = hexDigits[((codepoint)       & 0x0f)];
                m_buffer++;
            }
            else
            {
                assert(codepoint >= 0x010000 && codepoint <= 0x10FFFF);
                // Surrogate pair
                unsigned s = codepoint - 0x010000;
                unsigned lead = (s >> 10) + 0xD800;
                unsigned trail = (s & 0x3FF) + 0xDC00;
                *m_buffer = hexDigits[((lead >> 12) & 0x0f)];
                m_buffer++;
                *m_buffer = hexDigits[((lead >>  8) & 0x0f)];
                m_buffer++;
                *m_buffer = hexDigits[((lead >>  4) & 0x0f)];
                m_buffer++;
                *m_buffer = hexDigits[((lead)       & 0x0f)];
                m_buffer++;
                *m_buffer = '\\';
                m_buffer++;
                *m_buffer = 'u';
                m_buffer++;
                *m_buffer = hexDigits[((trail >> 12) & 0x0f)];
                m_buffer++;
                *m_buffer = hexDigits[((trail >>  8) & 0x0f)];
                m_buffer++;
                *m_buffer = hexDigits[((trail >>  4) & 0x0f)];
                m_buffer++;
                *m_buffer = hexDigits[((trail)       & 0x0f)];
                m_buffer++;
            }
            str++;
        }
    }
}

}   // namespace finalmq
