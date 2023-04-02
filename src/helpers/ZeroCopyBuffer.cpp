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



#include "finalmq/helpers/FmqDefines.h"
#include "finalmq/helpers/ZeroCopyBuffer.h"

#include <algorithm>
#include <assert.h>


namespace finalmq {


std::string ZeroCopyBuffer::getData() const
{
    std::string data;
    ssize_t size = 0;
    for (const auto& chunk : m_chunks)
    {
        size += chunk.size();
    }
    data.reserve(size);
    for (const auto& chunk : m_chunks)
    {
        data += chunk;
    }

    return data;
}


const std::list<std::string>& ZeroCopyBuffer::chunks() const
{
    return m_chunks;
}


char* ZeroCopyBuffer::addBuffer(ssize_t size, ssize_t /*reserve*/)
{
    assert(size > 0);
    std::string str;
    str.resize(size);
    m_chunks.push_back(std::move(str));
    return const_cast<char*>(m_chunks.back().data());
}

void ZeroCopyBuffer::downsizeLastBuffer(ssize_t newSize)
{
    if (m_chunks.empty() && newSize == 0)
    {
        return;
    }

    assert(!m_chunks.empty());

    if (newSize == 0)
    {
        m_chunks.pop_back();
    }
    else
    {
        std::string& str = m_chunks.back();
        str.resize(newSize);
    }
}

ssize_t ZeroCopyBuffer::getRemainingSize() const
{
    return 0;
}

}   // namespace finalmq
