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

#if !defined(WIN32) && !defined(__MINGW32__)

#include "Poller.h"
#include <unordered_map>
#include <array>
#include <mutex>


#include <sys/epoll.h>

namespace finalmq {


class SYMBOLEXP PollerImplEpoll : public IPoller
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
    virtual void addSocketEnableRead(const SocketDescriptorPtr& fd) override;
    virtual void removeSocket(const SocketDescriptorPtr& fd) override;
    virtual void enableRead(const SocketDescriptorPtr& fd) override;
    virtual void disableRead(const SocketDescriptorPtr& fd) override;
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

    std::unordered_map<SocketDescriptorPtr, int> m_socketDescriptors;

    PollerResult        m_result;
    int                 m_fdEpoll = -1;
    std::atomic_flag    m_socketDescriptorsStable;
    std::array<epoll_event, 32>     m_events;


    std::vector<SocketDescriptorPtr> m_socketDescriptorsConstForEpoll;

    std::mutex m_mutex;
};

}   // namespace finalmq


#endif
