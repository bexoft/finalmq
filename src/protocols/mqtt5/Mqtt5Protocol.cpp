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


#define HEADER_Command(header)      (((header) & 0xf0) >> 4)
#define HEADER_Dup(header)          (((header) & 0x08) >> 3)
#define HEADER_QoS(header)          (((header) & 0x06) >> 1)
#define HEADER_Retain(header)       (((header) & 0x01) >> 0)


#define COMMAND_CONNECT         1       // Server <-  Client, Connection request
#define COMMAND_CONNACK         2       // Server  -> Client, Connect acknowledgment
#define COMMAND_PUBLISH         3       // Server <-> Client, Publish message
#define COMMAND_PUBACK          4       // Server <-> Client, Publish acknowledgment(QoS 1)
#define COMMAND_PUBREC          5       // Server <-> Client, Publish received(QoS 2 delivery part 1)
#define COMMAND_PUBREL          6       // Server <-> Client, Publish release(QoS 2 delivery part 2)
#define COMMAND_PUBCOMP         7       // Server <-> Client, Publish complete(QoS 2 delivery part 3)
#define COMMAND_SUBSCRIBE       8       // Server <-  Client, Subscribe request
#define COMMAND_SUBACK          9       // Server  -> Client, Subscribe acknowledgment
#define COMMAND_UNSUBSCRIBE     10      // Server <-  Client, Unsubscribe request
#define COMMAND_UNSUBACK        11      // Server  -> Client, Unsubscribe acknowledgment
#define COMMAND_PING            12      // Server <-  Client, PING request
#define COMMAND_PINGRESP        13      // Server  -> Client, PING response
#define COMMAND_DISCONNECT      14      // Server <-> Client, Disconnect notification
#define COMMAND_AUTH            15      // Server <-> Client, Authentication exchange


Mqtt5Protocol::Mqtt5Protocol()
{
}


bool Mqtt5Protocol::receive(const SocketPtr& socket, int bytesToRead, std::deque<IMessagePtr>& messages)
{
    m_messages = &messages;
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
    m_messages = nullptr;
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
    m_message = std::make_shared<ProtocolMessage>(0, HEADERSIZE);
    m_buffer = m_message->resizeReceiveBuffer(HEADERSIZE + m_remainingSize);
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

    Mqtt5Serialization serialization(m_remainingSize, 1, m_header, m_buffer);

    bool ok = false;
    int command = HEADER_Command(m_header);
    switch (command)
    {
    case COMMAND_CONNECT:
        {
            Mqtt5ConnectData data;
            ok = serialization.deserializeConnect(data);
        }
        break;
    case COMMAND_CONNACK:
        {
            Mqtt5ConnAckData data;
            ok = serialization.deserializeConnAck(data);
        }
        break;
    case COMMAND_PUBLISH:
        {
            Mqtt5PublishData data;
            ok = serialization.deserializePublish(data);
            int indexRead = serialization.getReadIndex();
            m_message->setHeaderSize(indexRead);
            m_messages->push_back(m_message);
    }
        break;
    case COMMAND_PUBACK:
        {
            Mqtt5PubAckData data;
            ok = serialization.deserializePubAck(data);
        }
        break;
    case COMMAND_PUBREC:
        {
            Mqtt5PubAckData data;
            ok = serialization.deserializePubAck(data);
        }
        break;
    case COMMAND_PUBREL:
        {
            Mqtt5PubAckData data;
            ok = serialization.deserializePubAck(data);
        }
        break;
    case COMMAND_PUBCOMP:
        {
            Mqtt5PubAckData data;
            ok = serialization.deserializePubAck(data);
        }
        break;
    case COMMAND_SUBSCRIBE:
        {
            Mqtt5SubscribeData data;
            ok = serialization.deserializeSubscribe(data);
        }
        break;
    case COMMAND_SUBACK:
        {
            Mqtt5SubAckData data;
            ok = serialization.deserializeSubAck(data);
        }
        break;
    case COMMAND_UNSUBSCRIBE:
        {
            Mqtt5UnsubscribeData data;
            ok = serialization.deserializeUnsubscribe(data);
        }
        break;
    case COMMAND_UNSUBACK:
        {
            Mqtt5SubAckData data;
            ok = serialization.deserializeSubAck(data);
        }
        break;
    case COMMAND_PING:
        break;
    case COMMAND_PINGRESP:
        break;
    case COMMAND_DISCONNECT:
        {
            Mqtt5DisconnectData data;
            ok = serialization.deserializeDisconnect(data);
        }
        break;
    case COMMAND_AUTH:
        {
            Mqtt5AuthData data;
            ok = serialization.deserializeAuth(data);
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
    m_buffer = nullptr;
    m_state = State::WAITFORHEADER;
}


}   // namespace finalmq
