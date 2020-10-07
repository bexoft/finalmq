#pragma once


#include "helpers/SocketDescriptor.h"
#include <vector>
#include <assert.h>


struct DescriptorInfo
{
    void clear()
    {
        readable = false;
        writable = false;
        disconnected = false;
        bytesToRead = 0;
    }
    SOCKET sd = INVALID_FD;
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

    const DescriptorInfo& operator [](int ix) const
    {
        assert(ix < m_size);
        assert(ix < static_cast<int>(m_descriptorInfos.size()));
        return m_descriptorInfos[ix];
    }

private:
    int                         m_size = 0;
    std::vector<DescriptorInfo> m_descriptorInfos;
};



struct PollerResult
{
    bool error = false;
    bool timeout = false;
    bool releaseWait = false;
    DescriptorInfos descriptorInfos;

    void clear()
    {
        error = false;
        timeout = false;
        releaseWait = false;
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
    virtual void removeSocket(const SocketDescriptorPtr& fd) = 0;
    virtual void enableWrite(const SocketDescriptorPtr& fd) = 0;
    virtual void disableWrite(const SocketDescriptorPtr& fd) = 0;
    virtual const PollerResult& wait(std::int32_t timeout) = 0;
    virtual void releaseWait() = 0;
};

typedef std::shared_ptr<IPoller> IPollerPtr;
