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


#include "finalmq/protocolsession/ProtocolMessage.h"


namespace finalmq {

//---------------------------------------
// ProtocolMessage
//---------------------------------------

const std::string ProtocolMessage::FMQ_PROTOCOLDATA = "fmq_protocoldata";


ProtocolMessage::ProtocolMessage(std::uint32_t protocolId, ssize_t sizeHeader, ssize_t sizeTrailer)
    : m_sizeHeader(sizeHeader)
    , m_sizeTrailer(sizeTrailer)
    , m_protocolId(protocolId)
{
    m_itSendBufferRefsPayloadBegin = m_sendBufferRefs.end();
}

char* ProtocolMessage::addBuffer(ssize_t size, ssize_t reserve)
{
    assert(!m_preparedToSend);
    if (size < 0)
    {
        return nullptr;
    }

    if (m_offset != -1)
    {
        assert(!m_payloadBuffers.empty());
        const std::string& bufLast = m_payloadBuffers.back();
        ssize_t remaining = bufLast.size() - m_offset - m_sizeTrailer;
        assert(remaining >= 0);
        if (size <= remaining)
        {
            m_sendBufferRefs.back().second += size;
            m_sendPayloadRefs.back().second += size;
            ssize_t offset = m_offset;
            m_offset += size;
            m_sizeLastBlock = size;
            m_sizeSendBufferTotal += size;
            m_sizeSendPayloadTotal += size;
            return const_cast<char*>(bufLast.data()) + offset;
        }
    }

    if (reserve < size)
    {
        reserve = size;
    }

    ssize_t sizeHeader = 0;
    if (m_payloadBuffers.empty())
    {
        assert(m_sizeSendPayloadTotal == 0);
        sizeHeader = m_sizeHeader;
        m_sizeSendBufferTotal += m_sizeHeader + m_sizeTrailer;
    }
    else
    {
        // remove the trailer of the last payload
        BufferRef& lastRef = m_sendBufferRefs.back();
        assert(lastRef.second >= m_sizeTrailer);
        lastRef.second -= m_sizeTrailer;
        if (lastRef.second == 0)
        {
            m_sendBufferRefs.pop_back();
            m_sendPayloadRefs.pop_back();
            m_payloadBuffers.pop_back();
        }
    }

    m_offset = sizeHeader + reserve;
    m_sizeLastBlock = reserve;

    m_sizeSendBufferTotal += reserve;
    m_sizeSendPayloadTotal += reserve;
    ssize_t sizeBuffer = sizeHeader + reserve + m_sizeTrailer;
    m_payloadBuffers.resize(m_payloadBuffers.size() + 1);
    m_payloadBuffers.back().resize(sizeBuffer);
    m_sendBufferRefs.push_back({const_cast<char*>(m_payloadBuffers.back().data()), sizeBuffer});
    m_sendPayloadRefs.push_back({const_cast<char*>(m_payloadBuffers.back().data() + sizeHeader), reserve});
    if (size < reserve)
    {
        downsizeLastBuffer(size);
    }
    return const_cast<char*>(m_payloadBuffers.back().data() + sizeHeader);
}

void ProtocolMessage::downsizeLastBuffer(ssize_t newSize)
{
    assert(!m_preparedToSend);

    if (m_payloadBuffers.empty() && newSize == 0)
    {
        return;
    }

    assert(newSize <= m_sizeLastBlock);
    assert(m_offset != -1 || newSize == 0);
    assert(!m_payloadBuffers.empty());
    assert(m_sendBufferRefs.size() == m_payloadBuffers.size() + m_headerBuffers.size());

    ssize_t diff = m_sizeLastBlock - newSize;
    assert(diff >= 0);

    if (diff == 0)
    {
        return;
    }

    m_sendBufferRefs.back().second -= diff;
    m_sizeSendBufferTotal -= diff;
    assert(m_sendBufferRefs.back().second >= 0);
    assert(m_sizeSendBufferTotal >= 0);
    m_sizeSendPayloadTotal -= diff;
    m_offset -= diff;
    m_sizeLastBlock = newSize;
    assert(m_sizeSendPayloadTotal >= 0);
    assert(m_offset >= 0);

    if (m_sendBufferRefs.back().second > 0)
    {
        m_sendPayloadRefs.back().second -= diff;
        assert(m_sendPayloadRefs.back().second >= 0);
    }
    else
    {
        m_sendBufferRefs.pop_back();
        m_sendPayloadRefs.pop_back();
        m_payloadBuffers.pop_back();
        m_offset = -1;
    }
}

ssize_t ProtocolMessage::getRemainingSize() const
{
    if (m_offset != -1)
    {
        assert(!m_payloadBuffers.empty());
        const std::string& bufLast = m_payloadBuffers.back();
        ssize_t remaining = bufLast.size() - m_offset - m_sizeTrailer;
        assert(remaining >= 0);
        return remaining;
    }

    return 0;
}


// metadata
const ProtocolMessage::Metainfo& ProtocolMessage::getAllMetainfo() const
{
    return m_metainfo;
}

IMessage::Metainfo& ProtocolMessage::getAllMetainfo()
{
    return m_metainfo;
}

void ProtocolMessage::addMetainfo(const std::string& key, const std::string& value)
{
    m_metainfo[key] = value;
}

void ProtocolMessage::addMetainfo(std::string&& key, std::string&& value)
{
    m_metainfo[std::move(key)] = std::move(value);
}

const std::string* ProtocolMessage::getMetainfo(const std::string& key) const
{
    return const_cast<ProtocolMessage*>(this)->getMetainfo(key);
}


std::string* ProtocolMessage::getMetainfo(const std::string& key)
{
    auto it = m_metainfo.find(key);
    if (it != m_metainfo.end())
    {
        return &it->second;
    }
    return nullptr;
}


// controlData

Variant* ProtocolMessage::getControlDataIfAvailable()
{
    if (m_controlData.getType() != VARTYPE_STRUCT)
    {
        return nullptr;
    }
    return &m_controlData;
}


Variant& ProtocolMessage::getControlData()
{
    if (m_controlData.getType() != VARTYPE_STRUCT)
    {
        m_controlData = VariantStruct();
    }
    return m_controlData;
}

const Variant& ProtocolMessage::getControlData() const
{
    if (m_controlData.getType() != VARTYPE_STRUCT)
    {
        const_cast<Variant&>(m_controlData) = VariantStruct();
    }
    return m_controlData;
}


// echoData
Variant& ProtocolMessage::getEchoData()
{
    if (m_echoData.getType() != VARTYPE_STRUCT)
    {
        m_echoData = VariantStruct();
    }
    return m_echoData;
}

const Variant& ProtocolMessage::getEchoData() const
{
    if (m_echoData.getType() != VARTYPE_STRUCT)
    {
        const_cast<Variant&>(m_echoData) = VariantStruct();
    }
    return m_echoData;
}



// for send
void ProtocolMessage::addSendPayload(const std::string& payload, ssize_t reserve)
{
    addSendPayload(payload.data(), payload.size(), reserve);
}
void ProtocolMessage::addSendPayload(const char* payload, ssize_t size, ssize_t reserve)
{
    char* payloadDest = addSendPayload(size, reserve);
    memcpy(payloadDest, payload, size);
}
char* ProtocolMessage::addSendPayload(ssize_t size, ssize_t reserve)
{
    return addBuffer(size, reserve);
}

void ProtocolMessage::downsizeLastSendPayload(ssize_t newSize)
{
    downsizeLastBuffer(newSize);
}

// for receive
BufferRef ProtocolMessage::getReceiveHeader() const
{
    return { m_receiveBufferRef.first, m_sizeHeader };
}

BufferRef ProtocolMessage::getReceivePayload() const
{
    return { const_cast<char*>(m_receiveBufferRef.first + m_sizeHeader), m_receiveBufferRef.second - m_sizeHeader };
}


char* ProtocolMessage::resizeReceiveBuffer(ssize_t size)
{
    if (size < 0)
    {
        size = 0;
    }
    if (m_receiveBuffer == nullptr)
    {
        m_receiveBuffer = std::make_shared<std::string>();
    }
    if (static_cast<size_t>(size) > m_receiveBuffer->size())
    {
        m_receiveBuffer->resize(size);
        m_receiveBufferRef.first = const_cast<char*>(m_receiveBuffer->data());
    }
    m_receiveBufferRef.second = size;
    return const_cast<char*>(m_receiveBufferRef.first);
}

void ProtocolMessage::setReceiveBuffer(const std::shared_ptr<std::string>& receiveBuffer, ssize_t offset, ssize_t size)
{
    assert(receiveBuffer != nullptr);
    assert(offset + size <= static_cast<ssize_t>(receiveBuffer->size()));
    m_receiveBuffer = receiveBuffer;
    m_receiveBufferRef.first = const_cast<char*>(m_receiveBuffer->data()) + offset;
    m_receiveBufferRef.second = size;
}

void ProtocolMessage::setHeaderSize(ssize_t sizeHeader)
{
    m_sizeHeader = sizeHeader;
}


// for the framework
const std::list<BufferRef>& ProtocolMessage::getAllSendBuffers() const
{
    return m_sendBufferRefs;
}
ssize_t ProtocolMessage::getTotalSendBufferSize() const
{
    return m_sizeSendBufferTotal;
}

const std::list<BufferRef>& ProtocolMessage::getAllSendPayloads() const
{
    return m_sendPayloadRefs;
}
ssize_t ProtocolMessage::getTotalSendPayloadSize() const
{
    return m_sizeSendPayloadTotal;
}

void ProtocolMessage::moveSendBuffers(std::list<std::string>&& payloadBuffers, const std::list<BufferRef>& payloads)
{
    assert(payloadBuffers.size() == payloads.size());
    auto itSize = payloads.begin();
    for (auto it = payloadBuffers.begin(); it != payloadBuffers.end(); ++it, ++itSize)
    {
        m_payloadBuffers.emplace_back(std::move(*it));
        size_t size = itSize->second;
        m_sendBufferRefs.emplace_back(const_cast<char*>(m_payloadBuffers.back().data()), size);
        m_sendPayloadRefs.emplace_back(const_cast<char*>(m_payloadBuffers.back().data()), size);
        m_offset = size;
        m_sizeLastBlock = size;
        m_sizeSendBufferTotal += size;
        m_sizeSendPayloadTotal += size;
    }
}

std::list<std::string>& ProtocolMessage::getSendPayloadBuffers()
{
    return m_payloadBuffers;
}


// for the protocol to add a header
void ProtocolMessage::addSendHeader(const std::string& buffer)
{
    addSendHeader(buffer.data(), buffer.size());
}
void ProtocolMessage::addSendHeader(const char* buffer, ssize_t size)
{
    char* header = addSendHeader(size);
    memcpy(header, buffer, size);
}
char* ProtocolMessage::addSendHeader(ssize_t size)
{
    assert(!m_preparedToSend);
    if (m_headerBuffers.empty())
    {
        m_itSendBufferRefsPayloadBegin = m_sendBufferRefs.begin();
    }
    m_headerBuffers.resize(m_headerBuffers.size() + 1);
    m_headerBuffers.back().resize(size);
    m_sendBufferRefs.insert(m_itSendBufferRefsPayloadBegin, {const_cast<char*>(m_headerBuffers.back().data()), m_headerBuffers.back().size()});
    m_sizeSendBufferTotal += size;
    return const_cast<char*>(m_headerBuffers.back().data());
}
void ProtocolMessage::downsizeLastSendHeader(ssize_t newSize)
{
    assert(!m_preparedToSend);
    assert(!m_headerBuffers.empty());
    assert(m_sendBufferRefs.size() == m_payloadBuffers.size() + m_headerBuffers.size());
    auto itSendBufferRefs = m_itSendBufferRefsPayloadBegin;
    assert(itSendBufferRefs != m_sendBufferRefs.begin());
    --itSendBufferRefs;
    ssize_t& sizeCurrent = itSendBufferRefs->second;
    assert(newSize <= sizeCurrent);
    m_sizeSendBufferTotal += (newSize - sizeCurrent);
    sizeCurrent = newSize;
    if (newSize == 0)
    {
        m_sendBufferRefs.erase(itSendBufferRefs);
        m_headerBuffers.pop_back();
    }
}

// for the protocol to prepare the message for send
void ProtocolMessage::prepareMessageToSend()
{
    m_preparedToSend = true;
    for (auto it = m_sendBufferRefs.begin(); it != m_sendBufferRefs.end(); )
    {
        if (it->second == 0)
        {
            it = m_sendBufferRefs.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// for the protocol to check if which protocol created the message
std::uint32_t ProtocolMessage::getProtocolId() const
{
    return m_protocolId;
}

bool ProtocolMessage::wasSent() const
{
    return m_preparedToSend;
}

void ProtocolMessage::addMessage(const IMessagePtr& msg)
{
    m_messages[msg->getProtocolId()] = msg;
}

IMessagePtr ProtocolMessage::getMessage(std::uint32_t protocolId) const
{
    auto it = m_messages.find(protocolId);
    if (it != m_messages.end())
    {
        return it->second;
    }
    return nullptr;
}

}   // namespace finalmq
