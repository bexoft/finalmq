#pragma once


#include "helpers/IOperatingSystem.h"


#include "gmock/gmock.h"

class MockIOperatingSystem : public IOperatingSystem
{
public:
    MOCK_METHOD(int, close, (int fd), (override));
    MOCK_METHOD(int, closeSocket, (int fd), (override));
    MOCK_METHOD(int, socket, (int af, int type, int protocol), (override));
    MOCK_METHOD(int, bind, (int fd, const struct sockaddr* name, socklen_t namelen), (override));
    MOCK_METHOD(int, accept, (int fd, struct sockaddr* addr, socklen_t* addrlen), (override));
    MOCK_METHOD(int, listen, (int fd, int backlog), (override));
    MOCK_METHOD(int, connect, (int fd, const struct sockaddr* name, socklen_t namelen), (override));
    MOCK_METHOD(int, setsockopt, (int fd, int level, int optname, const char* optval, int optlen), (override));
    MOCK_METHOD(int, getsockname, (int fd, struct sockaddr* name, socklen_t* namelen), (override));
    MOCK_METHOD(int, write, (int fd, const void* buffer, size_t len), (override));
    MOCK_METHOD(int, read, (int fd, void* buffer, size_t len), (override));
    MOCK_METHOD(int, send, (int fd, const void* buffer, size_t len, int flags), (override));
    MOCK_METHOD(int, recv, (int fd, void* buffer, size_t len, int flags), (override));
    MOCK_METHOD(int, getLastError, (), (override));
    MOCK_METHOD(int, select, (int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout), (override));
    MOCK_METHOD(int, epoll_create1, (int flags), (override));
    MOCK_METHOD(int, epoll_ctl, (int epfd, int op, int fd, struct epoll_event* event), (override));
    MOCK_METHOD(int, epoll_pwait, (int epfd, struct epoll_event *events, int maxevents, int timeout, const sigset_t* sigmask), (override));
    MOCK_METHOD(int, makeSocketPair, (SocketDescriptorPtr& socket1, SocketDescriptorPtr& socket2), (override));
    MOCK_METHOD(int, ioctlInt, (int fd, unsigned long int request, int* value), (override));
    MOCK_METHOD(int, setNoDelay, (int fd, bool noDelay));
    MOCK_METHOD(int, setNonBlocking, (int fd, bool nonBlock));
    MOCK_METHOD(int, setLinger, (int fd, bool on, int timeToLinger));
};

