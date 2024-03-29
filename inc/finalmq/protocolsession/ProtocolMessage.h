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

#include <unordered_map>

#include "finalmq/protocolsession/IProtocol.h"
#include "finalmq/streamconnection/IMessage.h"
#include "finalmq/variant/Variant.h"
#include "finalmq/variant/VariantValueList.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValues.h"

namespace finalmq
{
class SYMBOLEXP ProtocolMessage : public IMessage
{
public:
    static const std::string FMQ_PROTOCOLDATA;

    ProtocolMessage(std::uint32_t protocolId, ssize_t sizeHeader = 0, ssize_t sizeTrailer = 0);

private:
    virtual char* addBuffer(ssize_t size, ssize_t reserve = 0) override;
    virtual void downsizeLastBuffer(ssize_t newSize) override;
    virtual ssize_t getRemainingSize() const override;

    // metainfo
    virtual const Metainfo& getAllMetainfo() const override;
    virtual Metainfo& getAllMetainfo() override;
    virtual void addMetainfo(const std::string& key, const std::string& value) override;
    virtual void addMetainfo(std::string&& key, std::string&& value) override;
    virtual const std::string* getMetainfo(const std::string& key) const override;
    virtual std::string* getMetainfo(const std::string& key) override;

    // controlData
    virtual Variant* getControlDataIfAvailable() override;
    virtual Variant& getControlData() override;
    virtual const Variant& getControlData() const override;

    // echoData
    virtual Variant& getEchoData() override;
    virtual const Variant& getEchoData() const override;

    // for send
    virtual void addSendPayload(const std::string& payload, ssize_t reserve = 0) override;
    virtual void addSendPayload(const char* payload, ssize_t size, ssize_t reserve = 0) override;
    virtual char* addSendPayload(ssize_t size, ssize_t reserve = 0) override;
    virtual void downsizeLastSendPayload(ssize_t newSize) override;

    // for receive
    virtual BufferRef getReceiveHeader() const override;
    virtual BufferRef getReceivePayload() const override;
    virtual char* resizeReceiveBuffer(ssize_t size) override;
    virtual void setReceiveBuffer(const std::shared_ptr<std::string>& receiveBuffer, ssize_t offset, ssize_t size) override;
    virtual void setHeaderSize(ssize_t header) override;

    // for the framework
    virtual const std::list<BufferRef>& getAllSendBuffers() const override;
    virtual ssize_t getTotalSendBufferSize() const override;
    virtual const std::list<BufferRef>& getAllSendPayloads() const override;
    virtual ssize_t getTotalSendPayloadSize() const override;
    virtual void moveSendBuffers(std::list<std::string>&& payloadBuffers, const std::list<BufferRef>& payloads) override;
    virtual std::list<std::string>& getSendPayloadBuffers() override;

    // for the protocol to add a header
    virtual void addSendHeader(const std::string& header) override;
    virtual void addSendHeader(const char* header, ssize_t size) override;
    virtual char* addSendHeader(ssize_t size) override;
    virtual void downsizeLastSendHeader(ssize_t newSize) override;

    // for the protocol to prepare the message for send
    virtual void prepareMessageToSend() override;

    // for the protocol to check if which protocol created the message
    virtual std::uint32_t getProtocolId() const override;
    virtual bool wasSent() const override;

    virtual void addMessage(const IMessagePtr& msg) override;
    virtual IMessagePtr getMessage(std::uint32_t protocolId) const override;

private:
    Metainfo m_metainfo{};
    Variant m_controlData{};
    Variant m_echoData{};

    // send
    std::list<std::string> m_headerBuffers{};
    std::list<std::string> m_payloadBuffers{};
    std::list<BufferRef> m_sendBufferRefs{};
    std::list<BufferRef>::iterator m_itSendBufferRefsPayloadBegin{};
    ssize_t m_offset = -1;
    ssize_t m_sizeLastBlock = 0;
    ssize_t m_sizeSendBufferTotal = 0;
    std::list<BufferRef> m_sendPayloadRefs{};
    ssize_t m_sizeSendPayloadTotal = 0;

    // receive
    std::shared_ptr<std::string> m_receiveBuffer{};
    BufferRef m_receiveBufferRef{};

    ssize_t m_sizeHeader = 0;
    ssize_t m_sizeTrailer = 0;

    bool m_preparedToSend = false;
    const std::uint32_t m_protocolId;

    std::unordered_map<std::uint32_t, IMessagePtr> m_messages{};
};

} // namespace finalmq
