
#include "protocolconnection/ProtocolFixHeaderHelper.h"
#include "streamconnection/Socket.h"
#include "protocolconnection/ProtocolMessage.h"


ProtocolFixHeaderHelper::ProtocolFixHeaderHelper(int sizeHeader, std::function<int(const std::string& header)> funcGetPayloadSize)
    : m_funcGetPayloadSize(funcGetPayloadSize)
{
    m_header.resize(sizeHeader);
    assert(funcGetPayloadSize);
}



std::vector<IMessagePtr> ProtocolFixHeaderHelper::receive(const SocketPtr& socket, int bytesToRead)
{
    std::vector<IMessagePtr> messages;

    bool ok = true;
    while (bytesToRead > 0 && ok)
    {
        switch (m_state)
        {
        case State::WAITFORHEADER:
            ok = receiveHeader(socket, bytesToRead);
            break;
        case State::HEADERRECEIVED:
            {
                assert(m_funcGetPayloadSize);
                int sizePayload = m_funcGetPayloadSize(m_header);
                setPayloadSize(sizePayload);
            }
            break;
        case State::WAITFORPAYLOAD:
            ok = receivePayload(socket, bytesToRead);
            if (m_state == State::PAYLOADRECEIVED)
            {
                messages.push_back(m_message);
                clearState();
            }
            break;
        default:
            assert(false);
            break;
        }
    }

    return messages;
}


bool ProtocolFixHeaderHelper::receiveHeader(const SocketPtr& socket, int& bytesToRead)
{
    assert(m_state == State::WAITFORHEADER);
    assert(m_sizeCurrent < static_cast<int>(m_header.size()));
    bool ret = false;

    int sizeRead = m_header.size() - m_sizeCurrent;
    if (bytesToRead < sizeRead)
    {
        sizeRead = bytesToRead;
    }
    int res = socket->receive(const_cast<char*>(m_header.data() + m_sizeCurrent), sizeRead);
    if (res > 0)
    {
        int bytesReceived = res;
        assert(bytesReceived <= sizeRead);
        bytesToRead -= bytesReceived;
        assert(bytesToRead >= 0);
        m_sizeCurrent += bytesReceived;
        assert(m_sizeCurrent <= static_cast<int>(m_header.size()));
        if (m_sizeCurrent == static_cast<int>(m_header.size()))
        {
            m_sizeCurrent = 0;
            m_state = State::HEADERRECEIVED;
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
    assert(m_state == State::HEADERRECEIVED);
    m_sizePayload = sizePayload;
    m_message = std::make_shared<ProtocolMessage>(0, m_header.size());
    m_payload = m_message->resizeReceivePayload(m_header.size() + sizePayload);
    m_state = State::WAITFORPAYLOAD;
}


bool ProtocolFixHeaderHelper::receivePayload(const SocketPtr& socket, int& bytesToRead)
{
    assert(m_state == State::WAITFORPAYLOAD);
    assert(m_sizeCurrent < m_sizePayload);
    bool ret = false;

    int sizeRead = m_sizePayload - m_sizeCurrent;
    if (bytesToRead < sizeRead)
    {
        sizeRead = bytesToRead;
    }
    memcpy(m_payload, m_header.data(), m_header.size());
    int res = socket->receive(m_payload + m_header.size() + m_sizeCurrent, sizeRead);
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
            m_state = State::PAYLOADRECEIVED;
        }
        if (bytesReceived == sizeRead)
        {
            ret = true;
        }
    }
    return ret;
}


void ProtocolFixHeaderHelper::clearState()
{
    assert(m_state == State::PAYLOADRECEIVED);
    m_sizePayload = 0;
    m_message = nullptr;
    m_payload = nullptr;
    m_state = State::WAITFORHEADER;
}
