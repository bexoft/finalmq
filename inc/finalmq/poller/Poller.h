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


#include "finalmq/helpers/SocketDescriptor.h"
#include <vector>
#include <assert.h>


namespace finalmq {

struct DescriptorInfo
{
    void clear()
    {
        sd = INVALID_SOCKET;
        readable = false;
        writable = false;
        disconnected = false;
        bytesToRead = 0;
    }
    SOCKET sd = INVALID_SOCKET;
    bool readable = false;
    bool writable = false;
    bool disconnected = false;
    std::int32_t bytesToRead = 0;
};


class DescriptorInfos
{
public:
    DescriptorInfo& add()
    {
        if (m_size >= static_cast<int>(m_descriptorInfos.size()))
        {
            assert(static_cast<int>(m_descriptorInfos.size()) == m_size);
            m_descriptorInfos.resize(m_size + 1);
        }
        m_size++;
        DescriptorInfo& info = m_descriptorInfos[m_size - 1];
        info.clear();
        return info;
    }

    void clear()
    {
        m_size = 0;
    }

    size_t size() const
    {
        return m_size;
    }

    const DescriptorInfo& operator [](ssize_t ix) const
    {
        assert(ix < m_size);
        assert(ix < static_cast<int>(m_descriptorInfos.size()));
        return m_descriptorInfos[ix];
    }

private:
    ssize_t                     m_size = 0;
    std::vector<DescriptorInfo> m_descriptorInfos;
};



struct PollerResult
{
    bool error = false;
    bool timeout = false;
    std::uint32_t releaseWait = 0;
    DescriptorInfos descriptorInfos;

    void clear()
    {
        error = false;
        timeout = false;
        releaseWait = 0;
        descriptorInfos.clear();
    }
    PollerResult() = default;

private:
    PollerResult(PollerResult&) = delete;
};



struct IPoller
{
    virtual ~IPoller() {}
    virtual void init() = 0;
    virtual void addSocket(const SocketDescriptorPtr& fd) = 0;
    virtual void addSocketEnableRead(const SocketDescriptorPtr& fd) = 0;
    virtual void removeSocket(const SocketDescriptorPtr& fd) = 0;
    virtual void enableRead(const SocketDescriptorPtr& fd) = 0;
    virtual void disableRead(const SocketDescriptorPtr& fd) = 0;
    virtual void enableWrite(const SocketDescriptorPtr& fd) = 0;
    virtual void disableWrite(const SocketDescriptorPtr& fd) = 0;
    virtual const PollerResult& wait(std::int32_t timeout) = 0;
    virtual void releaseWait(std::uint32_t info) = 0;
};

typedef std::shared_ptr<IPoller> IPollerPtr;

}   // namespace finalmq
