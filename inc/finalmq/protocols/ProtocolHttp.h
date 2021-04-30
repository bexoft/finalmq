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


class SYMBOLEXP ProtocolHttp : public IProtocol
                             , private std::enable_shared_from_this<ProtocolHttp>
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

    ProtocolHttp();

private:
    // IProtocol
    virtual void setCallback(const std::weak_ptr<IProtocolCallback>& callback) override;
    virtual std::uint32_t getProtocolId() const override;
    virtual bool areMessagesResendable() const override;
    virtual bool doesSupportMetainfo() const override;
    virtual bool doesSupportSession() const override;
    virtual bool needsReply() const override;
    virtual FuncCreateMessage getMessageFactory() const override;
    virtual void receive(const SocketPtr& socket, int bytesToRead) override;
    virtual void prepareMessageToSend(IMessagePtr message) override;
    virtual void socketConnected(IProtocolSession& session) override;
    virtual void socketDisconnected() override;

    bool receiveHeaders(ssize_t bytesReceived);
    void reset();
    std::string createSessionName();
    void checkSessionName();

    enum State
    {
        STATE_FIND_FIRST_LINE,
        STATE_FIND_HEADERS,
        STATE_CONTENT,
        STATE_CONTENT_DONE
    };

    std::random_device                              m_randomDevice;
    std::mt19937                                    m_randomGenerator;
    std::uniform_int_distribution<std::uint64_t>    m_randomVariable;

    State                               m_state = STATE_FIND_FIRST_LINE;
    std::string                         m_receiveBuffer;
    ssize_t                             m_offsetRemaining = 0;
    ssize_t                             m_sizeRemaining = 0;
    IMessagePtr                         m_message;
    ssize_t                             m_contentLength = 0;
    ssize_t                             m_indexFilled = 0;
    std::string                         m_headerHost;
    bool                                m_checkSessionName = false;
    std::string                         m_sessionName;
    std::uint64_t                       m_nextSessionNameCounter = 1;
    std::weak_ptr<IProtocolCallback>    m_callback;
};


class SYMBOLEXP ProtocolHttpFactory : public IProtocolFactory
{
public:

private:
    // IProtocolFactory
    virtual IProtocolPtr createProtocol() override;
};

}   // namespace finalmq
