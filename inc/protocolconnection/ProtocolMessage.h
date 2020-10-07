#pragma once

#include "streamconnection/IMessage.h"
#include "protocolconnection/IProtocol.h"

#include <unordered_map>


class ProtocolMessage : public IMessage
{
public:
    ProtocolMessage(int protocolId, int sizeHeader = 0, int sizeTrailer = 0);

private:
    virtual char* addBuffer(int size) override;
    virtual void downsizeLastBuffer(int newSize) override;

    // for send
    virtual void addSendPayload(const std::string& payload) override;
    virtual void addSendPayload(const char* payload, int size) override;
    virtual char* addSendPayload(int size) override;
    virtual void downsizeLastSendPayload(int newSize) override;

    // for receive
    virtual BufferRef getReceivePayload() override;
    virtual char* resizeReceivePayload(int size) override;

    // for the framework
    virtual const std::list<BufferRef>& getAllSendBuffers() const override;
    virtual int getTotalSendBufferSize() const override;
    virtual const std::list<BufferRef>& getAllSendPayloads() const override;
    virtual int getTotalSendPayloadSize() const override;

    // for the protocol to add a header
    virtual void addSendHeader(const std::string& header) override;
    virtual void addSendHeader(const char* header, int size) override;
    virtual char* addSendHeader(int size) override;
    virtual void downsizeLastSendHeader(int newSize) override;

    // for the protocol to prepare the message for send
    virtual void prepareMessageToSend() override;

    // for the protocol to check if which protocol created the message
    virtual int getProtocolId() const;
    virtual bool wasSent() const override;

    virtual void addMessage(const IMessagePtr& msg) override;
    virtual IMessagePtr getMessage(int protocolId) const override;

private:
    // send
    std::list<std::string>      m_headerBuffers;
    std::list<std::string>      m_payloadBuffers;
    std::list<BufferRef>        m_sendBufferRefs;
    std::list<BufferRef>::iterator m_itSendBufferRefsPayloadBegin;
    int                         m_sizeSendBufferTotal = 0;
    std::list<BufferRef>        m_sendPayloadRefs;
    int                         m_sizeSendPayloadTotal = 0;

    // receive
    std::string                 m_receiveBuffer;
    int                         m_sizeReceiveBuffer = 0;

    int                         m_sizeHeader = 0;
    int                         m_sizeTrailer = 0;

    bool                        m_preparedToSend = false;
    const int                   m_protocolId;

    std::unordered_map<int, IMessagePtr> m_messages;
};

