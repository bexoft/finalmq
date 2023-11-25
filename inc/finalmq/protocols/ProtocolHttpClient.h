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

#include "finalmq/streamconnection/IMessage.h"
#include "finalmq/protocolsession/IProtocol.h"
#include "finalmq/helpers/FmqDefines.h"
#include "finalmq/helpers/Executor.h"

#include <random>


namespace finalmq {

class Cookie;

class CookieStore
{
public:
    void add(const Cookie& cookie);
    const Cookie* getCookie(const std::string& host, const std::string& path) const;

private:
    std::unique_ptr<Cookie>* CookieStore::getCookieIntern(const std::string& host, const std::string& path);

    std::list<std::unique_ptr<Cookie>> m_cookies;
};


class SYMBOLEXP ProtocolHttpClient : public IProtocol
                                   , public std::enable_shared_from_this<ProtocolHttpClient>
{
public:
    static const std::uint32_t PROTOCOL_ID;           // 7
    static const std::string PROTOCOL_NAME; // httpclient

    static const std::string FMQ_HTTP;
    static const std::string FMQ_METHOD;
    static const std::string FMQ_PROTOCOL;
    static const std::string FMQ_PATH;
    static const std::string FMQ_QUERY_PREFIX;
    static const std::string FMQ_HTTP_STATUS;
    static const std::string FMQ_HTTP_STATUSTEXT;
    static const std::string HTTP_REQUEST;
    static const std::string HTTP_RESPONSE;

    ProtocolHttpClient();
    virtual ~ProtocolHttpClient();

private:

    // IProtocol
    virtual void setCallback(const std::weak_ptr<IProtocolCallback>& callback) override;
    virtual void setConnection(const IStreamConnectionPtr& connection) override;
    virtual IStreamConnectionPtr getConnection() const override;
    virtual void disconnect() override;
    virtual std::uint32_t getProtocolId() const override;
    virtual bool areMessagesResendable() const override;
    virtual bool doesSupportMetainfo() const override;
    virtual bool doesSupportSession() const override;
    virtual bool needsReply() const override;
    virtual bool isMultiConnectionSession() const override;
    virtual bool isSendRequestByPoll() const override;
    virtual bool doesSupportFileTransfer() const override;
    virtual bool isSynchronousRequestReply() const override;
    virtual FuncCreateMessage getMessageFactory() const override;
    virtual void sendMessage(IMessagePtr message) override;
    virtual void moveOldProtocolState(IProtocol& protocolOld) override;
    virtual bool received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) override;
    virtual hybrid_ptr<IStreamConnectionCallback> connected(const IStreamConnectionPtr& connection) override;
    virtual void disconnected(const IStreamConnectionPtr& connection) override;
    virtual IMessagePtr pollReply(std::deque<IMessagePtr>&& messages) override;
    virtual void subscribe(const std::vector<std::string>& subscribtions) override;
    virtual void cycleTime() override;


    bool receiveHeaders(ssize_t bytesReceived);
    void reset();
//    bool handleInternalCommands(const std::shared_ptr<IProtocolCallback>& callback, bool& ok);

    enum class State
    {
        STATE_FIND_FIRST_LINE,
        STATE_FIND_HEADERS,
        STATE_CONTENT,
        STATE_CONTENT_DONE
    };

    enum class StateSessionId
    {
        SESSIONID_NONE = 0,
        SESSIONID_COOKIE = 1,
        SESSIONID_FMQ = 2
    };

    std::random_device                              m_randomDevice;
    std::mt19937                                    m_randomGenerator;
    std::uniform_int_distribution<std::uint64_t>    m_randomVariable;
    IMessage::Metainfo                              m_headerSendNext;

    State                               m_state = State::STATE_FIND_FIRST_LINE;
    std::string                         m_receiveBuffer;
    ssize_t                             m_offsetRemaining = 0;
    ssize_t                             m_sizeRemaining = 0;
    IMessagePtr                         m_message;
    ssize_t                             m_contentLength = 0;
    ssize_t                             m_indexFilled = 0;
    std::string                         m_headerHost;
    std::int64_t                        m_connectionId = 0;
    std::weak_ptr<IProtocolCallback>    m_callback;
    IStreamConnectionPtr                m_connection;
    bool                                m_multipart = false;

    CookieStore                         m_cookieStore;

    mutable std::mutex                  m_mutex;
    static std::atomic_int64_t          m_nextSessionNameCounter;
};


class SYMBOLEXP ProtocolHttpClientFactory : public IProtocolFactory
{
public:

private:
    // IProtocolFactory
    virtual IProtocolPtr createProtocol(const Variant& data) override;
};

}   // namespace finalmq
