#pragma once

#include "SocketDescriptor.h"

struct IOperatingSystem
{
    virtual ~IOperatingSystem() {}
	virtual int close(int fd) = 0;
    virtual int closeSocket(int fd) = 0;
	virtual int socket(int af, int type, int protocol) = 0;
    virtual int bind(int fd, const struct sockaddr* name, socklen_t namelen) = 0;
    virtual int accept(int fd, struct sockaddr* addr, socklen_t* addrlen) = 0;
	virtual int listen(int fd, int backlog) = 0;
    virtual int connect(int fd, const struct sockaddr* name, socklen_t namelen) = 0;
	virtual int setsockopt(int fd, int level, int optname, const char* optval, int optlen) = 0;
    virtual int getsockname(int fd, struct sockaddr* name, socklen_t* namelen) = 0;
    virtual int write(int fd, const void* buffer, size_t len) = 0;
    virtual int read(int fd, void* buffer, size_t len) = 0;
    virtual int send(int fd, const void* buffer, size_t len, int flags) = 0;
    virtual int recv(int fd, void* buffer, size_t len, int flags) = 0;
    virtual int getLastError() = 0;
	virtual int select (int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout) = 0;
	virtual int epoll_create1(int flags) = 0; 
	virtual int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event) = 0;
	virtual int epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout, const sigset_t* sigmask) = 0;
    virtual int makeSocketPair(SocketDescriptorPtr& socket1, SocketDescriptorPtr& socket2) = 0;
    virtual int ioctlInt(int fd, unsigned long int request, int* value) = 0;
    virtual int setNoDelay(int fd, bool noDelay) = 0;
    virtual int setNonBlocking(int fd, bool nonBlock) = 0;
    virtual int setLinger(int fd, bool on, int timeToLinger) = 0;
};




