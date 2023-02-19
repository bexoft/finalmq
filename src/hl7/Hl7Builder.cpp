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
    , m_root(std::make_unique<Hl7Node>())
{
    assert(delimiters.size() == 5);
    m_delimitersForField = delimiters;
    m_delimitersForField += delimiters[0];  // |^~\&|
    m_delimiterField[0] = SEGMENT_END;      // segment delimiter '\r', 0x0D
    m_delimiterField[1] = delimiters[0];    // |
    m_delimiterField[2] = delimiters[1];    // ^
    m_delimiterField[3] = delimiters[4];    // &
    m_delimiterRepeat   = delimiters[2];    // ~
    m_escape            = delimiters[3];    // '\\'
}

Hl7Builder::~Hl7Builder()
{
    m_root.reset();
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


class Hl7Node
{
public:
    void enterString(const int* levelIndex, int sizeLevelIndex, int index, std::string&& value)
    {
        if (index == -1 && sizeLevelIndex == 0)
        {
            if (!value.empty())
            {
                m_segmentId = std::move(value);
            }
            return;
        }
        int i = index;
        if (sizeLevelIndex > 0)
        {
            i = levelIndex[0];
        }
        if (i + 1 > static_cast<int>(m_nodes.size()))
        {
            m_nodes.reserve(std::max((i + 1) * 2, 30));
            m_nodes.resize(i + 1);
        }

        if (sizeLevelIndex == 0)
        {
            m_nodes[i].m_strings.emplace_back(std::move(value));
        }
        else
        {
            m_nodes[i].enterString(levelIndex + 1, sizeLevelIndex - 1, index, std::move(value));
        }
    }

    std::string                 m_segmentId;
    std::vector<Hl7Node>        m_nodes;
    std::vector<std::string>    m_strings;
};

// IJsonParserVisitor


void Hl7Builder::enterNull(const int* levelIndex, int sizeLevelIndex, int index)
{
    m_root->enterString(levelIndex, sizeLevelIndex, index, std::string("\"\""));
}

void Hl7Builder::enterInt64(const int* levelIndex, int sizeLevelIndex, int index, std::int64_t value)
{
    m_root->enterString(levelIndex, sizeLevelIndex, index, std::to_string(value));
}

void Hl7Builder::enterUInt64(const int* levelIndex, int sizeLevelIndex, int index, std::uint64_t value)
{
    m_root->enterString(levelIndex, sizeLevelIndex, index, std::to_string(value));
}

void Hl7Builder::enterDouble(const int* levelIndex, int sizeLevelIndex, int index, double value)
{
    m_root->enterString(levelIndex, sizeLevelIndex, index, std::to_string(value));
}

void Hl7Builder::enterString(const int* levelIndex, int sizeLevelIndex, int index, const char* value, ssize_t size)
{
    m_root->enterString(levelIndex, sizeLevelIndex, index, std::string(value, size));
}

void Hl7Builder::enterString(const int* levelIndex, int sizeLevelIndex, int index, std::string&& value)
{
    m_root->enterString(levelIndex, sizeLevelIndex, index, std::move(value));
}

void Hl7Builder::finished()
{
    reserveSpace(3 + m_delimitersForField.size());
    assert(m_buffer);
    memcpy(m_buffer, "MSH", 3);
    m_buffer += 3;
    memcpy(m_buffer, m_delimitersForField.c_str(), m_delimitersForField.size());
    m_buffer += m_delimitersForField.size();

    serialize(*m_root, 0, 0);

    resizeBuffer();

    m_root.reset();
}


void Hl7Builder::serialize(const Hl7Node& node, int index, int iStart)
{
    const std::vector<Hl7Node>& nodes = node.m_nodes;

    for (size_t i = iStart; i < nodes.size(); ++i)
    {
        const Hl7Node& subNode = nodes[i];        

        if (i != 0 && index == 0 && !subNode.m_segmentId.empty())
        {
            reserveSpace(subNode.m_segmentId.size() + 1);
            assert(m_buffer);
            memcpy(m_buffer, subNode.m_segmentId.c_str(), subNode.m_segmentId.size());
            m_buffer += subNode.m_segmentId.size();
            if (!subNode.m_nodes.empty())
            {
                *m_buffer = m_delimiterField[index + 1];
                ++m_buffer;
            }
        }

        if (!subNode.m_nodes.empty())
        {
            serialize(subNode, index + 1, (index == 0 && i == 0) ? 2 : 0);    // skip MSH|^~\&|
        }
        else
        {
            if (!subNode.m_strings.empty())
            {
                for (size_t n = 0; n < subNode.m_strings.size(); ++n)
                {
                    const std::string& str = subNode.m_strings[n];
                    reserveSpace(str.size() * 6 + 1);
                    escapeString(str.c_str(), str.size());
                    if (n < subNode.m_strings.size() - 1)
                    {
                        *m_buffer = m_delimiterRepeat;
                        ++m_buffer;
                    }
                }
            }
        }
        if ((i < nodes.size() - 1) || (index == 0))
        {
            reserveSpace(1);
            *m_buffer = m_delimiterField[index];
            ++m_buffer;
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
