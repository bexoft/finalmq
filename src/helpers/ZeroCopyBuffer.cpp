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



#include "helpers/BexDefines.h"
#include "helpers/ZeroCopyBuffer.h"

#include <algorithm>
#include <assert.h>


namespace finalmq {


std::string ZeroCopyBuffer::getData() const
{
    std::string data;
    ssize_t size = 0;
    std::for_each(m_strings.begin(), m_strings.end(), [&size] (const std::string& entry) {
        size += entry.size();
    });
    data.reserve(size);
    std::for_each(m_strings.begin(), m_strings.end(), [&data] (const std::string& entry) {
        data +=entry;
    });

    return data;
}




char* ZeroCopyBuffer::addBuffer(int size)
{
    assert(size > 0);
    std::string str;
    str.resize(size);
    m_strings.push_back(std::move(str));
    return const_cast<char*>(m_strings.back().data());
}

void ZeroCopyBuffer::downsizeLastBuffer(int newSize)
{
    if (m_strings.empty() && newSize == 0)
    {
        return;
    }

    assert(!m_strings.empty());

    if (newSize == 0)
    {
        m_strings.pop_back();
    }
    else
    {
        std::string& str = m_strings.back();
        str.resize(newSize);
    }
}

}   // namespace finalmq
