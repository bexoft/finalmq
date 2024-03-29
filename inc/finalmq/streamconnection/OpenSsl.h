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

#include <atomic>
#include <mutex>
#include <string>

#include "finalmq/logger/LogStream.h"

#ifdef USE_OPENSSL
#include "openssl/ossl_typ.h"
#endif

struct x509_store_ctx_st;
typedef struct x509_store_ctx_st X509_STORE_CTX;

namespace finalmq
{
struct CertificateData
{
    bool ssl = false;
    int verifyMode = 0;                                      // SSL_CTX_set_verify: SSL_VERIFY_NONE, SSL_VERIFY_PEER, SSL_VERIFY_FAIL_IF_NO_PEER_CERT, SSL_VERIFY_CLIENT_ONCE
    std::string certificateFile;                             // SSL_CTX_use_certificate_file, pem
    std::string privateKeyFile;                              // SSL_CTX_use_PrivateKey_file, pem
    std::string caFile;                                      // SSL_CTX_load_verify_location, pem
    std::string caPath;                                      // SSL_CTX_load_verify_location, pem
    std::string certificateChainFile;                        // SSL_CTX_use_certificate_chain_file, pem
    std::string clientCaFile;                                // SSL_load_client_CA_file, pem, SSL_CTX_set_client_CA_list
    std::function<int(int, X509_STORE_CTX*)> verifyCallback; // SSL_CTX_set_verify
};

} // namespace finalmq

#ifdef USE_OPENSSL

#define MODULENAME "FinalMQ"

#include <memory>
#include <mutex>

#include <assert.h>

#include <openssl/ssl.h>

namespace finalmq
{
class SslContext;
class SslSocket;

struct IOpenSsl
{
    virtual ~IOpenSsl()
    {}
    virtual std::shared_ptr<SslContext> createServerContext(const CertificateData& certificateData) = 0;
    virtual std::shared_ptr<SslContext> createClientContext(const CertificateData& certificateData) = 0;
    virtual std::mutex& getMutex() = 0;
};

class SYMBOLEXP OpenSslImpl : public IOpenSsl
{
public:
    OpenSslImpl();
    ~OpenSslImpl();

private:
    // IOpenSsl
    virtual std::shared_ptr<SslContext> createServerContext(const CertificateData& certificateData) override;
    virtual std::shared_ptr<SslContext> createClientContext(const CertificateData& certificateData) override;
    virtual std::mutex& getMutex() override;

    std::shared_ptr<SslContext> configContext(SSL_CTX* ctx, const CertificateData& certificateData);

    OpenSslImpl(const OpenSslImpl&) = delete;
    const OpenSslImpl& operator=(const OpenSslImpl&) = delete;

    std::mutex m_sslMutex{};
};

class SYMBOLEXP OpenSsl
{
public:
    inline static IOpenSsl& instance()
    {
        static auto& instanceRef = getStaticInstanceRef();
        auto* inst = instanceRef.load(std::memory_order_acquire);
        if (!inst)
        {
            inst = createInstance();
        }
        return *inst;
    }

    /**
    * Overwrite the default implementation, e.g. with a mock for testing purposes.
    * This method is not thread-safe. Make sure that no one uses the current instance before
    * calling this method.
    */
    static void setInstance(std::unique_ptr<IOpenSsl>&& instance);

private:
    OpenSsl() = delete;
    ~OpenSsl() = delete;
    static IOpenSsl* createInstance();

    static std::atomic<IOpenSsl*>& getStaticInstanceRef();
    static std::unique_ptr<IOpenSsl>& getStaticUniquePtrRef();
};

class SslSocket
{
public:
    enum class IoState
    {
        SUCCESS,
        WANT_READ,
        WANT_WRITE,
        SSL_ERROR
    };

    SslSocket(SSL* ssl)
        : m_ssl(ssl), m_sslMutex(OpenSsl::instance().getMutex())
    {
    }
    ~SslSocket()
    {
        if (m_ssl)
        {
            std::unique_lock<std::mutex> lock(m_sslMutex);
            SSL_free(m_ssl);
            m_ssl = nullptr;
        }
    }

    SSL* getSsl()
    {
        return m_ssl;
    }

    void startAccept()
    {
        assert(m_ssl);
        std::unique_lock<std::mutex> lock(m_sslMutex);
        SSL_set_accept_state(m_ssl);
    }

    IoState accepting()
    {
        assert(m_ssl);
        IoState state = IoState::SSL_ERROR;
        std::unique_lock<std::mutex> lock(m_sslMutex);
        int res = SSL_accept(m_ssl);
        if (res == 1)
        {
            state = IoState::SUCCESS;
        }
        else if (res == -1)
        {
            int err = SSL_get_error(m_ssl, res);
            if (err == SSL_ERROR_WANT_READ)
            {
                state = IoState::WANT_READ;
            }
            else if (err == SSL_ERROR_WANT_WRITE)
            {
                state = IoState::WANT_WRITE;
            }
            else
            {
                state = IoState::SSL_ERROR;
                streamError << "SSL_accept failed with " << err;
            }
        }
        return state;
    }

    void startConnect()
    {
        assert(m_ssl);
        std::unique_lock<std::mutex> lock(m_sslMutex);
        SSL_set_connect_state(m_ssl);
    }

    IoState connecting()
    {
        assert(m_ssl);
        IoState state = IoState::SSL_ERROR;
        std::unique_lock<std::mutex> lock(m_sslMutex);
        int res = SSL_connect(m_ssl);
        if (res == 1)
        {
            state = IoState::SUCCESS;
        }
        else if (res == -1)
        {
            int err = SSL_get_error(m_ssl, res);
            if (err == SSL_ERROR_WANT_READ)
            {
                state = IoState::WANT_READ;
            }
            else if (err == SSL_ERROR_WANT_WRITE)
            {
                state = IoState::WANT_WRITE;
            }
            else
            {
                state = IoState::SSL_ERROR;
                streamError << "SSL_connect failed with " << err;
            }
        }
        return state;
    }

    IoState read(char* buffer, int size, int& numRead)
    {
        assert(m_ssl);
        IoState state = IoState::SSL_ERROR;
        numRead = 0;
        std::unique_lock<std::mutex> lock(m_sslMutex);
        m_readWhenWritable = false;
        if (!m_writeWhenReadable)
        {
            int res = SSL_read(m_ssl, buffer, size);
            if ((res > 0) || (size == 0 && res == 0))
            {
                state = IoState::SUCCESS;
                numRead = res;
            }
            else if (res == -1)
            {
                res = SSL_get_error(m_ssl, res);
                if (res == SSL_ERROR_WANT_READ)
                {
                    state = IoState::WANT_READ;
                }
                else if (res == SSL_ERROR_WANT_WRITE)
                {
                    state = IoState::WANT_WRITE;
                    m_readWhenWritable = true;
                }
            }
        }
        return state;
    }

    IoState write(const char* buffer, int size, int& numRead)
    {
        assert(m_ssl);
        IoState state = IoState::SSL_ERROR;
        numRead = 0;
        std::unique_lock<std::mutex> lock(m_sslMutex);
        m_writeWhenReadable = false;
        if (!m_readWhenWritable)
        {
            int res = SSL_write(m_ssl, buffer, size);
            if (res > 0)
            {
                state = IoState::SUCCESS;
                numRead = res;
            }
            else if (res == -1)
            {
                res = SSL_get_error(m_ssl, res);
                if (res == SSL_ERROR_WANT_READ)
                {
                    state = IoState::WANT_READ;
                    m_writeWhenReadable = true;
                }
                else if (res == SSL_ERROR_WANT_WRITE)
                {
                    state = IoState::WANT_WRITE;
                }
            }
        }
        return state;
    }

    int sslPending()
    {
        std::unique_lock<std::mutex> lock(m_sslMutex);
        int pending = SSL_pending(m_ssl);
        return pending;
    }

    inline bool isReadWhenWritable() const
    {
        return m_readWhenWritable;
    }

    inline bool isWriteWhenReadable() const
    {
        return m_writeWhenReadable;
    }

private:
    SslSocket(const SslSocket&) = delete;
    const SslSocket& operator=(const SslSocket&) = delete;

    SSL* m_ssl = nullptr;
    bool m_readWhenWritable = false;
    bool m_writeWhenReadable = false;
    std::mutex& m_sslMutex;
};

class SslContext
{
public:
    SslContext(SSL_CTX* ctx)
        : m_ctx(ctx), m_sslMutex(OpenSsl::instance().getMutex())
    {
    }
    ~SslContext()
    {
        if (m_ctx)
        {
            std::unique_lock<std::mutex> lock(m_sslMutex);
            SSL_CTX_free(m_ctx);
            m_ctx = nullptr;
        }
    }

    SSL_CTX* getSslCtx()
    {
        return m_ctx;
    }

    std::shared_ptr<SslSocket> createSocket(SOCKET sd)
    {
        assert(m_ctx);
        std::unique_lock<std::mutex> lock(m_sslMutex);
        SSL* ssl = SSL_new(m_ctx);
        if (ssl)
        {
#ifdef WIN32
            SSL_set_fd(ssl, static_cast<int>(sd)); // the cast is not nice for win64 sockets, but it works!
#else
            SSL_set_fd(ssl, sd);
#endif
            return std::make_shared<SslSocket>(ssl);
        }
        return nullptr;
    }

    void setVerifyCallback(std::function<int(int, X509_STORE_CTX*)> verifyCallback)
    {
        m_verifyCallback = std::move(verifyCallback);
    }

    std::function<int(int, X509_STORE_CTX*)>& getVerifyCallback()
    {
        return m_verifyCallback;
    }

private:
    SslContext(const SslContext&) = delete;
    const SslContext& operator=(const SslContext&) = delete;

    SSL_CTX* m_ctx{nullptr};
    std::function<int(int, X509_STORE_CTX*)> m_verifyCallback{};
    std::mutex& m_sslMutex;
};

} // namespace finalmq

#undef MODULENAME

#endif // USE_OPENSSL
