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


#include "finalmq/protocolsession/ProtocolFixHeaderHelper.h"
#include "finalmq/streamconnection/Socket.h"
#include "finalmq/protocolsession/ProtocolMessage.h"

namespace finalmq {

ProtocolFixHeaderHelper::ProtocolFixHeaderHelper(int sizeHeader, std::function<int(const std::string& header)> funcGetPayloadSize)
    : m_funcGetPayloadSize(funcGetPayloadSize)
{
    m_header.resize(sizeHeader);
    assert(funcGetPayloadSize);
}



bool ProtocolFixHeaderHelper::receive(const SocketPtr& socket, int bytesToRead, std::deque<IMessagePtr>& messages)
{
    m_messages = &messages;
    bool ok = true;
    while (bytesToRead > 0 && ok)
    {
        switch (m_state)
        {
        case State::WAITFORHEADER:
            ok = receiveHeader(socket, bytesToRead);
            break;
        case State::WAITFORPAYLOAD:
            ok = receivePayload(socket, bytesToRead);
            break;
        default:
            assert(false);
            break;
        }
    }
    m_messages = nullptr;

    return ok;
}


bool ProtocolFixHeaderHelper::receiveHeader(const SocketPtr& socket, int& bytesToRead)
{
    assert(m_state == State::WAITFORHEADER);
    assert(m_sizeCurrent < static_cast<ssize_t>(m_header.size()));
    bool ret = false;

    ssize_t sizeRead = m_header.size() - m_sizeCurrent;
    if (bytesToRead < sizeRead)
    {
        sizeRead = bytesToRead;
    }
    int res = socket->receive(const_cast<char*>(m_header.data() + m_sizeCurrent), static_cast<int>(sizeRead));
    if (res > 0)
    {
        int bytesReceived = res;
        assert(bytesReceived <= sizeRead);
        bytesToRead -= bytesReceived;
        assert(bytesToRead >= 0);
        m_sizeCurrent += bytesReceived;
        assert(m_sizeCurrent <= static_cast<ssize_t>(m_header.size()));
        if (m_sizeCurrent == static_cast<ssize_t>(m_header.size()))
        {
            m_sizeCurrent = 0;
            assert(m_funcGetPayloadSize);
            int sizePayload = m_funcGetPayloadSize(m_header);
            setPayloadSize(sizePayload);
        }
        if (bytesReceived == sizeRead)
        {
            ret = true;
        }
    }
    return ret;
}


void ProtocolFixHeaderHelper::setPayloadSize(int sizePayload)
{
    assert(m_state == State::WAITFORHEADER);
    assert(sizePayload >= 0);
    m_sizePayload = sizePayload;
    m_message = std::make_shared<ProtocolMessage>(0, m_header.size());
    m_buffer = m_message->resizeReceiveBuffer(m_header.size() + sizePayload);
    memcpy(m_buffer, m_header.data(), m_header.size());
    if (sizePayload != 0)
    {
        m_state = State::WAITFORPAYLOAD;
    }
    else
    {
        handlePayloadReceived();
    }
}


bool ProtocolFixHeaderHelper::receivePayload(const SocketPtr& socket, int& bytesToRead)
{
    assert(m_state == State::WAITFORPAYLOAD);
    assert(m_sizeCurrent < m_sizePayload);
    bool ret = false;

    ssize_t sizeRead = m_sizePayload - m_sizeCurrent;
    if (bytesToRead < sizeRead)
    {
        sizeRead = bytesToRead;
    }
    int res = socket->receive(m_buffer + m_header.size() + m_sizeCurrent, static_cast<int>(sizeRead));
    if (res > 0)
    {
        int bytesReceived = res;
        assert(bytesReceived <= sizeRead);
        bytesToRead -= bytesReceived;
        assert(bytesToRead >= 0);
        m_sizeCurrent += bytesReceived;
        assert(m_sizeCurrent <= m_sizePayload);
        if (m_sizeCurrent == m_sizePayload)
        {
            m_sizeCurrent = 0;
            handlePayloadReceived();
        }
        if (bytesReceived == sizeRead)
        {
            ret = true;
        }
    }
    return ret;
}


void ProtocolFixHeaderHelper::handlePayloadReceived()
{
    m_messages->push_back(m_message);
    clearState();
}



void ProtocolFixHeaderHelper::clearState()
{
    m_sizePayload = 0;
    m_message = nullptr;
    m_buffer = nullptr;
    m_sizeCurrent = 0;
    m_state = State::WAITFORHEADER;
}

}   // namespace finalmq
