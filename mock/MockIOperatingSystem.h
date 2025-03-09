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


#include "finalmq/helpers/IOperatingSystem.h"

#include "gmock/gmock.h"

namespace finalmq {

class MockIOperatingSystem : public IOperatingSystem
{
public:
    MOCK_METHOD(int, open, (const char* filename, int flags, int sflags), (override));
    MOCK_METHOD(int, stat, (const char* filename, struct stat* buf), (override));
    MOCK_METHOD(int, fstat, (int fd, struct stat* buf), (override));
    MOCK_METHOD(int, lseek, (int fd, long offset, int origin), (override));
    MOCK_METHOD(int, unlink, (const char* filename), (override));
    MOCK_METHOD(int, close, (int fd), (override));
    MOCK_METHOD(int, closeSocket, (SOCKET fd), (override));
    MOCK_METHOD(SOCKET, socket, (int af, int type, int protocol), (override));
    MOCK_METHOD(int, bind, (SOCKET fd, const struct sockaddr* name, socklen_t namelen), (override));
    MOCK_METHOD(SOCKET, accept, (SOCKET fd, struct sockaddr* addr, socklen_t* addrlen), (override));
    MOCK_METHOD(int, listen, (SOCKET fd, int backlog), (override));
    MOCK_METHOD(int, connect, (SOCKET fd, const struct sockaddr* name, socklen_t namelen), (override));
    MOCK_METHOD(int, setsockopt, (SOCKET fd, int level, int optname, const char* optval, int optlen), (override));
    MOCK_METHOD(int, getsockname, (SOCKET fd, struct sockaddr* name, socklen_t* namelen), (override));
    MOCK_METHOD(int, write, (int fd, const char* buffer, int len), (override));
    MOCK_METHOD(int, read, (int fd, char* buffer, int len), (override));
    MOCK_METHOD(int, send, (SOCKET fd, const char* buffer, int len, int flags), (override));
    MOCK_METHOD(int, recv, (SOCKET fd, char* buffer, int len, int flags), (override));
    MOCK_METHOD(int, getLastError, (), (override));
    MOCK_METHOD(int, select, (int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout), (override));
#if !defined(WIN32) && !defined(__MINGW32__)
    MOCK_METHOD(int, epoll_create1, (int flags), (override));
    MOCK_METHOD(int, epoll_ctl, (int epfd, int op, int fd, struct epoll_event* event), (override));
    MOCK_METHOD(int, epoll_pwait, (int epfd, struct epoll_event *events, int maxevents, int timeout, const sigset_t* mask), (override));
#endif
    MOCK_METHOD(int, makeSocketPair, (SocketDescriptorPtr& socket1, SocketDescriptorPtr& socket2), (override));
    MOCK_METHOD(int, ioctlInt, (SOCKET fd, unsigned long int request, int* value), (override));
    MOCK_METHOD(int, setNoDelay, (SOCKET fd, bool noDelay));
    MOCK_METHOD(int, setNonBlocking, (SOCKET fd, bool nonBlock));
    MOCK_METHOD(int, setLinger, (SOCKET fd, bool on, int timeToLinger));
};

}   // namespace finalmq
