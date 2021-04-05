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


#include "finalmq/helpers/OperatingSystem.h"


#if defined(WIN32) || defined(__MINGW32__)
#include <direct.h>
#include <io.h>
#pragma warning(disable: 4996)
#else
#include <sys/unistd.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <sys/unistd.h>
#endif


namespace finalmq {


    OperatingSystemImpl::OperatingSystemImpl()
    {
    }


    // IOperatingSystem
    int OperatingSystemImpl::close(int fd)
    {
#if defined(WIN32)
#pragma warning(suppress : 4996)
#endif
        return ::close(fd);
    }


    int OperatingSystemImpl::closeSocket(SOCKET fd)
    {
#if defined(WIN32) || defined(__MINGW32__)
        return ::closesocket(fd);
#else
        return ::close(fd);
#endif
    }


    SOCKET OperatingSystemImpl::socket(int af, int type, int protocol)
    {
        return ::socket(af, type, protocol);
    }

    int OperatingSystemImpl::bind(SOCKET fd, const struct sockaddr* name, socklen_t namelen)
    {
        return ::bind(fd, name, namelen);
    }

    SOCKET OperatingSystemImpl::accept(SOCKET fd, struct sockaddr* addr, socklen_t* addrlen)
    {
        return ::accept(fd, addr, addrlen);
    }

    int OperatingSystemImpl::listen(SOCKET fd, int backlog)
    {
        return ::listen(fd, backlog);
    }

    int OperatingSystemImpl::connect(SOCKET fd, const struct sockaddr* name, socklen_t namelen)
    {
        return ::connect(fd, name, namelen);
    }


    int OperatingSystemImpl::setsockopt(SOCKET fd, int level, int optname, const char* optval, int optlen)
    {
        return ::setsockopt(fd, level, optname, optval, optlen);
    }

    int OperatingSystemImpl::getsockname(SOCKET fd, struct sockaddr* name, socklen_t* namelen)
    {
        return ::getsockname(fd, name, namelen);
    }


    int OperatingSystemImpl::write(int fd, const char* buffer, int len)
    {
        return ::write(fd, buffer, len);
    }

    int OperatingSystemImpl::read(int fd, char* buffer, int len)
    {
        return ::read(fd, buffer, len);
    }

    int OperatingSystemImpl::send(SOCKET fd, const char* buffer, int len, int flags)
    {
#if defined(WIN32) || defined(__MINGW32__)
        return ::send(fd, static_cast<const char*>(buffer), len, flags);
#else
        return ::send(fd, buffer, len, flags);
#endif
    }

    int OperatingSystemImpl::recv(SOCKET fd, char* buffer, int len, int flags)
    {
#if defined(WIN32) || defined(__MINGW32__)
        return ::recv(fd, static_cast<char*>(buffer), len, flags);
#else
        return ::recv(fd, buffer, len, flags);
#endif
    }

    int OperatingSystemImpl::getLastError()
    {
        int err = -1;
#if defined(WIN32) || defined(__MINGW32__)
        err = ::GetLastError();
#else
        err = errno;
#endif
        if (err == SOCKETERROR(EINPROGRESS) || err == EAGAIN)
        {
            err = SOCKETERROR(EWOULDBLOCK);
        }
        return err;
    }


    int OperatingSystemImpl::select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout)
    {
        int err = ::select(nfds, readfds, writefds, exceptfds, timeout);
        return err;
    }





#if defined(WIN32) || defined(__MINGW32__)

    int OperatingSystemImpl::makeSocketPair(SocketDescriptorPtr& socket1, SocketDescriptorPtr& socket2)
    {
        socket1 = nullptr;
        socket2 = nullptr;

        SOCKET fdAccept = OperatingSystem::instance().socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (fdAccept == -1) {
            perror("socket");
            return -1;
        }
        SocketDescriptorPtr fileDescriptorAccept(std::make_shared<SocketDescriptor>(fdAccept));

        SOCKET fdClient = OperatingSystem::instance().socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (fdClient == -1) {
            perror("socket");
            return -1;
        }
        SocketDescriptorPtr fileDescriptorClient = std::make_shared<SocketDescriptor>(fdClient);
        setLinger(fdClient, true, 0);
        setNoDelay(fdClient, true);

        int b = 1;
        if (OperatingSystem::instance().setsockopt(fdAccept, SOL_SOCKET, SO_REUSEADDR, (const char*)&b, sizeof(int)) == -1) {
            perror("setsockopt");
            return -1;
        }

        struct sockaddr_in addrBind = { 0, 0, 0, 0 };
        addrBind.sin_family = AF_INET;
        addrBind.sin_port = 0;
        addrBind.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (OperatingSystem::instance().bind(fdAccept, (struct sockaddr*)&addrBind, sizeof(addrBind)) == -1) {
            perror("bind");
            return -1;
        }

        if (OperatingSystem::instance().listen(fdAccept, 50) == -1) {
            perror("listen");
            return -1;
        }

        struct sockaddr addr = { 0, 0, 0, 0 };
        socklen_t lenaddr = sizeof(addr);
        if (OperatingSystem::instance().getsockname(fdAccept, &addr, &lenaddr) == -1) {
            perror("getsockname");
            return -1;
        }
        if (OperatingSystem::instance().connect(fdClient, &addr, sizeof(addr)) == -1) {
            fprintf(stderr, "connect wsaerrr %d\n", OperatingSystem::instance().getLastError());
            return -1;
        }

        SOCKET fdServer = OperatingSystem::instance().accept(fdAccept, &addr, &lenaddr);
        if (fdServer == INVALID_SOCKET) {
            perror("accept");
            return -1;
        }
        socket1 = std::make_shared<SocketDescriptor>(fdServer);
        socket2 = fileDescriptorClient;

        setNonBlocking(fdClient, true);
        setNonBlocking(fdServer, true);
        setLinger(fdServer, true, 0);
        setNoDelay(fdServer, true);

        fileDescriptorAccept = nullptr;

        return 0;
    }

#else

    int OperatingSystemImpl::epoll_create1(int flags)
    {
        int err = ::epoll_create1(flags);
        return err;
    }

    int OperatingSystemImpl::epoll_ctl(int epfd, int op, int fd, struct epoll_event* event)
    {
        int err = ::epoll_ctl(epfd, op, fd, event);
        return err;
    }

    int OperatingSystemImpl::epoll_pwait(int epfd, struct epoll_event* events, int maxevents, int timeout, const sigset_t* sigmask)
    {
        int err = ::epoll_pwait(epfd, events, maxevents, timeout, sigmask);
        return err;
    }


    int OperatingSystemImpl::makeSocketPair(SocketDescriptorPtr& socket1, SocketDescriptorPtr& socket2)
    {
        int sds[2];
        int res = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, sds);
        if (res == 0)
        {
            setNonBlocking(sds[0], true);
            setLinger(sds[0], true, 0);
            setNoDelay(sds[0], true);
            setNonBlocking(sds[1], true);
            setLinger(sds[1], true, 0);
            setNoDelay(sds[1], true);

            socket1 = std::make_shared<SocketDescriptor>(sds[0]);
            socket2 = std::make_shared<SocketDescriptor>(sds[1]);
        }
        return res;
    }


#endif



    int OperatingSystemImpl::ioctlInt(SOCKET fd, unsigned long int request, int* value)
    {
#if defined(WIN32) || defined(__MINGW32__)
        u_long val = 0;
        int err = ioctlsocket(fd, request, &val);
        *value = val;
#else
        int err = ::ioctl(fd, request, value);
#endif		
        return err;
    }




    int OperatingSystemImpl::setNoDelay(SOCKET fd, bool noDelay)
    {
        int val = noDelay ? 1 : 0;
        int res = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&val, sizeof(val));
        return res;
    }


    int OperatingSystemImpl::setNonBlocking(SOCKET fd, bool nonBlock)
    {
#if defined(WIN32) || defined(__MINGW32__)
        unsigned long val = (nonBlock) ? 1 : 0;	// set non-blocking
        int res = ioctlsocket(fd, FIONBIO, &val);
#else
        int res = fcntl(fd, F_GETFL, 0);
        if (res != -1)
        {
            int val = (nonBlock) ? (res | O_NONBLOCK) : (res & ~O_NONBLOCK);
            res = fcntl(fd, F_SETFL, val);
        }
#endif
        return res;
    }


    int OperatingSystemImpl::setLinger(SOCKET fd, bool on, int timeToLinger)
    {
        struct linger l;
        l.l_onoff = on;
        l.l_linger = timeToLinger;
        int res = setsockopt(fd, SOL_SOCKET, SO_LINGER, (const char*)&l, sizeof(l));
        return res;
    }




    //////////////////////////////////////
    /// OperatingSystem

    std::unique_ptr<IOperatingSystem> OperatingSystem::m_instance;

    void OperatingSystem::setInstance(std::unique_ptr<IOperatingSystem>& instance)
    {
        m_instance = std::move(instance);
    }

}   // namespace finalmq
