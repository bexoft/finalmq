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


#include "protocolconnection/ProtocolMessage.h"


namespace finalmq {

//---------------------------------------
// ProtocolMessage
//---------------------------------------


ProtocolMessage::ProtocolMessage(int protocolId, int sizeHeader, int sizeTrailer)
    : m_sizeHeader(sizeHeader)
    , m_sizeTrailer(sizeTrailer)
    , m_protocolId(protocolId)
{
    m_itSendBufferRefsPayloadBegin = m_sendBufferRefs.end();
}

char* ProtocolMessage::addBuffer(int size)
{
    assert(!m_preparedToSend);
    assert(size > 0);

    if (m_offset > 0)
    {
        const std::string& bufLast = m_payloadBuffers.back();
        int remaining = bufLast.size() - m_offset - m_sizeTrailer;
        assert(remaining >= 0);
        if (size <= remaining)
        {
            m_sendBufferRefs.back().second += size;
            m_sendPayloadRefs.back().second += size;
            int offset = m_offset;
            m_offset += size;
            m_sizeLastBlock = size;
            m_sizeSendBufferTotal += size;
            m_sizeSendPayloadTotal += size;
            return const_cast<char*>(bufLast.data()) + offset;
        }
    }

    int sizeHeader = 0;
    if (m_payloadBuffers.empty())
    {
        sizeHeader = m_sizeHeader;
        assert(m_sizeSendBufferTotal == 0);
        m_sizeSendBufferTotal = m_sizeHeader + m_sizeTrailer;
    }
    else
    {
        // remove the trailer of the last payload
        m_sendBufferRefs.back().second -= m_sizeTrailer;
    }

    m_offset = sizeHeader + size;
    m_sizeLastBlock = size;

    m_sizeSendBufferTotal += size;
    m_sizeSendPayloadTotal += size;
    int sizeBuffer = sizeHeader + size + m_sizeTrailer;
    m_payloadBuffers.resize(m_payloadBuffers.size() + 1);
    m_payloadBuffers.back().resize(sizeBuffer);
    m_sendBufferRefs.push_back({const_cast<char*>(m_payloadBuffers.back().data()),               sizeBuffer});
    m_sendPayloadRefs.push_back({const_cast<char*>(m_payloadBuffers.back().data() + sizeHeader), size});
    return const_cast<char*>(m_payloadBuffers.back().data() + sizeHeader);
}

void ProtocolMessage::downsizeLastBuffer(int newSize)
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

    int diff = m_sizeLastBlock - newSize;
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
        m_offset = -1;
    }
}


// for send
void ProtocolMessage::addSendPayload(const std::string& payload)
{
    addSendPayload(payload.data(), payload.size());
}
void ProtocolMessage::addSendPayload(const char* payload, int size)
{
    char* payloadDest = addSendPayload(size);
    memcpy(payloadDest, payload, size);
}
char* ProtocolMessage::addSendPayload(int size)
{
    return addBuffer(size);
}

void ProtocolMessage::downsizeLastSendPayload(int newSize)
{
    return downsizeLastBuffer(newSize);
}

// for receive
BufferRef ProtocolMessage::getReceivePayload() const
{
    return {const_cast<char*>(m_receiveBuffer.data() + m_sizeHeader), m_sizeReceiveBuffer - m_sizeHeader};
}


char* ProtocolMessage::resizeReceivePayload(int size)
{
    if (size > m_sizeReceiveBuffer)
    {
        m_receiveBuffer.resize(size);
    }
    m_sizeReceiveBuffer = size;
    return const_cast<char*>(m_receiveBuffer.data());
}



// for the framework
const std::list<BufferRef>& ProtocolMessage::getAllSendBuffers() const
{
    return m_sendBufferRefs;
}
int ProtocolMessage::getTotalSendBufferSize() const
{
    return m_sizeSendBufferTotal;
}

const std::list<BufferRef>& ProtocolMessage::getAllSendPayloads() const
{
    return m_sendPayloadRefs;
}
int ProtocolMessage::getTotalSendPayloadSize() const
{
    return m_sizeSendPayloadTotal;
}



// for the protocol to add a header
void ProtocolMessage::addSendHeader(const std::string& buffer)
{
    addSendHeader(buffer.data(), buffer.size());
}
void ProtocolMessage::addSendHeader(const char* buffer, int size)
{
    char* header = addSendHeader(size);
    memcpy(header, buffer, size);
}
char* ProtocolMessage::addSendHeader(int size)
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
void ProtocolMessage::downsizeLastSendHeader(int newSize)
{
    assert(!m_preparedToSend);
    assert(!m_headerBuffers.empty());
    assert(m_sendBufferRefs.size() == m_payloadBuffers.size() + m_headerBuffers.size());
    auto itSendBufferRefs = m_itSendBufferRefsPayloadBegin;
    --itSendBufferRefs;
    assert(itSendBufferRefs != m_sendBufferRefs.end());
    int& sizeCurrent = itSendBufferRefs->second;
    m_sizeSendBufferTotal += (newSize - sizeCurrent);
    sizeCurrent = newSize;
}

// for the protocol to prepare the message for send
void ProtocolMessage::prepareMessageToSend()
{
    m_preparedToSend = true;
//    std::string buffer;
//    buffer.resize(getTotalSendBufferSize());
//    int offset = 0;
    for (auto it = m_sendBufferRefs.begin(); it != m_sendBufferRefs.end(); )
    {
//        assert(it->first != nullptr);
//        memcpy(const_cast<char*>(buffer.data())+offset, it->first, it->second);
//        offset += it->second;
//        ++it;
        if (it->second == 0)
        {
            it = m_sendBufferRefs.erase(it);
        }
        else
        {
            ++it;
        }
    }
//    m_payloadBuffers.push_back(std::move(buffer));
//    m_sendBufferRefs.clear();
//    m_sendBufferRefs.push_back(std::make_pair(const_cast<char*>(m_payloadBuffers.rbegin()->data()), m_payloadBuffers.rbegin()->size()));
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

IMessagePtr ProtocolMessage::getMessage(int protocolId) const
{
    auto it = m_messages.find(protocolId);
    if (it != m_messages.end())
    {
        return it->second;
    }
    return nullptr;
}

}   // namespace finalmq
