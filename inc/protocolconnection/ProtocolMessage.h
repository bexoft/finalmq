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

#include "streamconnection/IMessage.h"
#include "protocolconnection/IProtocol.h"

#include <unordered_map>

namespace finalmq {

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
    virtual BufferRef getReceivePayload() const override;
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
    virtual std::uint32_t getProtocolId() const;
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

}   // namespace finalmq
