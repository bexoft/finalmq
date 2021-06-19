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

#ifdef USE_OPENSSL

#include "finalmq/streamconnection/OpenSsl.h"
#include "finalmq/logger/LogStream.h"
#include "finalmq/helpers/ModulenameFinalmq.h"

#include <iostream>

#include <openssl/err.h>

namespace finalmq {


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

    // SSL_CTX_set_read_ahead(ctx, true);
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
    // SSL_CTX_set_mode(ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER | SSL_MODE_AUTO_RETRY);// | SSL_MODE_ENABLE_PARTIAL_WRITE);

    std::unique_lock<std::mutex> lock(m_sslMutex);

    if (!certificateData.certificateFile.empty())
    {
        if (SSL_CTX_use_certificate_file(ctx, certificateData.certificateFile.c_str(), SSL_FILETYPE_PEM) < 0)
        {
            streamError << "SSL_CTX_use_certificate_file failed";
            return nullptr;
        }
    }
    if (!certificateData.privateKeyFile.empty())
    {
        if (SSL_CTX_use_PrivateKey_file(ctx, certificateData.privateKeyFile.c_str(), SSL_FILETYPE_PEM) < 0)
        {
            streamError << "SSL_CTX_use_PrivateKey_file failed";
            return nullptr;
        }
    }
    const char* cafile = certificateData.caFile.empty() ? nullptr : certificateData.caFile.c_str();
    const char* capath = certificateData.caPath.empty() ? nullptr : certificateData.caPath.c_str();
    if (SSL_CTX_load_verify_locations(ctx, cafile, capath) < 0)
    {
        streamError << "SSL_CTX_load_verify_locations failed";
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
        }
    }

    sslContext->setVerifyCallback(std::move(certificateData.verifyCallback));
    typedef int VerifyCallback(int, X509_STORE_CTX*);
    VerifyCallback** pfunc = sslContext->getVerifyCallback().target<VerifyCallback*>();
    VerifyCallback* func = nullptr;
    if (pfunc)
    {
        func = *pfunc;
    }
    SSL_CTX_set_verify(ctx, certificateData.verifyMode, func);

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
        streamError << "create SSL context failed";
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
        streamError << "create SSL context failed";
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

void OpenSsl::setInstance(std::unique_ptr<IOpenSsl>&& instance)
{
    m_instance = std::move(instance);
}


}   // namespace finalmq

#endif  // USE_OPENSSL
