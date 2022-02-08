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

#if !defined(WIN32) && !defined(__MINGW32__)

#include "finalmq/poller/PollerImplEpoll.h"

#include "finalmq/helpers/OperatingSystem.h"
#include "finalmq/helpers/ModulenameFinalmq.h"
#include "finalmq/logger/LogStream.h"


#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <assert.h>

namespace finalmq {


PollerImplEpoll::PollerImplEpoll()
    : m_socketDescriptorsStable(ATOMIC_FLAG_INIT)
{
    m_socketDescriptorsStable.test_and_set();
}

PollerImplEpoll::~PollerImplEpoll()
{
    if (m_fdEpoll != -1)
    {
        OperatingSystem::instance().close(m_fdEpoll);
    }
}



void PollerImplEpoll::init()
{
    m_fdEpoll = OperatingSystem::instance().epoll_create1(EPOLL_CLOEXEC);
    assert(m_fdEpoll != -1);
    int res = OperatingSystem::instance().makeSocketPair(m_controlSocketRead, m_controlSocketWrite);
    if (res == 0)
    {
        assert(m_controlSocketWrite);
        assert(m_controlSocketRead);
        addSocketEnableRead(m_controlSocketRead);
    }
}


void PollerImplEpoll::addSocket(const SocketDescriptorPtr& fd)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    auto result = m_socketDescriptors.insert(std::make_pair(fd, 0));
    if (result.second)
    {
        epoll_event ev;
        ev.events = 0;
        ev.data.fd = fd->getDescriptor();
        int res = OperatingSystem::instance().epoll_ctl(m_fdEpoll, EPOLL_CTL_ADD, fd->getDescriptor(), &ev);
        if (res == -1)
        {
            streamFatal << "epoll_ctl failed with errno: " << errno;
        }
        assert(res != -1);
        sockedDescriptorHasChanged();
    }
    else
    {
        // socket already added
    }
    locker.unlock();
}


void PollerImplEpoll::addSocketEnableRead(const SocketDescriptorPtr& fd)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    auto result = m_socketDescriptors.insert(std::make_pair(fd, EPOLLIN));
    if (result.second)
    {
        epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = fd->getDescriptor();
        int res = OperatingSystem::instance().epoll_ctl(m_fdEpoll, EPOLL_CTL_ADD, fd->getDescriptor(), &ev);
        if (res == -1)
        {
            streamFatal << "epoll_ctl failed with errno: " << errno;
        }
        assert(res != -1);
        sockedDescriptorHasChanged();
    }
    else
    {
        // socket already added
    }
    locker.unlock();
}


void PollerImplEpoll::removeSocket(const SocketDescriptorPtr& fd)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    auto it = m_socketDescriptors.find(fd);
    if (it != m_socketDescriptors.end())
    {
        epoll_event ev; // to be compatible with linux versions before 2.6.9
        ev.events = 0;
        ev.data.fd = fd->getDescriptor();
        int res = OperatingSystem::instance().epoll_ctl(m_fdEpoll, EPOLL_CTL_DEL, fd->getDescriptor(), &ev);
        if (res == -1)
        {
            streamFatal << "epoll_ctl failed with errno: " << errno;
        }
        assert(res != -1);
        m_socketDescriptors.erase(it);
        sockedDescriptorHasChanged();
    }
    else
    {
        // socket not added
    }
    locker.unlock();
}



void PollerImplEpoll::enableRead(const SocketDescriptorPtr& fd)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    auto it = m_socketDescriptors.find(fd);
    if (it != m_socketDescriptors.end())
    {
        it->second |= EPOLLIN;
        epoll_event ev;
        ev.events = it->second;
        ev.data.fd = fd->getDescriptor();
        int res = OperatingSystem::instance().epoll_ctl(m_fdEpoll, EPOLL_CTL_MOD, fd->getDescriptor(), &ev);
        if (res == -1)
        {
            streamFatal << "epoll_ctl failed with errno: " << errno;
        }
        assert(res != -1);
    }
    else
    {
        // error: socket not added
    }
    locker.unlock();
}


void PollerImplEpoll::disableRead(const SocketDescriptorPtr& fd)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    auto it = m_socketDescriptors.find(fd);
    if (it != m_socketDescriptors.end())
    {
        it->second &= ~EPOLLIN;
        epoll_event ev;
        ev.events = it->second;
        ev.data.fd = fd->getDescriptor();
        int res = OperatingSystem::instance().epoll_ctl(m_fdEpoll, EPOLL_CTL_MOD, fd->getDescriptor(), &ev);
        if (res == -1)
        {
            streamFatal << "epoll_ctl failed with errno: " << errno;
        }
        assert(res != -1);
    }
    else
    {
        // error: socket not added
    }
    locker.unlock();
}




void PollerImplEpoll::enableWrite(const SocketDescriptorPtr& fd)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    auto it = m_socketDescriptors.find(fd);
    if (it != m_socketDescriptors.end())
    {
        it->second |= EPOLLOUT;
        epoll_event ev;
        ev.events = it->second;
        ev.data.fd = fd->getDescriptor();
        int res = OperatingSystem::instance().epoll_ctl(m_fdEpoll, EPOLL_CTL_MOD, fd->getDescriptor(), &ev);
        if (res == -1)
        {
            streamFatal << "epoll_ctl failed with errno: " << errno;
        }
        assert(res != -1);
    }
    else
    {
        // error: socket not added
    }
    locker.unlock();
}


void PollerImplEpoll::disableWrite(const SocketDescriptorPtr& fd)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    auto it = m_socketDescriptors.find(fd);
    if (it != m_socketDescriptors.end())
    {
        it->second &= ~EPOLLOUT;
        epoll_event ev;
        ev.events = it->second;
        ev.data.fd = fd->getDescriptor();
        int res = OperatingSystem::instance().epoll_ctl(m_fdEpoll, EPOLL_CTL_MOD, fd->getDescriptor(), &ev);
        if (res == -1)
        {
            streamFatal << "epoll_ctl failed with errno: " << errno;
        }
        assert(res != -1);
    }
    else
    {
        // error: socket not added
    }
    locker.unlock();
}



void PollerImplEpoll::sockedDescriptorHasChanged()
{
    m_socketDescriptorsStable.clear(std::memory_order_release);
}


void PollerImplEpoll::updateSocketDescriptors()
{
    m_socketDescriptorsConstForEpoll.clear();
    m_socketDescriptorsConstForEpoll.reserve(m_socketDescriptors.size());
    for (auto it = m_socketDescriptors.begin(); it != m_socketDescriptors.end(); ++it)
    {
        m_socketDescriptorsConstForEpoll.push_back(it->first);
    }
}



void PollerImplEpoll::collectSockets(int res)
{
    m_result.clear();

    if (res > 0)
    {
        assert(res <= static_cast<int>(m_events.size()));
        for (int i = 0; i < res; i++)
        {
            const epoll_event& pe = m_events[i];
            SOCKET sd = pe.data.fd;
            DescriptorInfo* descriptorInfo = nullptr;

            if (pe.events & (EPOLLERR | EPOLLHUP))
            {
                descriptorInfo = &m_result.descriptorInfos.add();
                descriptorInfo->sd = sd;
                descriptorInfo->disconnected = true;
            }
            if (pe.events & EPOLLOUT)
            {
                if (descriptorInfo == nullptr)
                {
                    descriptorInfo = &m_result.descriptorInfos.add();
                    descriptorInfo->sd = sd;
                }
                assert(descriptorInfo);
                assert(descriptorInfo->sd == sd);
                descriptorInfo->writable = true;
            }
            if (pe.events & EPOLLIN)
            {
                int countRead = 0;
                int resIoCtl = OperatingSystem::instance().ioctlInt(sd, FIONREAD, &countRead);
                if (resIoCtl == -1)
                {
                    countRead = 0;
                }

                if (sd == m_controlSocketRead->getDescriptor())
                {
                    if (countRead > 0)
                    {
                        // read pending bytes from control socket
                        std::vector<char> buffer(countRead);
                        OperatingSystem::instance().recv(sd, buffer.data(), buffer.size(), 0);
                        m_result.releaseWait = m_releaseFlags.exchange(0, std::memory_order_acq_rel);
                    }
                    else
                    {
                        // control socket broken
                    }
                }
                else
                {
                    if (descriptorInfo == nullptr)
                    {
                        descriptorInfo = &m_result.descriptorInfos.add();
                        descriptorInfo->sd = sd;
                    }
                    assert(descriptorInfo);
                    assert(descriptorInfo->sd == sd);
                    descriptorInfo->readable = true;
                    descriptorInfo->bytesToRead = countRead;
                }
            }
        }
    }
    else if (res == 0)
    {
        // timeout
        m_result.timeout = true;
    }
    else
    {
        // error
        m_result.error = true;
    }
}

const PollerResult& PollerImplEpoll::wait(std::int32_t timeout)
{
    // check if init happened
    assert(m_fdEpoll != -1);

    int res = 0;
    int err = 0;

    if (!m_socketDescriptorsStable.test_and_set(std::memory_order_acq_rel))
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        updateSocketDescriptors();
    }

    do
    {
        res = OperatingSystem::instance().epoll_pwait(m_fdEpoll, &m_events[0], m_events.size(), timeout, nullptr);

        if (res == -1)
        {
            err = OperatingSystem::instance().getLastError();
        }
    } while (res == -1 && (err == SOCKETERROR(EINTR) || err == SOCKETERROR(EAGAIN)));

    if (res == -1)
    {
        streamError << "epoll_pwait failed with errno: " << err;
    }

    collectSockets(res);

    if (!m_socketDescriptorsStable.test_and_set(std::memory_order_acq_rel))
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        updateSocketDescriptors();
    }

    return m_result;
}

void PollerImplEpoll::releaseWait(std::uint32_t info)
{
    m_releaseFlags.fetch_or(info, std::memory_order_acq_rel);
    if (m_controlSocketWrite)
    {
        char dummy = 0;
        OperatingSystem::instance().send(m_controlSocketWrite->getDescriptor(), &dummy, 1, 0);
    }
}

}   // namespace finalmq

#endif
