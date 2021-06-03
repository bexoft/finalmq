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

#include "SocketDescriptor.h"


struct stat;

namespace finalmq {


    struct IOperatingSystem
    {
        virtual ~IOperatingSystem() {}
        virtual int open(const char* filename, int flags) = 0;
        virtual int stat(const char* filename, struct stat* buf) = 0;
        virtual int fstat(int fd, struct stat* buf) = 0;
        virtual int close(int fd) = 0;
        virtual int closeSocket(SOCKET fd) = 0;
        virtual SOCKET socket(int af, int type, int protocol) = 0;
        virtual int bind(SOCKET fd, const struct sockaddr* name, socklen_t namelen) = 0;
        virtual SOCKET accept(SOCKET fd, struct sockaddr* addr, socklen_t* addrlen) = 0;
        virtual int listen(SOCKET fd, int backlog) = 0;
        virtual int connect(SOCKET fd, const struct sockaddr* name, socklen_t namelen) = 0;
        virtual int setsockopt(SOCKET fd, int level, int optname, const char* optval, int optlen) = 0;
        virtual int getsockname(SOCKET fd, struct sockaddr* name, socklen_t* namelen) = 0;
        virtual int write(int fd, const char* buffer, int len) = 0;
        virtual int read(int fd, char* buffer, int len) = 0;
        virtual int send(SOCKET fd, const char* buffer, int len, int flags) = 0;
        virtual int recv(SOCKET fd, char* buffer, int len, int flags) = 0;
        virtual int getLastError() = 0;
        virtual int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout) = 0;
#if !defined(WIN32) && !defined(__MINGW32__)
        virtual int epoll_create1(int flags) = 0;
        virtual int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event) = 0;
        virtual int epoll_pwait(int epfd, struct epoll_event* events, int maxevents, int timeout, const sigset_t* sigmask) = 0;
#endif
        virtual int makeSocketPair(SocketDescriptorPtr& socket1, SocketDescriptorPtr& socket2) = 0;
        virtual int ioctlInt(SOCKET fd, unsigned long int request, int* value) = 0;
        virtual int setNoDelay(SOCKET fd, bool noDelay) = 0;
        virtual int setNonBlocking(SOCKET fd, bool nonBlock) = 0;
        virtual int setLinger(SOCKET fd, bool on, int timeToLinger) = 0;
    };


}   // namespace finalmq

