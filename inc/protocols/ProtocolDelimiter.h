#pragma once

#include "streamconnection/IMessage.h"
#include "protocolconnection/IProtocol.h"
#include <vector>





class ProtocolDelimiter : public IProtocol
{
public:
    ProtocolDelimiter(const std::string& delimiter);

private:
    // IProtocol
    virtual void setCallback(const std::weak_ptr<IProtocolCallback>& callback) override;
    virtual int getProtocolId() const override;
    virtual bool areMessagesResendable() const override;
    virtual IMessagePtr createMessage() const override;
    virtual void receive(const SocketPtr& socket, int bytesToRead) override;
    virtual void prepareMessageToSend(IMessagePtr message) override;
    virtual void socketConnected() override;
    virtual void socketDisconnected() override;

    std::vector<int> findEndOfMessage(const char* buffer, int size);

    std::weak_ptr<IProtocolCallback>    m_callback;

    std::string                         m_delimiter;

    std::list<std::string>              m_receiveBuffers;
    int                                 m_indexStartBuffer = 0;
    int                                 m_characterCounter = 0;
    int                                 m_indexDelimiter = -1;

    std::string                         m_delimiterPartial;

    const int                           m_protocolId = 0x3a948e1c;
};


class ProtocolDelimiterFactory : public IProtocolFactory
{
public:
    ProtocolDelimiterFactory(const std::string& delimiter);

private:
    // IProtocolFactory
    virtual IProtocolPtr createProtocol() override;

    std::string                         m_delimiter;
};


