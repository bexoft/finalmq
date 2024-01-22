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

#include <memory>
#include <mutex>
#include <vector>

#include "OpenSsl.h"
#include "finalmq/helpers/OperatingSystem.h"
#include "finalmq/helpers/SocketDescriptor.h"

#if !defined(WIN32) && !defined(__MINGW32__)
#include <netdb.h>
#endif

namespace finalmq
{
class SslContext;
class SslSocket;

class Socket;
typedef std::shared_ptr<Socket> SocketPtr;

class SYMBOLEXP Socket
{
public:
    Socket();
    ~Socket();

    bool create(int af, int type, int protocol);
#ifdef USE_OPENSSL
    bool createSslServer(int af, int type, int protocol, const CertificateData& certificateData);
    bool createSslClient(int af, int type, int protocol, const CertificateData& certificateData);
#endif
    int connect(const sockaddr* addr, int addrlen);
    bool accept(sockaddr* addr, socklen_t* addrlen, SocketPtr& socketAccept);
    int bind(const sockaddr* addr, int namelen);
    int listen(int backlog);
    int send(const char* buf, int len, int flags = 0);
    int receive(char* buf, int len, int flags = 0);
    void destroy();
    void attach(SOCKET sd);
    int pendingRead() const;
    SocketDescriptorPtr getSocketDescriptor() const;

    bool isValid() const;

    static int getLastError();

private:
    Socket(const Socket& obj) = delete;
    const Socket& operator=(const Socket& obj) = delete;

    int handleError(int err, const char* funcName);

    SocketDescriptorPtr m_sd{};
    int m_af = 0;
    int m_protocol = 0;
    std::string m_name{};

#ifdef USE_OPENSSL
public:
    SSL_CTX* getSslCtx();
    SSL* getSslSocket();
    SslSocket::IoState sslAccepting();
    SslSocket::IoState sslConnecting();
    inline bool isSsl() const
    {
        return !!m_sslContext;
    }
    inline bool isReadWhenWritable() const
    {
        if (m_sslContext)
        {
            assert(m_sslSocket);
            return m_sslSocket->isReadWhenWritable();
        }
        return false;
    }
    inline bool isWriteWhenReadable() const
    {
        if (m_sslContext)
        {
            assert(m_sslSocket);
            return m_sslSocket->isWriteWhenReadable();
        }
        return false;
    }
    int sslPending();

private:
    void startSslAccept(const std::shared_ptr<SslContext>& sslContext);
#endif
    std::shared_ptr<SslContext> m_sslContext{};
    std::shared_ptr<SslSocket> m_sslSocket{};
    bool m_readWhenWritable = false;
    bool m_writeWhenReadable = false;
    //std::mutex          m_mtx{};
};

} // namespace finalmq
