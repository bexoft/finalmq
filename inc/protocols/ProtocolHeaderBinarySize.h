#pragma once

#include "streamconnection/IMessage.h"
#include "protocolconnection/IProtocol.h"
#include "protocolconnection/ProtocolFixHeaderHelper.h"






class ProtocolHeaderBinarySize : public IProtocol
{
public:
    ProtocolHeaderBinarySize();

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

    std::weak_ptr<IProtocolCallback>    m_callback;
    ProtocolFixHeaderHelper             m_headerHelper;

    const int                           m_protocolId = 0x5284d303;
};


class ProtocolHeaderBinarySizeFactory : public IProtocolFactory
{
public:

private:
    // IProtocolFactory
    virtual IProtocolPtr createProtocol() override;
};

