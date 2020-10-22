

#include "helpers/ZeroCopyBuffer.h"

#include <algorithm>
#include <assert.h>

std::string ZeroCopyBuffer::getData() const
{
    std::string data;
    int size = 0;
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
    std::string& str = m_strings.back();
    str.resize(newSize);
}
