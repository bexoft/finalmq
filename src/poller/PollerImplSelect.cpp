

#include "poller/PollerImplSelect.h"

#include "helpers/OperatingSystem.h"

#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <assert.h>



PollerImplSelect::PollerImplSelect()
{
}



void PollerImplSelect::init()
{
    FD_ZERO(&m_readfdsCached);
    FD_ZERO(&m_writefdsCached);
    int res = OperatingSystem::instance().makeSocketPair(m_controlSocketRead, m_controlSocketWrite);
    if (res == 0)
    {
        assert(m_controlSocketWrite);
        assert(m_controlSocketRead);
        addSocket(m_controlSocketRead);
    }
}


void PollerImplSelect::addSocket(const SocketDescriptorPtr& fd)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    if (!FD_ISSET(fd->getDescriptor(), &m_readfdsCached))
    {
        FD_SET(fd->getDescriptor(), &m_readfdsCached);
        m_socketDescriptors.insert(fd);
        sockedDescriptorHasChanged();
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
    if (FD_ISSET(fd->getDescriptor(), &m_readfdsCached))
    {
        FD_CLR(fd->getDescriptor(), &m_readfdsCached);
        FD_CLR(fd->getDescriptor(), &m_writefdsCached);
        m_socketDescriptors.erase(fd);
        sockedDescriptorHasChanged();
    }
    else
    {
        // socket not added
    }
    locker.unlock();
}



void PollerImplSelect::enableWrite(const SocketDescriptorPtr& fd)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    if (FD_ISSET(fd->getDescriptor(), &m_readfdsCached))
    {
        if (!FD_ISSET(fd->getDescriptor(), &m_writefdsCached))
        {
            FD_SET(fd->getDescriptor(), &m_writefdsCached);
            writeSockedDescriptorHasChanged();
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
    if (FD_ISSET(fd->getDescriptor(), &m_readfdsCached))
    {
        if (FD_ISSET(fd->getDescriptor(), &m_writefdsCached))
        {
            FD_CLR(fd->getDescriptor(), &m_writefdsCached);
            writeSockedDescriptorHasChanged();
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


void PollerImplSelect::sockedDescriptorHasChanged()
{
    writeSockedDescriptorHasChanged();

    if (!m_insideCollect)
    {
        updateSocketDescriptors();
    }
}


void PollerImplSelect::writeSockedDescriptorHasChanged()
{
    m_socketDescriptorsChanged = true;
    if (m_insideWait)
    {
        releaseWaitInternal();
    }
}



void PollerImplSelect::updateSocketDescriptors()
{
    if (m_socketDescriptorsChanged)
    {
        m_socketDescriptorsChanged = false;
        m_sdMax = 0;
        m_socketDescriptorsConstForSelect = m_socketDescriptors;
        for (auto it = m_socketDescriptors.begin(); it != m_socketDescriptors.end(); ++it)
        {
            m_sdMax = std::max((*it)->getDescriptor(), m_sdMax);
        }
    }
}


void PollerImplSelect::releaseWaitInternal()
{
    if (m_controlSocketWrite)
    {
        OperatingSystem::instance().write(m_controlSocketWrite->getDescriptor(), "", 1);
    }
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
                        OperatingSystem::instance().read(sd, buffer.data(), buffer.size());
                        std::unique_lock<std::mutex> locker(m_mutex);
                        m_result.releaseWait = m_releaseWaitExternal;
                        m_releaseWaitExternal = false;
                        locker.unlock();
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
    do
    {
        int res = 0;
        do
        {
            std::unique_lock<std::mutex> locker(m_mutex);
            m_insideWait = true;
            m_insideCollect = true;

            updateSocketDescriptors();

            // copy fds
            copyFds(m_readfds, m_readfdsCached);
            copyFds(m_writefds, m_writefdsCached);

            locker.unlock();

            timeval tim;
            tim.tv_sec = 0;
            tim.tv_usec = timeout * MILLITOMICRO;

            res = OperatingSystem::instance().select(m_sdMax + 1, &m_readfds, &m_writefds, nullptr, &tim);

        } while (res == -1 && OperatingSystem::instance().getLastError() == SOCKETERROR(EINTR));

        m_insideWait = false;

        collectSockets(res);

    } while (!m_result.error && !m_result.timeout && !m_result.releaseWait && (m_result.descriptorInfos.size() == 0));

    m_insideCollect = false;

    if (m_socketDescriptorsChanged)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        updateSocketDescriptors();
        locker.unlock();
    }

    return m_result;
}


void PollerImplSelect::releaseWait()
{
    std::unique_lock<std::mutex> locker(m_mutex);
    m_releaseWaitExternal = true;
    locker.unlock();
    releaseWaitInternal();
}
