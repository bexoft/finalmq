#ifdef USE_OPENSSL

#include "streamconnection/OpenSsl.h"

#include <iostream>

#include <openssl/err.h>




OpenSslImpl::OpenSslImpl()
{
    std::unique_lock<std::mutex> lock(m_sslMutex);
    SSL_load_error_strings();
    ERR_load_crypto_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();
    SSL_library_init();
}

OpenSslImpl::~OpenSslImpl()
{
    std::unique_lock<std::mutex> lock(m_sslMutex);
    EVP_cleanup();
}


std::shared_ptr<SslContext> OpenSslImpl::configContext(SSL_CTX* ctx, const CertificateData& certificateData)
{
    std::shared_ptr<SslContext> sslContext = std::make_shared<SslContext>(ctx);

    //    SSL_CTX_set_read_ahead(ctx, true);
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
    //SSL_CTX_set_mode(ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER | SSL_MODE_AUTO_RETRY);// | SSL_MODE_ENABLE_PARTIAL_WRITE);

    std::unique_lock<std::mutex> lock(m_sslMutex);

    if (!certificateData.certificateFile.empty())
    {
        if (SSL_CTX_use_certificate_file(ctx, certificateData.certificateFile.c_str(), SSL_FILETYPE_PEM) < 0)
        {
            std::cout << "SSL_CTX_use_certificate_file failed" << std::endl;
            return nullptr;
        }
    }
    if (!certificateData.privateKeyFile.empty())
    {
        if (SSL_CTX_use_PrivateKey_file(ctx, certificateData.privateKeyFile.c_str(), SSL_FILETYPE_PEM) < 0)
        {
            std::cout << "SSL_CTX_use_PrivateKey_file failed" << std::endl;
            return nullptr;
        }
    }
    const char* cafile = certificateData.caFile.empty() ? nullptr : certificateData.caFile.c_str();
    const char* capath = certificateData.caPath.empty() ? nullptr : certificateData.caPath.c_str();
    if (SSL_CTX_load_verify_locations(ctx, cafile, capath) < 0)
    {
        std::cout << "SSL_CTX_load_verify_locations failed" << std::endl;
        return nullptr;
    }
    if (!certificateData.certificateChainFile.empty())
    {
        SSL_CTX_use_certificate_chain_file(ctx, certificateData.certificateChainFile.c_str());
    }
    if (!certificateData.clientCaFile.empty())
    {
        auto list = SSL_load_client_CA_file(certificateData.clientCaFile.c_str());
        if (list)
        {
            SSL_CTX_set_client_CA_list(ctx, list);
            SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
        }
    }

    return sslContext;
}


// IOpenSsl

std::shared_ptr<SslContext> OpenSslImpl::createServerContext(const CertificateData& certificateData)
{
    std::unique_lock<std::mutex> lock(m_sslMutex);
    SSL_CTX* ctx = SSL_CTX_new(SSLv23_server_method());
    lock.unlock();
    if (ctx == nullptr)
    {
        std::cout << "create SSL context failed" << std::endl;
        return nullptr;
    }

    std::shared_ptr<SslContext> sslContext = configContext(ctx, certificateData);
    return sslContext;
}

std::shared_ptr<SslContext> OpenSslImpl::createClientContext(const CertificateData& certificateData)
{
    std::unique_lock<std::mutex> lock(m_sslMutex);
    SSL_CTX* ctx = SSL_CTX_new(SSLv23_client_method());
    lock.unlock();
    if (ctx == nullptr)
    {
        std::cout << "create SSL context failed" << std::endl;
        return nullptr;
    }

    std::shared_ptr<SslContext> sslContext = configContext(ctx, certificateData);
    return sslContext;
}


std::mutex& OpenSslImpl::getMutex()
{
    return m_sslMutex;
}

std::unique_ptr<IOpenSsl> OpenSsl::m_instance;

IOpenSsl& OpenSsl::instance()
{
    if (!m_instance)
    {
        m_instance = std::make_unique<OpenSslImpl>();
    }
    return *m_instance;
}

void OpenSsl::setInstance(std::unique_ptr<IOpenSsl>&& instance)
{
    m_instance = std::move(instance);
}



#endif  // USE_OPENSSL
