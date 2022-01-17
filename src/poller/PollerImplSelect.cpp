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



#include "finalmq/poller/PollerImplSelect.h"

#include "finalmq/helpers/OperatingSystem.h"
#include "finalmq/helpers/ModulenameFinalmq.h"
#include "finalmq/logger/LogStream.h"

#if !defined(WIN32) && !defined(__MINGW32__)
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#endif

#include <assert.h>


namespace finalmq {

static const int SOCKET_POLLIN  = 1;
static const int SOCKET_POLLOUT = 2;


PollerImplSelect::PollerImplSelect()
    : m_socketDescriptorsStable{}
    , m_sdMaxStable{}
{
    m_socketDescriptorsStable.test_and_set();
    m_sdMaxStable.test_and_set();
}



void PollerImplSelect::init()
{
    FD_ZERO(&m_readfdsCached);
    FD_ZERO(&m_writefdsCached);
    FD_ZERO(&m_errorfdsCached);
    FD_ZERO(&m_readfdsOriginal);
    FD_ZERO(&m_writefdsOriginal);
    FD_ZERO(&m_errorfdsOriginal);
    int res = OperatingSystem::instance().makeSocketPair(m_controlSocketRead, m_controlSocketWrite);
    if (res == 0)
    {
        assert(m_controlSocketWrite);
        assert(m_controlSocketRead);
        addSocketEnableRead(m_controlSocketRead);
    }
}


void PollerImplSelect::addSocket(const SocketDescriptorPtr& fd)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    auto result = m_socketDescriptors.insert(std::make_pair(fd, 0));
    if (result.second)
    {
        if (!FD_ISSET(fd->getDescriptor(), &m_errorfdsCached))
        {
            FD_SET(fd->getDescriptor(), &m_errorfdsCached);
            sockedDescriptorAndSdMaxHasChanged();
        }
    }
    else
    {
        // socket already added
    }
    locker.unlock();
}


void PollerImplSelect::addSocketEnableRead(const SocketDescriptorPtr& fd)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    auto result = m_socketDescriptors.insert(std::make_pair(fd, SOCKET_POLLIN));
    if (result.second)
    {
        if (!FD_ISSET(fd->getDescriptor(), &m_readfdsCached) ||
            !FD_ISSET(fd->getDescriptor(), &m_errorfdsCached))
        {
            FD_SET(fd->getDescriptor(), &m_readfdsCached);
            FD_SET(fd->getDescriptor(), &m_errorfdsCached);
            sockedDescriptorAndSdMaxHasChanged();
        }
    }
    else
    {
        // socket already added
    }
    locker.unlock();
}


void PollerImplSelect::removeSocket(const SocketDescriptorPtr& fd)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    auto it = m_socketDescriptors.find(fd);
    if (it != m_socketDescriptors.end())
    {
        FD_CLR(fd->getDescriptor(), &m_readfdsCached);
        FD_CLR(fd->getDescriptor(), &m_writefdsCached);
        FD_CLR(fd->getDescriptor(), &m_errorfdsCached);
        m_socketDescriptors.erase(it);
        sockedDescriptorAndSdMaxHasChanged();
    }
    else
    {
        // socket not added
    }
    locker.unlock();
}


void PollerImplSelect::enableRead(const SocketDescriptorPtr& fd)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    auto it = m_socketDescriptors.find(fd);
    if (it != m_socketDescriptors.end())
    {
        it->second |= SOCKET_POLLIN;
        if (!FD_ISSET(fd->getDescriptor(), &m_readfdsCached))
        {
            FD_SET(fd->getDescriptor(), &m_readfdsCached);
            sdMaxHasChanged();
        }
    }
    else
    {
        // error: socket not added
    }
    locker.unlock();
}


void PollerImplSelect::disableRead(const SocketDescriptorPtr& fd)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    auto it = m_socketDescriptors.find(fd);
    if (it != m_socketDescriptors.end())
    {
        it->second &= ~SOCKET_POLLIN;
        if (FD_ISSET(fd->getDescriptor(), &m_readfdsCached))
        {
            FD_CLR(fd->getDescriptor(), &m_readfdsCached);
            sdMaxHasChanged();
        }
    }
    else
    {
        // error: socket not added
    }
    locker.unlock();
}




void PollerImplSelect::enableWrite(const SocketDescriptorPtr& fd)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    auto it = m_socketDescriptors.find(fd);
    if (it != m_socketDescriptors.end())
    {
        it->second |= SOCKET_POLLOUT;
        if (!FD_ISSET(fd->getDescriptor(), &m_writefdsCached))
        {
            FD_SET(fd->getDescriptor(), &m_writefdsCached);
            sdMaxHasChanged();
        }
    }
    else
    {
        // error: socket not added
    }
    locker.unlock();
}


void PollerImplSelect::disableWrite(const SocketDescriptorPtr& fd)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    auto it = m_socketDescriptors.find(fd);
    if (it != m_socketDescriptors.end())
    {
        it->second &= ~SOCKET_POLLOUT;
        if (FD_ISSET(fd->getDescriptor(), &m_writefdsCached))
        {
            FD_CLR(fd->getDescriptor(), &m_writefdsCached);
            sdMaxHasChanged();
        }
    }
    else
    {
        // error: socket not added
    }
    locker.unlock();
}


void PollerImplSelect::copyFds(fd_set& dest, fd_set& source)
{
    memcpy(&dest, &source, sizeof(fd_set));
}


void PollerImplSelect::sdMaxHasChanged()
{
    m_sdMaxStable.clear(std::memory_order_release);
    releaseWait(0);
}


void PollerImplSelect::sockedDescriptorAndSdMaxHasChanged()
{
    m_socketDescriptorsStable.clear(std::memory_order_release);
    m_sdMaxStable.clear(std::memory_order_release);
    releaseWait(0);
}



void PollerImplSelect::updateSocketDescriptors()
{
    m_socketDescriptorsConstForSelect.clear();
    m_socketDescriptorsConstForSelect.reserve(m_socketDescriptors.size());
    for (auto it = m_socketDescriptors.begin(); it != m_socketDescriptors.end(); ++it)
    {
        m_socketDescriptorsConstForSelect.push_back(it->first);
    }
}

void PollerImplSelect::updateSdMax()
{
    m_sdMax = 0;
#if !defined(WIN32) && !defined(__MINGW32__)
    for (auto it = m_socketDescriptors.begin(); it != m_socketDescriptors.end(); ++it)
    {
        // if an event is active (readable or writeable)
        if (it->second)
        {
            m_sdMax = std::max(it->first->getDescriptor(), m_sdMax);
        }
    }
#endif
    copyFds(m_readfdsOriginal,  m_readfdsCached);
    copyFds(m_writefdsOriginal, m_writefdsCached);
    copyFds(m_errorfdsOriginal, m_errorfdsCached);
}


static int MILLITOMICRO = 1000;


void PollerImplSelect::collectSockets(int res)
{
    m_result.clear();

    int cntFd = 0;
    if (res > 0)
    {
        for (auto it = m_socketDescriptorsConstForSelect.begin(); it != m_socketDescriptorsConstForSelect.end() && cntFd < res; ++it)
        {
            const SocketDescriptorPtr& psd = *it;
            assert(psd);
            SOCKET sd = psd->getDescriptor();
            DescriptorInfo* descriptorInfo = nullptr;
            if (FD_ISSET(sd, &m_readfds))
            {
                cntFd++;

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
                        OperatingSystem::instance().recv(sd, buffer.data(), static_cast<int>(buffer.size()), 0);
                        m_result.releaseWait = m_releaseFlags.exchange(0, std::memory_order_acq_rel);
                    }
                    else
                    {
                        // control socket broken
                    }
                }
                else
                {
                    descriptorInfo = &m_result.descriptorInfos.add();
                    assert(descriptorInfo);
                    descriptorInfo->sd = sd;
                    descriptorInfo->readable = true;
                    descriptorInfo->bytesToRead = countRead;
                }
            }
            if (FD_ISSET(sd, &m_writefds))
            {
                cntFd++;
                if (descriptorInfo == nullptr)
                {
                    descriptorInfo = &m_result.descriptorInfos.add();
                    descriptorInfo->sd = sd;
                }
                assert(descriptorInfo);
                assert(descriptorInfo->sd == sd);
                if (!descriptorInfo->disconnected)
                {
                    descriptorInfo->writable = true;
                }
            }
            if (FD_ISSET(sd, &m_errorfds))
            {
                cntFd++;
                if (descriptorInfo == nullptr)
                {
                    descriptorInfo = &m_result.descriptorInfos.add();
                    descriptorInfo->sd = sd;
                    descriptorInfo->disconnected = true;
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

const PollerResult& PollerImplSelect::wait(std::int32_t timeout)
{
    // check if init happened
    assert(m_controlSocketRead);
    do
    {
        int res = 0;
        int err = 0;
        do
        {
            if (!m_socketDescriptorsStable.test_and_set(std::memory_order_acq_rel))
            {
                std::unique_lock<std::mutex> locker(m_mutex);
                updateSocketDescriptors();
            }
            if (!m_sdMaxStable.test_and_set(std::memory_order_acq_rel))
            {
                std::unique_lock<std::mutex> locker(m_mutex);
                updateSdMax();
            }

            // copy fds
            copyFds(m_readfds,  m_readfdsOriginal);
            copyFds(m_writefds, m_writefdsOriginal);
            copyFds(m_errorfds, m_errorfdsOriginal);

            timeval tim;
            tim.tv_sec = 0;
            tim.tv_usec = timeout * MILLITOMICRO;

            res = OperatingSystem::instance().select(m_sdMax + 1, &m_readfds, &m_writefds, &m_errorfds, &tim);

            if (res == -1)
            {
                err = OperatingSystem::instance().getLastError();
            }

        } while (res == -1 && (err == SOCKETERROR(EINTR) || err == EAGAIN));

        if (res == -1)
        {
            streamError << "select failed with errno: " << err;
        }

        collectSockets(res);

    } while (!m_result.error && !m_result.timeout && m_result.releaseWait == 0 && (m_result.descriptorInfos.size() == 0));

    if (!m_socketDescriptorsStable.test_and_set(std::memory_order_acq_rel))
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        updateSocketDescriptors();
    }

    return m_result;
}


void PollerImplSelect::releaseWait(std::uint32_t info)
{
    m_releaseFlags.fetch_or(info, std::memory_order_acq_rel);
    if (m_controlSocketWrite)
    {
        char dummy = 0;
        OperatingSystem::instance().send(m_controlSocketWrite->getDescriptor(), &dummy, 1, 0);
    }
}

}   // namespace finalmq
