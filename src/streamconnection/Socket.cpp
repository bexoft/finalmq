

#include "streamconnection/Socket.h"
#include "helpers/OperatingSystem.h"


#include <errno.h>
#include <string.h>
#include <iostream>
#include <assert.h>

#if !defined(MSVCPP) && !defined(__MINGW32__)
#include <netinet/tcp.h>
#include <fcntl.h>
#include <sys/unistd.h>
#endif





Socket::Socket()
{
}


Socket::~Socket()
{
    destroy();
}


int Socket::create(int af, int type, int protocol)
{
    destroy();
    m_af = af;
    m_protocol = protocol;
    int err = OperatingSystem::instance().socket(af, type, protocol);
    if (err != -1)
    {
        int sd = err;
        m_sd = std::make_shared<SocketDescriptor>(sd);
        OperatingSystem::instance().setNonBlocking(sd, true);
        OperatingSystem::instance().setLinger(sd, true, 0);
        if (protocol == IPPROTO_TCP)
        {
            OperatingSystem::instance().setNoDelay(sd, true);
        }
    }
    err = handleError(err, "create socket");
    return err;
}



#ifdef USE_OPENSSL
int Socket::createSslServer(int af, int type, int protocol, const CertificateData& certificateData)
{
    int err = this->create(af, type, protocol);
    if (err != -1)
    {
        assert(m_sd->getDescriptor());
        err = -1;
        m_sslContext = OpenSsl::instance().createServerContext(certificateData);
        if (m_sslContext)
        {
            err = m_sd->getDescriptor();
        }
    }
    return err;
}

int Socket::createSslClient(int af, int type, int protocol, const CertificateData& certificateData)
{
    int err = create(af, type, protocol);
    if (err != -1)
    {
        assert(m_sd->getDescriptor());
        err = -1;
        m_sslContext = OpenSsl::instance().createClientContext(certificateData);
        if (m_sslContext)
        {
            m_sslSocket = m_sslContext->createSocket(m_sd->getDescriptor());
            if (m_sslSocket)
            {
                err = m_sd->getDescriptor();
            }
        }
    }
    return err;
}
#endif


bool Socket::isValid() const
{
    return (m_sd != nullptr);
}

int Socket::connect(const sockaddr* addr, int addrlen)
{
    assert(m_sd);
    int err = OperatingSystem::instance().connect(m_sd->getDescriptor(), addr, addrlen);
    err = handleError(err, "connect");
#ifdef USE_OPENSSL
    if (m_sslContext && err != -1)
    {
        assert(m_sslSocket);
        m_sslSocket->startConnect();
    }
#endif
    return err;
}

int Socket::accept(sockaddr* addr, socklen_t* addrlen, SocketPtr& socketAccept)
{
    socketAccept = nullptr;
    assert(m_sd);
    int err = OperatingSystem::instance().accept(m_sd->getDescriptor(), addr, (socklen_t*)addrlen);
    if (err != -1)
    {
        int sd = err;
        socketAccept = std::make_shared<Socket>();
        socketAccept->attach(sd);
#ifdef USE_OPENSSL
        if (m_sslContext)
        {
            socketAccept->startSslAccept(m_sslContext);
        }
#endif
    }
    err = handleError(err, "accept");
    return err;
}


int Socket::bind(const sockaddr* addr, int addrlen)
{
#if !defined(MSVCPP) && !defined(__MINGW32__)
    if (m_af == AF_UNIX)
    {
        struct sockaddr_un* addrunix = (sockaddr_un*)addr;
        if (addrunix)
        {
            m_name = addrunix->sun_path;
        }
    }
#endif
    assert(m_sd);
    int err = OperatingSystem::instance().bind(m_sd->getDescriptor(), addr, addrlen);
    err = handleError(err, "bind");
    return err;
}


int Socket::listen(int backlog)
{
    assert(m_sd);
    int err = OperatingSystem::instance().listen(m_sd->getDescriptor(), backlog);
    err = handleError(err, "listen");
    return err;
}




int Socket::send(const char* buf, int len, int flags)
{
    assert(m_sd);
    int err = 0;
    int lenWritten = 0;
    bool ex = false;
    while (!ex)
    {
#ifdef USE_OPENSSL
        if (m_sslContext)
        {
            m_writeWhenReadable = false;
            assert(m_sslSocket);
            SslSocket::IoState state = m_sslSocket->write(buf, len, err);
            if (state == SslSocket::IoState::WANT_READ)
            {
                ex = true;
                err = 0;
                m_writeWhenReadable = true;
            }
            else if (state == SslSocket::IoState::WANT_WRITE)
            {
                ex = true;
                err = 0;
            }
        }
        else
#endif
        {
            do
            {
                err = OperatingSystem::instance().send(m_sd->getDescriptor(), buf, len, flags);
            } while(err == -1 && getLastError() == SOCKETERROR(EINTR));
        }

        if (err > 0)
        {
            buf += err;
            len -= err;
            lenWritten += err;
            err = 0;
            assert(len >= 0);
            if (len == 0)
            {
                ex = true;
            }
        }
        else
        {
            ex = true;
        }
    }
    err = handleError(err, "write");
    if (err == 0)
    {
        err = lenWritten;
    }
    return err;
}



int Socket::receive(char* buf, int len, int flags)
{
    assert(m_sd);
    int err = 0;
    int lenReceived = 0;
    bool ex = false;
    while (!ex)
    {
#ifdef USE_OPENSSL
        if (m_sslContext)
        {
            m_readWhenWritable = false;
            assert(m_sslSocket);
            SslSocket::IoState state = m_sslSocket->read(buf, len, err);
            if (state == SslSocket::IoState::WANT_READ)
            {
                ex = true;
                err = 0;
            }
            else if (state == SslSocket::IoState::WANT_WRITE)
            {
                ex = true;
                err = 0;
                m_readWhenWritable = true;
            }
        }
        else
#endif
        {
            do
            {
                err = OperatingSystem::instance().recv(m_sd->getDescriptor(), buf, len, flags);
            } while(err == -1 && getLastError() == SOCKETERROR(EINTR));
        }

        if (err > 0)
        {
            buf += err;
            len -= err;
            lenReceived += err;
            err = 0;
            assert(len >= 0);
            if (len == 0)
            {
                ex = true;
            }
        }
        else
        {
            ex = true;
        }
    }
    err = handleError(err, "read");
    if (err == 0)
    {
        err = lenReceived;
    }
    return err;
}


void Socket::destroy()
{
    if (m_sd)
    {
        m_sd = nullptr;
        if (!m_name.empty())
        {
            ::unlink(m_name.c_str());
        }
    }
}



SocketDescriptorPtr Socket::getSocketDescriptor() const
{
    return m_sd;
}





void Socket::attach(int sd)
{
    if (sd != INVALID_FD)
    {
        m_sd = std::make_shared<SocketDescriptor>(sd);
        OperatingSystem::instance().setNonBlocking(sd, true);
        OperatingSystem::instance().setLinger(sd, true, 0);
        if (m_protocol == IPPROTO_TCP)
        {
            OperatingSystem::instance().setNoDelay(sd, true);
        }
    }
}


int Socket::pendingRead() const
{
    assert(m_sd);
    int countRead = 0;
    int resIoCtl = OperatingSystem::instance().ioctlInt(m_sd->getDescriptor(), FIONREAD, &countRead);
    if (resIoCtl == -1)
    {
        countRead = 0;
    }
    return countRead;
}


int Socket::getLastError()
{
    int errorNumber = OperatingSystem::instance().getLastError();
    if (errorNumber == SOCKETERROR(EAGAIN))
    {
        errorNumber = SOCKETERROR(EWOULDBLOCK);
    }
    return errorNumber;
}


int Socket::handleError(int err, const char* funcName)
{
    if (err == -1)
    {
        if (getLastError() == SOCKETERROR(EWOULDBLOCK))
        {
            err = 0;
        }
        else
        {
            std::cout << funcName << " failed with error " << getLastError();
        }
    }
    return err;
}


#ifdef USE_OPENSSL

SSL_CTX* Socket::getSslCtx()
{
    if (m_sslContext)
    {
        return m_sslContext->getSslCtx();
    }
    return nullptr;
}

SSL* Socket::getSslSocket()
{
    if (m_sslSocket)
    {
        return m_sslSocket->getSsl();
    }
    return nullptr;
}

void Socket::startSslAccept(const std::shared_ptr<SslContext>& sslContext)
{
    assert(sslContext);
    assert(m_sd);
    assert(m_sd->getDescriptor());
    m_sslContext = sslContext;
    m_sslSocket = m_sslContext->createSocket(m_sd->getDescriptor());
    m_sslSocket->startAccept();
}

bool Socket::isSsl() const
{
    return !!m_sslContext;
}
bool Socket::isReadWhenWritable() const
{
    return m_readWhenWritable;
}
bool Socket::isWriteWhenReadable() const
{
    return m_writeWhenReadable;
}
int Socket::sslPending()
{
    assert(m_sslSocket);
    return m_sslSocket->sslPending();
}



SslSocket::IoState Socket::sslAccepting()
{
    assert(m_sslSocket);
    return m_sslSocket->accepting();
}


SslSocket::IoState Socket::sslConnecting()
{
    assert(m_sslSocket);
    return m_sslSocket->connecting();
}


#endif
