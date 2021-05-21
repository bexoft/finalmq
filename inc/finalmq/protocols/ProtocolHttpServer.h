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
#include "finalmq/protocolconnection/IProtocol.h"
#include "finalmq/helpers/BexDefines.h"

#include <random>


namespace finalmq {


class SYMBOLEXP ProtocolHttpServer : public IProtocol
                             , private std::enable_shared_from_this<ProtocolHttpServer>
{
public:
    enum { PROTOCOL_ID = 4 };

    static const std::string FMQ_HTTP;
    static const std::string FMQ_METHOD;
    static const std::string FMQ_PROTOCOL;
    static const std::string FMQ_PATH;
    static const std::string FMQ_QUERY;
    static const std::string FMQ_STATUS;
    static const std::string FMQ_STATUSTEXT;
    static const std::string HTTP_REQUEST;
    static const std::string HTTP_RESPONSE;

    ProtocolHttpServer();

private:
    // IProtocol
    virtual void setCallback(const std::weak_ptr<IProtocolCallback>& callback) override;
    virtual std::uint32_t getProtocolId() const override;
    virtual bool areMessagesResendable() const override;
    virtual bool doesSupportMetainfo() const override;
    virtual bool doesSupportSession() const override;
    virtual bool needsReply() const override;
    virtual bool isMultiConnectionSession() const override;
    virtual bool isSendRequestByPoll() const override;
    virtual FuncCreateMessage getMessageFactory() const override;
    virtual void prepareMessageToSend(IMessagePtr message) override;
    virtual void moveOldProtocolState(IProtocol& protocolOld) override;
    virtual void received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) override;
    virtual hybrid_ptr<IStreamConnectionCallback> connected(const IStreamConnectionPtr& connection) override;
    virtual void disconnected(const IStreamConnectionPtr& connection) override;
    virtual IMessagePtr pollReply(std::deque<IMessagePtr>&& messages) override;


    bool receiveHeaders(ssize_t bytesReceived);
    void reset();
    std::string createSessionName();
    void checkSessionName();
    void cookiesToSessionIds(const std::string& cookies);

    enum State
    {
        STATE_FIND_FIRST_LINE,
        STATE_FIND_HEADERS,
        STATE_CONTENT,
        STATE_CONTENT_DONE
    };

    enum StateSessionId
    {
        SESSIONID_NONE = 0,
        SESSIONID_COOKIE = 1,
        SESSIONID_FMQ = 2
    };

    std::random_device                              m_randomDevice;
    std::mt19937                                    m_randomGenerator;
    std::uniform_int_distribution<std::uint64_t>    m_randomVariable;
    std::vector<std::string>                        m_headerSendNext;
    StateSessionId                                  m_stateSessionId = SESSIONID_NONE;
    std::vector<std::string>                        m_sessionNames;

    State                               m_state = STATE_FIND_FIRST_LINE;
    std::string                         m_receiveBuffer;
    ssize_t                             m_offsetRemaining = 0;
    ssize_t                             m_sizeRemaining = 0;
    IMessagePtr                         m_message;
    ssize_t                             m_contentLength = 0;
    ssize_t                             m_indexFilled = 0;
    std::string                         m_headerHost;
    std::int64_t                        m_connectionId = 0;
    bool                                m_createSession = false;
    std::string                         m_sessionName;
    std::weak_ptr<IProtocolCallback>    m_callback;

    // longpoll
    bool                                m_longpoll = false;

    static std::atomic_int64_t          m_nextSessionNameCounter;
};


class SYMBOLEXP ProtocolHttpServerFactory : public IProtocolFactory
{
public:

private:
    // IProtocolFactory
    virtual IProtocolPtr createProtocol() override;
};

}   // namespace finalmq
