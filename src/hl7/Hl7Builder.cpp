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


#include "finalmq/hl7/Hl7Builder.h"
#include "finalmq/conversions/itoa.h"
#include "finalmq/conversions/dtoa.h"
#include "finalmq/logger/LogStream.h"

#include <assert.h>
#include <string.h>
#include <iostream>


namespace finalmq {

static const char SEGMENT_END = 0x0D;   // '\r'

//static const char MESSAGE_START = 0x0B;
//static const char MESSAGE_END1 = 0x1C;
//static const char MESSAGE_END2 = 0x0D;   // '\r'


Hl7Builder::Hl7Builder(IZeroCopyBuffer& buffer, int maxBlockSize, const std::string& delimiters)
    : m_zeroCopybuffer(buffer)
    , m_maxBlockSize(maxBlockSize)
{
    assert(delimiters.size() == 5);
    m_delimitersForField = delimiters.substr(1);
    m_delimiterField[0] = SEGMENT_END;      // segment delimiter '\r', 0x0D
    m_delimiterField[1] = delimiters[0];    // |
    m_delimiterField[2] = delimiters[1];    // ^
    m_delimiterField[3] = delimiters[4];    // &
    m_delimiterRepeat   = delimiters[2];    // ~
    m_escape            = delimiters[3];    // '\\'
}

Hl7Builder::~Hl7Builder()
{
    finished();
}


void Hl7Builder::reserveSpace(ssize_t space)
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

void Hl7Builder::resizeBuffer()
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
void Hl7Builder::syntaxError(const char* /*str*/, const char* /*message*/)
{
}


void Hl7Builder::enterStruct()
{
    assert(!m_array);
    assert(m_level < LAYER_MAX - 1);
    if (m_level < LAYER_MAX - 1)
    {
        ++m_level;
        m_delimiterCurrent = m_delimiterField[m_level];
    }
}

void Hl7Builder::exitStruct()
{
    assert(!m_array);
    correctDelimiters();
    assert(m_level > 0);
    if (m_level > 0)
    {
        --m_level;
        m_delimiterCurrent = m_delimiterField[m_level];

        reserveSpace(1);
        assert(m_buffer);
        *m_buffer = m_delimiterCurrent;
        ++m_buffer;
    }
}
void Hl7Builder::enterArray()
{
    assert(!m_array);
    m_array = true;
    m_delimiterCurrent = m_delimiterRepeat;
}

void Hl7Builder::exitArray()
{
    correctOneDelimiter();
    assert(m_array);
    m_array = false;
    m_delimiterCurrent = m_delimiterField[m_level];

    reserveSpace(1);
    assert(m_buffer);
    *m_buffer = m_delimiterCurrent;
    ++m_buffer;
}


void Hl7Builder::enterNull()
{
    reserveSpace(3);
    assert(m_buffer);
    *m_buffer = '"';
    ++m_buffer;
    *m_buffer = '"';
    ++m_buffer;
    *m_buffer = m_delimiterCurrent;
    ++m_buffer;
}

void Hl7Builder::enterEmpty()
{
    reserveSpace(1);
    assert(m_buffer);
    *m_buffer = m_delimiterCurrent;
    ++m_buffer;
}

void Hl7Builder::enterInt64(std::int64_t value)
{
    reserveSpace(30);   // 21 + 1 is enough: 20 digits + 1 minus + 1 delimiter
    assert(m_buffer);
    m_buffer = rapidjson::i64toa(value, m_buffer);
    *m_buffer = m_delimiterCurrent;
    ++m_buffer;
}

void Hl7Builder::enterUInt64(std::uint64_t value)
{
    reserveSpace(30);   // 20 + 1 is enough: 20 digits + 1 comma
    assert(m_buffer);
    m_buffer = rapidjson::u64toa(value, m_buffer);
    *m_buffer = m_delimiterCurrent;
    ++m_buffer;
}

void Hl7Builder::enterDouble(double value)
{
    reserveSpace(50);
    assert(m_buffer);
    m_buffer = rapidjson::dtoa(value, m_buffer);
    *m_buffer = m_delimiterCurrent;
    ++m_buffer;
}

void Hl7Builder::enterString(const char* value, ssize_t size)
{
    if (m_waitForDeleimiterField != 1 && m_waitForDeleimiterField != 2)
    {
        reserveSpace(size * 6 + 1); // string*6 + delimiter
        escapeString(value, size);
        *m_buffer = m_delimiterCurrent;
        ++m_buffer;
    }

    if (m_waitForDeleimiterField == 0)
    {
        reserveSpace(m_delimitersForField.size() + 1); // string + delimiter
        memcpy(m_buffer, m_delimitersForField.c_str(), m_delimitersForField.size());
        m_buffer += m_delimitersForField.size();
        *m_buffer = m_delimiterCurrent;
        ++m_buffer;
    }

    if (m_waitForDeleimiterField < 3)
    {
        ++m_waitForDeleimiterField;
    }
}

void Hl7Builder::enterString(const std::string& value)
{
    enterString(value.c_str(), value.size());
}

void Hl7Builder::finished()
{
    resizeBuffer();
}

void Hl7Builder::correctDelimiters()
{
    if (m_buffer)
    {
        while (m_buffer > m_bufferStart)
        {
            char c = *(m_buffer - 1);
            if (c == m_delimiterCurrent)
            {
                --m_buffer;
            }
            else
            {
                break;
            }
        }
    }
}

void Hl7Builder::correctOneDelimiter()
{
    if (m_buffer)
    {
        ssize_t size = m_buffer - m_bufferStart;
        assert(size >= 0);
        if (size >= 1)
        {
            char c = *(m_buffer - 1);
            if (c == m_delimiterCurrent)
            {
                --m_buffer;
            }
        }
    }
}



void Hl7Builder::escapeString(const char* str, ssize_t size)
{
    static const char hexDigits[16]{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

    const char* end = str + size;

    while (str < end)
    {
        unsigned char c = static_cast<unsigned char>(*str);
        if (c < 0x20)
        {
            *m_buffer = m_escape;
            m_buffer++;
            *m_buffer = 'X';
            m_buffer++;
            *m_buffer = hexDigits[(c >> 4) & 0x0f];
            m_buffer++;
            *m_buffer = hexDigits[(c & 0x0f)];
            m_buffer++;
            *m_buffer = m_escape;
            m_buffer++;
        }
        else
        {
            if (c == m_delimiterField[1])   // '|'
            {
                *m_buffer = m_escape;
                m_buffer++;
                *m_buffer = 'F';
                m_buffer++;
                *m_buffer = m_escape;
                m_buffer++;
            }
            else if (c == m_delimiterField[2])   // '^'
            {
                *m_buffer = m_escape;
                m_buffer++;
                *m_buffer = 'S';
                m_buffer++;
                *m_buffer = m_escape;
                m_buffer++;
            }
            else if (c == m_delimiterField[3])   // '&'
            {
                *m_buffer = m_escape;
                m_buffer++;
                *m_buffer = 'T';
                m_buffer++;
                *m_buffer = m_escape;
                m_buffer++;
            }
            else if (c == m_escape)   // '\\'
            {
                *m_buffer = m_escape;
                m_buffer++;
                *m_buffer = 'E';
                m_buffer++;
                *m_buffer = m_escape;
                m_buffer++;
            }
            else if (c == m_delimiterRepeat)   // '~'
            {
                *m_buffer = m_escape;
                m_buffer++;
                *m_buffer = 'R';
                m_buffer++;
                *m_buffer = m_escape;
                m_buffer++;
            }
            else
            {
                *m_buffer = c;
                m_buffer++;
            }
        }
        str++;
    }
}

}   // namespace finalmq
