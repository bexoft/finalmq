#pragma once


#include <memory>

#if defined(WIN32) || defined(__MINGW32__)

#include <winsock2.h>
#define SOCKETERROR(err)	WSA##err
typedef	int					socklen_t;
typedef sockaddr t_sockaddr;

#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <sys/epoll.h>
#define SOCKETERROR(err)	err

#endif


#include "helpers/BexDefines.h"
#include "helpers/IOperatingSystem.h"




class OperatingSystemImpl : public IOperatingSystem
{
public:
    OperatingSystemImpl();
    
private:
    // IOperatingSystem
	virtual int close(int fd) override;
    virtual int closeSocket(int fd) override;
	virtual int socket(int af, int type, int protocol) override;
    virtual int bind(int fd, const struct sockaddr* name, socklen_t namelen) override;
    virtual int accept(int fd, struct sockaddr* addr, socklen_t* addrlen) override;
	virtual int listen(int fd, int backlog) override;
    virtual int connect(int fd, const struct sockaddr* name, socklen_t namelen) override;
	virtual int setsockopt(int fd, int level, int optname, const char* optval, int optlen) override;
    virtual int getsockname(int fd, struct sockaddr* name, socklen_t* namelen) override;
    virtual int write(int fd, const void* buffer, size_t len) override;
    virtual int read(int fd, void* buffer, size_t len) override;
    virtual int send(int fd, const void* buffer, size_t len, int flags) override;
    virtual int recv(int fd, void* buffer, size_t len, int flags) override;
    virtual int getLastError() override;
	virtual int select (int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout) override;
	virtual int epoll_create1(int flags) override; 
	virtual int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event) override;
	virtual int epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout, const sigset_t* sigmask) override;
    virtual int makeSocketPair(SocketDescriptorPtr& socket1, SocketDescriptorPtr& socket2) override;
    virtual int ioctlInt(int fd, unsigned long int request, int* value) override;
    virtual int setNoDelay(int fd, bool noDelay) override;
    virtual int setNonBlocking(int fd, bool nonBlock) override;
    virtual int setLinger(int fd, bool on, int timeToLinger) override;

	OperatingSystemImpl(const OperatingSystemImpl&) = delete;
	const OperatingSystemImpl& operator =(const OperatingSystemImpl&) = delete;

};


class OperatingSystem
{
public:
    static IOperatingSystem& instance();
    static void setInstance(std::unique_ptr<IOperatingSystem>& instance);

private:
    OperatingSystem() = delete;

    static std::unique_ptr<IOperatingSystem> m_instance;
};


