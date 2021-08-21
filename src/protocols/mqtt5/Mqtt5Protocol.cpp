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


#include "finalmq/protocols/mqtt5/Mqtt5Protocol.h"
#include "finalmq/streamconnection/Socket.h"
#include "finalmq/protocolsession/ProtocolMessage.h"

#include "finalmq/protocols/mqtt5/Mqtt5Serialization.h"



namespace finalmq {

static const ssize_t HEADERSIZE = 1;


#define HEADER_Command(header)      static_cast<Mqtt5Command>(((header) >> 4) & 0x0f)
#define HEADER_Dup(header)          (((header) >> 3) >> 0x01)
#define HEADER_QoS(header)          (((header) >> 1) >> 0x03)
#define HEADER_Retain(header)       (((header) >> 0) >> 0x01)

Mqtt5Protocol::Mqtt5Protocol()
{
}


bool Mqtt5Protocol::receive(const SocketPtr& socket, int bytesToRead)
{
    bool ok = true;
    while ((bytesToRead > 0) && ok)
    {
        switch (m_state)
        {
        case State::WAITFORHEADER:
            ok = receiveHeader(socket, bytesToRead);
            break;
        case State::WAITFORLENGTH:
            ok = receiveRemainingSize(socket, bytesToRead);
            break;
        case State::WAITFORPAYLOAD:
            ok = receivePayload(socket, bytesToRead);
            break;
        default:
            assert(false);
            break;
        }
    }
    return ok;
}


bool Mqtt5Protocol::receiveHeader(const SocketPtr& socket, int& bytesToRead)
{
    assert(bytesToRead >= 1);
    assert(m_state == State::WAITFORHEADER);
    bool ret = false;

    int res = socket->receive(reinterpret_cast<char*>(&m_header), HEADERSIZE);
    if (res == HEADERSIZE)
    {
        bytesToRead -= HEADERSIZE;
        ret = true;
        m_remainingSize = 0;
        m_remainingSizeShift = 0;
        m_state = State::WAITFORLENGTH;
    }
    return ret;
}

bool Mqtt5Protocol::receiveRemainingSize(const SocketPtr& socket, int& bytesToRead)
{
    assert(bytesToRead >= 1);
    assert(m_state == State::WAITFORHEADER);
    bool ok = true;
    bool done = false;
    while (bytesToRead > 1 && !done && ok)
    {
        ok = false;
        if (m_remainingSizeShift <= 21)
        {
            char data;
            int res = socket->receive(&data, 1);
            if (res == 1)
            {
                ok = true;
                bytesToRead -= 1;
                m_remainingSize |= (data & 0x7f) << m_remainingSizeShift;
                if ((data & 0x80) == 0)
                {
                    done = true;
                    setPayloadSize();
                    if (m_state == State::MESSAGECOMPLETE)
                    {
                        ok = processPayload();
                    }
                }
                m_remainingSizeShift += 7;
            }
        }
    }

    return ok;
}


void Mqtt5Protocol::setPayloadSize()
{
    assert(m_state == State::WAITFORLENGTH);
    assert(m_remainingSize >= 0);
    if (HEADER_Command(m_header) == Mqtt5Command::COMMAND_PUBLISH)
    {
        m_message = std::make_shared<ProtocolMessage>(0, HEADERSIZE);
        m_buffer = m_message->resizeReceiveBuffer(HEADERSIZE + m_remainingSize);
    }
    else
    {
        m_messageBuffer.resize(HEADERSIZE + m_remainingSize);
        m_buffer = m_messageBuffer.data();
    }
    m_buffer[0] = m_header;
    if (m_remainingSize != 0)
    {
        m_state = State::WAITFORPAYLOAD;
    }
    else
    {
        m_state = State::MESSAGECOMPLETE;
    }
    m_sizeCurrent = 0;
}


bool Mqtt5Protocol::receivePayload(const SocketPtr& socket, int& bytesToRead)
{
    assert(m_state == State::WAITFORPAYLOAD);
    assert(m_sizeCurrent < m_remainingSize);
    bool ok = false;

    ssize_t sizeRead = m_remainingSize - m_sizeCurrent;
    if (bytesToRead < sizeRead)
    {
        sizeRead = bytesToRead;
    }
    int res = socket->receive(m_buffer + HEADERSIZE + m_sizeCurrent, static_cast<int>(sizeRead));
    if (res > 0)
    {
        int bytesReceived = res;
        assert(bytesReceived <= sizeRead);
        bytesToRead -= bytesReceived;
        assert(bytesToRead >= 0);
        m_sizeCurrent += bytesReceived;
        assert(m_sizeCurrent <= m_remainingSize);
        if (bytesReceived == sizeRead)
        {
            ok = true;
        }
        if (ok && m_sizeCurrent == m_remainingSize)
        {
            m_state = State::MESSAGECOMPLETE;
            m_sizeCurrent = 0;
            if (ok)
            {
                ok = processPayload();
            }
        }
    }
    return ok;
}






bool Mqtt5Protocol::processPayload()
{
    assert(m_state == State::MESSAGECOMPLETE);

    Mqtt5Serialization serialization(m_buffer, m_remainingSize, 1);

    auto callback = m_callback.lock();

    bool ok = false;
    Mqtt5Command command = HEADER_Command(m_header);
    switch (command)
    {
    case Mqtt5Command::COMMAND_CONNECT:
        {
            Mqtt5ConnectData data;
            ok = serialization.deserializeConnect(data);
            if (callback && ok)
            {
                callback->receivedConnect(data);
            }
        }
        break;
    case Mqtt5Command::COMMAND_CONNACK:
        {
            Mqtt5ConnAckData data;
            ok = serialization.deserializeConnAck(data);
            if (callback && ok)
            {
                callback->receivedConnAck(data);
            }
    }
        break;
    case Mqtt5Command::COMMAND_PUBLISH:
        {
            Mqtt5PublishData data;
            ok = serialization.deserializePublish(data);
            if (callback && ok)
            {
                int indexRead = serialization.getReadIndex();
                m_message->setHeaderSize(indexRead);
                callback->receivedPublish(data, m_message);
            }
        }
        break;
    case Mqtt5Command::COMMAND_PUBACK:
        {
            Mqtt5PubAckData data;
            ok = serialization.deserializePubAck(data, Mqtt5Command::COMMAND_PUBACK);
        }
        break;
    case Mqtt5Command::COMMAND_PUBREC:
        {
            Mqtt5PubAckData data;
            ok = serialization.deserializePubAck(data, Mqtt5Command::COMMAND_PUBREC);
        }
        break;
    case Mqtt5Command::COMMAND_PUBREL:
        {
            Mqtt5PubAckData data;
            ok = serialization.deserializePubAck(data, Mqtt5Command::COMMAND_PUBREL);
        }
        break;
    case Mqtt5Command::COMMAND_PUBCOMP:
        {
            Mqtt5PubAckData data;
            ok = serialization.deserializePubAck(data, Mqtt5Command::COMMAND_PUBCOMP);
        }
        break;
    case Mqtt5Command::COMMAND_SUBSCRIBE:
        {
            Mqtt5SubscribeData data;
            ok = serialization.deserializeSubscribe(data);
            if (callback && ok)
            {
                callback->receivedSubscribe(data);
            }
        }
        break;
    case Mqtt5Command::COMMAND_SUBACK:
        {
            Mqtt5SubAckData data;
            ok = serialization.deserializeSubAck(data);
            if (callback && ok)
            {
                callback->receivedSubAck(data);
            }
    }
        break;
    case Mqtt5Command::COMMAND_UNSUBSCRIBE:
        {
            Mqtt5UnsubscribeData data;
            ok = serialization.deserializeUnsubscribe(data);
            if (callback && ok)
            {
                callback->receivedUnsubscribe(data);
            }
    }
        break;
    case Mqtt5Command::COMMAND_UNSUBACK:
        {
            Mqtt5SubAckData data;
            ok = serialization.deserializeSubAck(data);
            if (callback && ok)
            {
                callback->receivedUnsubAck(data);
            }
        }
        break;
    case Mqtt5Command::COMMAND_PINGREQ:
        if (callback)
        {
            callback->receivedPingReq();
        }
        break;
    case Mqtt5Command::COMMAND_PINGRESP:
        if (callback)
        {
            callback->receivedPingResp();
        }
        break;
    case Mqtt5Command::COMMAND_DISCONNECT:
        {
            Mqtt5DisconnectData data;
            ok = serialization.deserializeDisconnect(data);
            if (callback && ok)
            {
                callback->receivedDisconnect(data);
            }
        }
        break;
    case Mqtt5Command::COMMAND_AUTH:
        {
            Mqtt5AuthData data;
            ok = serialization.deserializeAuth(data);
            if (callback && ok)
            {
                callback->receivedAuth(data);
            }
        }
        break;
    default:
        break;
    }

    if (ok)
    {
        clearState();
    }

    return ok;
}



void Mqtt5Protocol::clearState()
{
    m_sizeCurrent = 0;
    m_sizePayload = 0;
    m_message = nullptr;
    m_messageBuffer.clear();
    m_buffer = nullptr;
    m_state = State::WAITFORHEADER;
}



// IMqtt5Protocol
void Mqtt5Protocol::setCallback(hybrid_ptr<IMqtt5ProtocolCallback> callback)
{
    m_callback = callback;
}

void Mqtt5Protocol::connected(const IStreamConnectionPtr& connection)
{
    m_connection = connection;
}

void Mqtt5Protocol::disconnected()
{
    m_connection = nullptr;
}

bool Mqtt5Protocol::sendConnect(const Mqtt5ConnectData& /*data*/)
{
    return false;
}

bool Mqtt5Protocol::sendConnAck(const Mqtt5ConnAckData& /*data*/)
{
    return false;
}

bool Mqtt5Protocol::sendPublish(const Mqtt5PublishData& /*data*/)
{
    return false;
}

bool Mqtt5Protocol::sendSubscribe(const Mqtt5SubscribeData& /*data*/)
{
    return false;
}

bool Mqtt5Protocol::sendSubAck(const Mqtt5SubAckData& /*data*/)
{
    return false;
}

bool Mqtt5Protocol::sendUnsubscribe(const Mqtt5UnsubscribeData& /*data*/)
{
    return false;
}

bool Mqtt5Protocol::sendUnsubAck(const Mqtt5SubAckData& /*data*/)
{
    return false;
}

bool Mqtt5Protocol::sendPingReq()
{
    return false;
}

bool Mqtt5Protocol::sendPingResp()
{
    return false;
}

bool Mqtt5Protocol::sendDisconnect(const Mqtt5DisconnectData& /*data*/)
{
    return false;
}

bool Mqtt5Protocol::sendAuth(const Mqtt5AuthData& /*data*/)
{
    return false;
}





}   // namespace finalmq
