#pragma once

#include "Poller.h"
#include <unordered_set>
#include <array>
#include <mutex>


#include <sys/epoll.h>


class PollerImplEpoll : public IPoller
{
public:
    PollerImplEpoll();
    ~PollerImplEpoll();

private:
    PollerImplEpoll(const PollerImplEpoll&) = delete;
    const PollerImplEpoll& operator =(const PollerImplEpoll&) = delete;
    PollerImplEpoll(const PollerImplEpoll&&) = delete;
    const PollerImplEpoll& operator =(PollerImplEpoll&&) = delete;

    virtual void init() override;
    virtual void addSocket(const SocketDescriptorPtr& fd) override;
    virtual void removeSocket(const SocketDescriptorPtr& fd) override;
    virtual void enableWrite(const SocketDescriptorPtr& fd) override;
    virtual void disableWrite(const SocketDescriptorPtr& fd) override;
    virtual const PollerResult& wait(std::int32_t timeout) override;
    virtual void releaseWait() override;

private:
    void updateSocketDescriptors();
    void sockedDescriptorHasChanged();
    void collectSockets(int res);
    void releaseWaitInternal();

    SocketDescriptorPtr m_controlSocketRead;
    SocketDescriptorPtr m_controlSocketWrite;

    std::unordered_set<SocketDescriptorPtr> m_socketDescriptors;

    PollerResult    m_result;
    int             m_fdEpoll = -1;
    bool            m_insideCollect = false;
    bool            m_socketDescriptorsChanged = false;
    std::array<epoll_event, 32>     m_events;


    std::unordered_set<SocketDescriptorPtr> m_socketDescriptorsConstForEpoll;

    std::mutex m_mutex;
};


