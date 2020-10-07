#pragma once

#include <string>

struct CertificateData
{
    std::string certificateFile;        // SSL_CTX_use_certificate_file, pem
    std::string privateKeyFile;         // SSL_CTX_use_PrivateKey_file, pem
    std::string caFile;                 // SSL_CTX_load_verify_location, pem
    std::string caPath;                 // SSL_CTX_load_verify_location, pem
    std::string certificateChainFile;   // SSL_CTX_use_certificate_chain_file, pem
    std::string clientCaFile;           // SSL_load_client_CA_file, pem, SSL_CTX_set_client_CA_list, SSL_CTX_set_verify
};


#ifdef USE_OPENSSL


#include <assert.h>
#include <memory>
#include <mutex>

#include <openssl/ssl.h>



class SslContext;
class SslSocket;


struct IOpenSsl
{
    virtual ~IOpenSsl() {}
    virtual std::shared_ptr<SslContext> createServerContext(const CertificateData& certificateData) = 0;
    virtual std::shared_ptr<SslContext> createClientContext(const CertificateData& certificateData) = 0;
    virtual std::mutex& getMutex() = 0;
};



class OpenSslImpl : public IOpenSsl
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
    const OpenSslImpl& operator =(const OpenSslImpl&) = delete;

    std::mutex m_sslMutex;
};


class OpenSsl
{
public:
    static IOpenSsl& instance();
    static void setInstance(std::unique_ptr<IOpenSsl>&& instance);

private:
    OpenSsl() = delete;

    static std::unique_ptr<IOpenSsl> m_instance;
};




class SslSocket
{
public:

    enum class IoState
    {
        SUCCESS,
        WANT_READ,
        WANT_WRITE,
        ERROR
    };

    SslSocket(SSL* ssl)
        : m_ssl(ssl)
        , m_sslMutex(OpenSsl::instance().getMutex())
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
        IoState state = IoState::ERROR;
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
                state = IoState::ERROR;
//                std::cout << "SSL_accept failed with " << err << std::endl;
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
        IoState state = IoState::ERROR;
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
                state = IoState::ERROR;
//                std::cout << "SSL_connect failed with " << err << std::endl;
            }
        }
        return state;
    }

    IoState read(char* buffer, int size, int& numRead)
    {
        IoState state = IoState::ERROR;
        numRead = 0;
        std::unique_lock<std::mutex> lock(m_sslMutex);
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
            }
        }
        return state;
    }

    IoState write(const char* buffer, int size, int& numRead)
    {
        IoState state = IoState::ERROR;
        numRead = 0;
        std::unique_lock<std::mutex> lock(m_sslMutex);
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
            }
            else if (res == SSL_ERROR_WANT_WRITE)
            {
                state = IoState::WANT_WRITE;
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

private:
    SslSocket(const SslSocket&) = delete;
    const SslSocket& operator =(const SslSocket&) = delete;

    SSL* m_ssl = nullptr;
    std::mutex& m_sslMutex;
};



class SslContext
{
public:
    SslContext(SSL_CTX* ctx)
        : m_ctx(ctx)
        , m_sslMutex(OpenSsl::instance().getMutex())
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

    std::shared_ptr<SslSocket> createSocket(int sd)
    {
        assert(m_ctx);
        std::unique_lock<std::mutex> lock(m_sslMutex);
        SSL* ssl = SSL_new(m_ctx);
        if (ssl)
        {
            SSL_set_fd(ssl, sd);
            return std::make_shared<SslSocket>(ssl);
        }
        return nullptr;
    }

private:
    SslContext(const SslContext&) = delete;
    const SslContext& operator =(const SslContext&) = delete;

    SSL_CTX* m_ctx = nullptr;
    std::mutex& m_sslMutex;
};




#endif  // USE_OPENSSL
