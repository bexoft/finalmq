#pragma once

#include "Poller.h"
#include <unordered_set>
#include <mutex>




class PollerImplSelect : public IPoller
{
public:
    PollerImplSelect();

private:
    PollerImplSelect(const PollerImplSelect&) = delete;
    const PollerImplSelect& operator =(const PollerImplSelect&) = delete;
    PollerImplSelect(const PollerImplSelect&&) = delete;
    const PollerImplSelect& operator =(PollerImplSelect&&) = delete;

    virtual void init() override;
    virtual void addSocket(const SocketDescriptorPtr& fd) override;
    virtual void removeSocket(const SocketDescriptorPtr& fd) override;
    virtual void enableWrite(const SocketDescriptorPtr& fd) override;
    virtual void disableWrite(const SocketDescriptorPtr& fd) override;
    virtual const PollerResult& wait(std::int32_t timeout) override;
    virtual void releaseWait() override;

private:
    void releaseWaitInternal();
    static void copyFds(fd_set& dest, fd_set& source);
    void updateSocketDescriptors();
    void sockedDescriptorHasChanged();
    void writeSockedDescriptorHasChanged();
    void collectSockets(int res);


    SocketDescriptorPtr m_controlSocketRead;
    SocketDescriptorPtr m_controlSocketWrite;

    std::unordered_set<SocketDescriptorPtr> m_socketDescriptors;

    PollerResult m_result;
    fd_set m_readfdsCached;
    fd_set m_writefdsCached;
    fd_set m_readfds;
    fd_set m_writefds;
    fd_set m_errorfds;
    bool m_insideWait = false;
    bool m_insideCollect = false;
    bool m_socketDescriptorsChanged = false;
    bool m_releaseWaitExternal = false;

    // parameters that are const during select and collect.
    SOCKET m_sdMax = 0;
    std::unordered_set<SocketDescriptorPtr> m_socketDescriptorsConstForSelect;

    std::mutex m_mutex;
};


