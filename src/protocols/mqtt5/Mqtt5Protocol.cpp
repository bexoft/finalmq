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
#include "finalmq/protocols/mqtt5/Mqtt5Properties.h"



namespace finalmq {

static const ssize_t HEADERSIZE = 1;


#define HEADER_Command(header)      static_cast<Mqtt5Command>(((header) >> 4) & 0x0f)
#define HEADER_Dup(header)          (((header) >> 3) >> 0x01)
#define HEADER_QoS(header)          (((header) >> 1) >> 0x03)
#define HEADER_Retain(header)       (((header) >> 0) >> 0x01)
#define HEADER_SetDup(value)        (((value) & 0x01) << 3)


Mqtt5Protocol::Mqtt5Protocol()
{
    // add the dummy packetId with index 0
    m_messageIdsAllocated.push_back({});
}


bool Mqtt5Protocol::receive(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead)
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
            ok = receiveRemainingSize(connection, socket, bytesToRead);
            break;
        case State::WAITFORPAYLOAD:
            ok = receivePayload(connection, socket, bytesToRead);
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

    int res = socket->receive(&m_header, HEADERSIZE);
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

bool Mqtt5Protocol::receiveRemainingSize(const IStreamConnectionPtr& connection, const SocketPtr& socket, int& bytesToRead)
{
    assert(bytesToRead >= 1);
    assert(m_state == State::WAITFORLENGTH);
    bool ok = true;
    bool done = false;
    while ((bytesToRead > 0) && !done && ok)
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
                        ok = processPayload(connection);
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


bool Mqtt5Protocol::receivePayload(const IStreamConnectionPtr& connection, const SocketPtr& socket, int& bytesToRead)
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
        if (ok && (m_sizeCurrent == m_remainingSize))
        {
            m_state = State::MESSAGECOMPLETE;
            m_sizeCurrent = 0;
            ok = processPayload(connection);
        }
    }
    return ok;
}



bool Mqtt5Protocol::handleAck(const IStreamConnectionPtr& connection, unsigned int command, unsigned int packetId, unsigned int reasoncode)
{
    bool ok = true;
    std::unique_lock<std::mutex> lock(m_mutex);
    if ((packetId != 0) && (packetId < m_messageIdsAllocated.size()))
    {
        auto cmd = static_cast<Mqtt5Command>(command);
        MessageStatus& status = m_messageIdsAllocated[packetId];
        if (((status.status == MessageStatus::SENDSTAT_WAITPUBACK)   && (cmd == Mqtt5Command::COMMAND_PUBACK))  ||
            ((status.status == MessageStatus::SENDSTAT_WAITPUBREC)   && (cmd == Mqtt5Command::COMMAND_PUBREC))  ||
            ((status.status == MessageStatus::SENDSTAT_WAITPUBCOMP)  && (cmd == Mqtt5Command::COMMAND_PUBCOMP)) ||
            ((status.status == MessageStatus::SENDSTAT_WAITSUBACK)   && (cmd == Mqtt5Command::COMMAND_SUBACK))  ||
            ((status.status == MessageStatus::SENDSTAT_WAITUNSUBACK) && (cmd == Mqtt5Command::COMMAND_UNSUBACK)))
        {
            assert(status.iterator != m_messagesWaitAck.end());
            m_messagesWaitAck.erase(status.iterator);
            status.iterator = m_messagesWaitAck.end();
            if ((cmd != Mqtt5Command::COMMAND_PUBREC) || (reasoncode >= 128))
            {
                status.status = MessageStatus::SENDSTAT_NONE;
                // if last entry is freed ...
                if (packetId == m_messageIdsAllocated.size() - 1)
                {
                    // ... remove last unused entry from the end of the allocation list
                    m_messageIdsAllocated.pop_back();
                }
                else
                {
                    m_messageIdsFree.push_back(packetId);
                }
                sendPendingMessages(connection);
            }
            else
            {
                status.status = MessageStatus::SENDSTAT_WAITPUBCOMP;
                Mqtt5PubAckData data;
                data.packetId = packetId;
                data.reasoncode = 0;
                IMessagePtr message = std::make_shared<ProtocolMessage>(0);
                m_messagesWaitAck.push_back(message);
                status.iterator = --m_messagesWaitAck.end();
                lock.unlock();
                sendPubRel(connection, data, message);
            }
        }
        else
        {
            ok = false;
        }
    }
    return ok;
}




bool Mqtt5Protocol::processPayload(const IStreamConnectionPtr& connection)
{
    assert(m_state == State::MESSAGECOMPLETE);

    Mqtt5Serialization serialization(m_buffer, HEADERSIZE + m_remainingSize, 1);

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
            if (ok && data.reasoncode < 0x80)
            {
                auto it = data.properties.find(Mqtt5PropertyId::ReceiveMaximum);

                std::unique_lock<std::mutex> lock(m_mutex);
                if (it != data.properties.end())
                {
                    m_sendMax = static_cast<std::uint32_t>(it->second);
                }
                else
                {
                    m_sendMax = 65535;
                }
                resendMessages(connection);
                sendPendingMessages(connection);
                m_connecting = false;
                lock.unlock();
            }
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
            if (ok)
            {
                bool execute = true;
                if (data.qos == 1)
                {
                    Mqtt5PubAckData dataAck;
                    dataAck.packetId = data.packetId;
                    dataAck.reasoncode = 0;
                    sendPubAck(connection, dataAck);
                }
                else if (data.qos == 2)
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    if (data.dup && (m_setExactlyOne.find(data.packetId) != m_setExactlyOne.end()))
                    {
                        execute = false;
                    }
                    else
                    {
                        m_setExactlyOne.insert(data.packetId);
                    }
                    lock.unlock();
                    Mqtt5PubAckData dataAck;
                    dataAck.packetId = data.packetId;
                    dataAck.reasoncode = 0;
                    sendPubRec(connection, dataAck);
                }
                if (callback && execute)
                {
                    int indexRead = serialization.getReadIndex();
                    m_message->setHeaderSize(indexRead);
                    callback->receivedPublish(std::move(data), m_message);
                }
            }
        }
        break;
    case Mqtt5Command::COMMAND_PUBACK:
        {
            Mqtt5PubAckData data;
            ok = serialization.deserializePubAck(data, Mqtt5Command::COMMAND_PUBACK);
            if (ok)
            {
                ok = handleAck(connection, command, data.packetId, data.reasoncode);
            }
        }
        break;
    case Mqtt5Command::COMMAND_PUBREC:
        {
            Mqtt5PubAckData data;
            ok = serialization.deserializePubAck(data, Mqtt5Command::COMMAND_PUBREC);
            if (ok)
            {
                ok = handleAck(connection, command, data.packetId, data.reasoncode);
            }
        }
        break;
    case Mqtt5Command::COMMAND_PUBREL:
        {
            Mqtt5PubAckData data;
            ok = serialization.deserializePubAck(data, Mqtt5Command::COMMAND_PUBREL);
            if (ok)
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_setExactlyOne.erase(data.packetId);
                lock.unlock();
                Mqtt5PubAckData dataAck;
                dataAck.packetId = data.packetId;
                dataAck.reasoncode = 0;
                sendPubComp(connection, dataAck);
            }
        }
        break;
    case Mqtt5Command::COMMAND_PUBCOMP:
        {
            Mqtt5PubAckData data;
            ok = serialization.deserializePubAck(data, Mqtt5Command::COMMAND_PUBCOMP);
            if (ok)
            {
                ok = handleAck(connection, command, data.packetId, data.reasoncode);
            }
        }
        break;
    case Mqtt5Command::COMMAND_SUBSCRIBE:
        {
            Mqtt5SubscribeData data;
            ok = serialization.deserializeSubscribe(data);
            if (ok)
            {
                Mqtt5SubAckData dataAck;
                dataAck.packetId = data.packetId;
                dataAck.reasoncodes = std::vector<std::uint8_t>(data.subscriptions.size(), 0);
                sendSubAck(connection, dataAck);
            }
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
            if (ok)
            {
                unsigned int reasoncode = 0;
                if (!data.reasoncodes.empty())
                {
                    reasoncode = data.reasoncodes[0];
                }
                ok = handleAck(connection, command, data.packetId, reasoncode);
            }
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
            if (ok)
            {
                Mqtt5SubAckData dataAck;
                dataAck.packetId = data.packetId;
                dataAck.reasoncodes = std::vector<std::uint8_t>(data.topics.size(), 0);
                sendUnsubAck(connection, dataAck);
            }
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
            if (ok)
            {
                unsigned int reasoncode = 0;
                if (!data.reasoncodes.empty())
                {
                    reasoncode = data.reasoncodes[0];
                }
                ok = handleAck(connection, command, data.packetId, reasoncode);
            }
            if (callback && ok)
            {
                callback->receivedUnsubAck(data);
            }
        }
        break;
    case Mqtt5Command::COMMAND_PINGREQ:
        ok = true;
        if (callback)
        {
            callback->receivedPingReq();
        }
        break;
    case Mqtt5Command::COMMAND_PINGRESP:
        ok = true;
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

    clearState();

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
    std::unique_lock<std::mutex> lock(m_mutex);
    m_callback = callback;
}

void Mqtt5Protocol::sendConnect(const IStreamConnectionPtr& connection, const Mqtt5ConnectData& data)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_connecting = true;
    clearState();
    lock.unlock();

    unsigned int sizePropPayload = 0;
    unsigned int sizePropWillMessage = 0;
    unsigned int sizePayload = Mqtt5Serialization::sizeConnect(data, sizePropPayload, sizePropWillMessage);
    unsigned int sizeMessage = 1u + Mqtt5Serialization::sizeVarByteNumber(sizePayload) + sizePayload;
    IMessagePtr message = std::make_shared<ProtocolMessage>(0);
    char* buffer = message->addSendHeader(sizeMessage);
    Mqtt5Serialization serialization(buffer, sizeMessage, 0);
    serialization.serializeConnect(data, sizePayload, sizePropPayload, sizePropWillMessage);

    if (connection)
    {
        connection->sendMessage(message);
    }
}

void Mqtt5Protocol::sendConnAck(const IStreamConnectionPtr& connection, const Mqtt5ConnAckData& data)
{
    unsigned int sizePropPayload = 0;
    unsigned int sizePayload = Mqtt5Serialization::sizeConnAck(data, sizePropPayload);
    unsigned int sizeMessage = 1u + Mqtt5Serialization::sizeVarByteNumber(sizePayload) + sizePayload;
    IMessagePtr message = std::make_shared<ProtocolMessage>(0);
    char* buffer = message->addSendHeader(sizeMessage);
    Mqtt5Serialization serialization(buffer, sizeMessage, 0);
    serialization.serializeConnAck(data, sizePayload, sizePropPayload);

    if (connection)
    {
        connection->sendMessage(message);
    }
}


void Mqtt5Protocol::resendMessages(const IStreamConnectionPtr& connection)
{
    if (connection)
    {
        for (const IMessagePtr& message : m_messagesWaitAck)
        {
            assert(message);

            // set dup
            const std::list<BufferRef>& buffers = message->getAllSendBuffers();
            if (!buffers.empty() && (buffers.front().second > 0))
            {
                const BufferRef bufferRef = buffers.front();
                assert(bufferRef.first);
                char& header = bufferRef.first[0];
                if (HEADER_Command(header) == Mqtt5Command::COMMAND_PUBLISH)
                {
                    header |= HEADER_SetDup(header);
                }
            }
            connection->sendMessage(message);
        }
    }
}


void Mqtt5Protocol::sendPendingMessages(const IStreamConnectionPtr& connection)
{
    if (connection)
    {
        while (!m_messagesPending.empty())
        {
            unsigned int packetId = getPacketId();
            if (packetId != 0)
            {
                const PendingMessage& pendingMessage = m_messagesPending.front();
                IMessagePtr message = pendingMessage.message;
                prepareForSendWithPacketId(message, pendingMessage.bufferPacketId, pendingMessage.qos, pendingMessage.command, packetId);
                m_messagesPending.pop_front();
                connection->sendMessage(message);
            }
            else
            {
                return;
            }
        }
    }
}


static void putPacketIdIntoMessage(std::uint8_t* bufferPacketId, unsigned int packetId)
{
    if (bufferPacketId)
    {
        assert(packetId != 0);
        Mqtt5Serialization::write2ByteNumber(bufferPacketId, packetId);
    }
}


void Mqtt5Protocol::prepareForSendWithPacketId(const IMessagePtr& message, std::uint8_t* bufferPacketId, unsigned qos, unsigned int command, unsigned int packetId)
{
    assert(bufferPacketId != nullptr);
    assert(packetId < m_messageIdsAllocated.size());
    putPacketIdIntoMessage(bufferPacketId, packetId);
    m_messagesWaitAck.push_back(message);
    MessageStatus::Status status = MessageStatus::SENDSTAT_NONE;
    if (qos == 1)
    {
        Mqtt5Command cmd = static_cast<Mqtt5Command>(command);
        switch (cmd)
        {
        case Mqtt5Command::COMMAND_PUBLISH:
            status = MessageStatus::SENDSTAT_WAITPUBACK;
            break;
        case Mqtt5Command::COMMAND_SUBSCRIBE:
            status = MessageStatus::SENDSTAT_WAITSUBACK;
            break;
        case Mqtt5Command::COMMAND_UNSUBSCRIBE:
            status = MessageStatus::SENDSTAT_WAITUNSUBACK;
            break;
        default:
            assert(false);
            break;
        }
    }
    else
    {
        assert(qos == 2);
        status = MessageStatus::SENDSTAT_WAITPUBREC;
    }
    m_messageIdsAllocated[packetId] = { status,  --m_messagesWaitAck.end() };
}


unsigned int Mqtt5Protocol::getPacketId()
{
    unsigned int packetId = 0;
    if (!m_messageIdsFree.empty())
    {
        packetId = m_messageIdsFree.back();
        m_messageIdsFree.pop_back();
        assert(packetId != 0);
    }
    else
    {
        // -1, because 0 is not a packetId. The index = 0 is just a dummy and must not be counted
        if (m_messageIdsAllocated.size() - 1 < m_sendMax)
        {
            m_messageIdsAllocated.push_back({});
            packetId = static_cast<unsigned int>(m_messageIdsAllocated.size() - 1);
        }
    }
    return packetId;
}


bool Mqtt5Protocol::prepareForSend(const IMessagePtr& message, std::uint8_t* bufferPacketId, unsigned qos, unsigned int command)
{
    // qos == 0 -> these messages will not be treated with ack and flow control
    // messages will get lost when the connection is (temporarly) gone.
    if (qos == 0)
    {
        return !m_connecting;
    }

    // when pending messages are available -> put the message to the back to keep ordering
    if (!m_messagesPending.empty() || m_connecting)
    {
        m_messagesPending.emplace_back(PendingMessage{ message, bufferPacketId, qos, command });
        return false;
    }

    unsigned int packetId = getPacketId();
    if (packetId != 0)
    {
        prepareForSendWithPacketId(message, bufferPacketId, qos, command, packetId);
        return true;
    }
    else
    {
        m_messagesPending.emplace_back(PendingMessage{ message, bufferPacketId, qos, command });
        return false;
    }
}



void Mqtt5Protocol::sendPublish(const IStreamConnectionPtr& connection, Mqtt5PublishData& data, const IMessagePtr& message)
{
    unsigned int sizeAppPayload = static_cast<unsigned int>(message->getTotalSendPayloadSize());
    unsigned int sizePropPayload = 0;
    unsigned int sizePayload = Mqtt5Serialization::sizePublish(data, sizePropPayload) + sizeAppPayload;
    int sizeMessage = 1 + Mqtt5Serialization::sizeVarByteNumber(sizePayload) + sizePayload;
    char* buffer = message->addSendHeader(sizeMessage - sizeAppPayload);
    Mqtt5Serialization serialization(buffer, sizeMessage - sizeAppPayload, 0);
    std::uint8_t* bufferPacketId = nullptr;
    serialization.serializePublish(data, sizePayload, sizePropPayload, bufferPacketId);

    std::unique_lock<std::mutex> lock(m_mutex);
    bool doTheSend = prepareForSend(message, bufferPacketId, data.qos, Mqtt5Command::COMMAND_PUBLISH);
    if (connection && doTheSend)
    {
        connection->sendMessage(message);
    }
}


void Mqtt5Protocol::sendPubAck(const IStreamConnectionPtr& connection, unsigned int command, const Mqtt5PubAckData& data)
{
    unsigned int sizePropPayload = 0;
    unsigned int sizePayload = Mqtt5Serialization::sizePubAck(data, sizePropPayload);
    int sizeMessage = 1 + Mqtt5Serialization::sizeVarByteNumber(sizePayload) + sizePayload;
    IMessagePtr message = std::make_shared<ProtocolMessage>(0);
    char* buffer = message->addSendHeader(sizeMessage);
    Mqtt5Serialization serialization(buffer, sizeMessage, 0);
    serialization.serializePubAck(data, static_cast<Mqtt5Command>(command), sizePayload, sizePropPayload);

    if (connection)
    {
        connection->sendMessage(message);
    }
}



void Mqtt5Protocol::sendPubAck(const IStreamConnectionPtr& connection, const Mqtt5PubAckData& data)
{
    sendPubAck(connection, Mqtt5Command::COMMAND_PUBACK, data);
}

void Mqtt5Protocol::sendPubRec(const IStreamConnectionPtr& connection, const Mqtt5PubAckData& data)
{
    sendPubAck(connection, Mqtt5Command::COMMAND_PUBREC, data);
}

void Mqtt5Protocol::sendPubRel(const IStreamConnectionPtr& connection, const Mqtt5PubAckData& data, const IMessagePtr& message)
{
    unsigned int sizePropPayload = 0;
    unsigned int sizePayload = Mqtt5Serialization::sizePubAck(data, sizePropPayload);
    int sizeMessage = 1 + Mqtt5Serialization::sizeVarByteNumber(sizePayload) + sizePayload;
    char* buffer = message->addSendHeader(sizeMessage);
    Mqtt5Serialization serialization(buffer, sizeMessage, 0);
    serialization.serializePubAck(data, Mqtt5Command::COMMAND_PUBREL, sizePayload, sizePropPayload);

    if (connection)
    {
        connection->sendMessage(message);
    }
}

void Mqtt5Protocol::sendPubComp(const IStreamConnectionPtr& connection, const Mqtt5PubAckData& data)
{
    sendPubAck(connection, Mqtt5Command::COMMAND_PUBCOMP, data);
}




void Mqtt5Protocol::sendSubscribe(const IStreamConnectionPtr& connection, const Mqtt5SubscribeData& data)
{
    unsigned int sizePropPayload = 0;
    unsigned int sizePayload = Mqtt5Serialization::sizeSubscribe(data, sizePropPayload);
    int sizeMessage = 1 + Mqtt5Serialization::sizeVarByteNumber(sizePayload) + sizePayload;
    IMessagePtr message = std::make_shared<ProtocolMessage>(0);
    char* buffer = message->addSendHeader(sizeMessage);
    Mqtt5Serialization serialization(buffer, sizeMessage, 0);
    std::uint8_t* bufferPacketId = nullptr;
    serialization.serializeSubscribe(data, sizePayload, sizePropPayload, bufferPacketId);

    std::unique_lock<std::mutex> lock(m_mutex);
    bool doTheSend = prepareForSend(message, bufferPacketId, 1, Mqtt5Command::COMMAND_SUBSCRIBE);
    if (connection && doTheSend)
    {
        connection->sendMessage(message);
    }
}

void Mqtt5Protocol::sendSubAck(const IStreamConnectionPtr& connection, unsigned int command, const Mqtt5SubAckData& data)
{
    unsigned int sizePropPayload = 0;
    unsigned int sizePayload = Mqtt5Serialization::sizeSubAck(data, sizePropPayload);
    int sizeMessage = 1 + Mqtt5Serialization::sizeVarByteNumber(sizePayload) + sizePayload;
    IMessagePtr message = std::make_shared<ProtocolMessage>(0);
    char* buffer = message->addSendHeader(sizeMessage);
    Mqtt5Serialization serialization(buffer, sizeMessage, 0);
    serialization.serializeSubAck(data, static_cast<Mqtt5Command>(command), sizePayload, sizePropPayload);

    if (connection)
    {
        connection->sendMessage(message);
    }
}


void Mqtt5Protocol::sendSubAck(const IStreamConnectionPtr& connection, const Mqtt5SubAckData& data)
{
    sendSubAck(connection, Mqtt5Command::COMMAND_SUBACK, data);
}

void Mqtt5Protocol::sendUnsubscribe(const IStreamConnectionPtr& connection, const Mqtt5UnsubscribeData& data)
{
    unsigned int sizePropPayload = 0;
    unsigned int sizePayload = Mqtt5Serialization::sizeUnsubscribe(data, sizePropPayload);
    int sizeMessage = 1 + Mqtt5Serialization::sizeVarByteNumber(sizePayload) + sizePayload;
    IMessagePtr message = std::make_shared<ProtocolMessage>(0);
    char* buffer = message->addSendHeader(sizeMessage);
    Mqtt5Serialization serialization(buffer, sizeMessage, 0);
    std::uint8_t* bufferPacketId = nullptr;
    serialization.serializeUnsubscribe(data, sizePayload, sizePropPayload, bufferPacketId);

    std::unique_lock<std::mutex> lock(m_mutex);
    bool doTheSend = prepareForSend(message, bufferPacketId, 1, Mqtt5Command::COMMAND_UNSUBSCRIBE);
    if (connection && doTheSend)
    {
        connection->sendMessage(message);
    }
}

void Mqtt5Protocol::sendUnsubAck(const IStreamConnectionPtr& connection, const Mqtt5SubAckData& data)
{
    sendSubAck(connection, Mqtt5Command::COMMAND_UNSUBACK, data);
}

void Mqtt5Protocol::sendPingReq(const IStreamConnectionPtr& connection)
{
    int sizeMessage = 1 + Mqtt5Serialization::sizeVarByteNumber(0);
    IMessagePtr message = std::make_shared<ProtocolMessage>(0);
    char* buffer = message->addSendHeader(sizeMessage);
    Mqtt5Serialization serialization(buffer, sizeMessage, 0);
    serialization.serializePingReq();

    if (connection)
    {
        connection->sendMessage(message);
    }
}

void Mqtt5Protocol::sendPingResp(const IStreamConnectionPtr& connection)
{
    int sizeMessage = 1 + Mqtt5Serialization::sizeVarByteNumber(0);
    IMessagePtr message = std::make_shared<ProtocolMessage>(0);
    char* buffer = message->addSendHeader(sizeMessage);
    Mqtt5Serialization serialization(buffer, sizeMessage, 0);
    serialization.serializePingResp();

    if (connection)
    {
        connection->sendMessage(message);
    }
}

void Mqtt5Protocol::sendDisconnect(const IStreamConnectionPtr& connection, const Mqtt5DisconnectData& data)
{
    unsigned int sizePropPayload = 0;
    unsigned int sizePayload = Mqtt5Serialization::sizeDisconnect(data, sizePropPayload);
    int sizeMessage = 1 + Mqtt5Serialization::sizeVarByteNumber(sizePayload) + sizePayload;
    IMessagePtr message = std::make_shared<ProtocolMessage>(0);
    char* buffer = message->addSendHeader(sizeMessage);
    Mqtt5Serialization serialization(buffer, sizeMessage, 0);
    serialization.serializeDisconnect(data, sizePayload, sizePropPayload);

    if (connection)
    {
        connection->sendMessage(message);
    }
}

void Mqtt5Protocol::sendAuth(const IStreamConnectionPtr& connection, const Mqtt5AuthData& data)
{
    unsigned int sizePropPayload = 0;
    unsigned int sizePayload = Mqtt5Serialization::sizeAuth(data, sizePropPayload);
    int sizeMessage = 1 + Mqtt5Serialization::sizeVarByteNumber(sizePayload) + sizePayload;
    IMessagePtr message = std::make_shared<ProtocolMessage>(0);
    char* buffer = message->addSendHeader(sizeMessage);
    Mqtt5Serialization serialization(buffer, sizeMessage, 0);
    serialization.serializeAuth(data, sizePayload, sizePropPayload);

    if (connection)
    {
        connection->sendMessage(message);
    }
}



}   // namespace finalmq
