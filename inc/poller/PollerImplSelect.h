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

#include "Poller.h"
#include <unordered_set>
#include <unordered_map>
#include <atomic>
#include <mutex>


namespace finalmq {

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
    virtual void addSocketEnableRead(const SocketDescriptorPtr& fd) override;
    virtual void removeSocket(const SocketDescriptorPtr& fd) override;
    virtual void enableRead(const SocketDescriptorPtr& fd) override;
    virtual void disableRead(const SocketDescriptorPtr& fd) override;
    virtual void enableWrite(const SocketDescriptorPtr& fd) override;
    virtual void disableWrite(const SocketDescriptorPtr& fd) override;
    virtual const PollerResult& wait(std::int32_t timeout) override;
    virtual void releaseWait() override;

private:
    void releaseWaitInternal();
    inline static void copyFds(fd_set& dest, fd_set& source);
    void updateSocketDescriptors();
    void updateSdMax();
    void sockedDescriptorHasChanged();
    void sdMaxHasChanged();
    void sockedDescriptorAndSdMaxHasChanged();
    void collectSockets(int res);


    SocketDescriptorPtr m_controlSocketRead;
    SocketDescriptorPtr m_controlSocketWrite;

    std::unordered_map<SocketDescriptorPtr, int> m_socketDescriptors;

    PollerResult m_result;
    fd_set m_readfdsCached;
    fd_set m_writefdsCached;
    fd_set m_readfdsOriginal;
    fd_set m_writefdsOriginal;
    fd_set m_readfds;
    fd_set m_writefds;
    fd_set m_errorfds;
    std::atomic_flag m_socketDescriptorsStable;
    std::atomic_flag m_sdMaxStable;
    bool m_releaseWaitExternal = false;

    // parameters that are const during select and collect.
    int m_sdMax = 0;
    std::vector<SocketDescriptorPtr> m_socketDescriptorsConstForSelect;

    std::mutex m_mutex;
};

}   // namespace finalmq
