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

#include "finalmq/streamconnection/IMessage.h"
#include "finalmq/protocols/mqtt5/Mqtt5CommandData.h"
#include "finalmq/streamconnection/StreamConnection.h"

#include <deque>
#include <functional>
#include <unordered_set>

namespace finalmq {

struct IMqtt5ProtocolCallback
{
    virtual ~IMqtt5ProtocolCallback() = default;
    virtual void receivedConnect(const Mqtt5ConnectData& data) = 0;
    virtual void receivedConnAck(const Mqtt5ConnAckData& data) = 0;
    virtual void receivedPublish(Mqtt5PublishData&& data, const IMessagePtr& message) = 0;
    virtual void receivedSubscribe(const Mqtt5SubscribeData& data) = 0;
    virtual void receivedSubAck(const Mqtt5SubAckData& data) = 0;
    virtual void receivedUnsubscribe(const Mqtt5UnsubscribeData& data) = 0;
    virtual void receivedUnsubAck(const Mqtt5SubAckData& data) = 0;
    virtual void receivedPingReq() = 0;
    virtual void receivedPingResp() = 0;
    virtual void receivedDisconnect(const Mqtt5DisconnectData& data) = 0;
    virtual void receivedAuth(const Mqtt5AuthData& data) = 0;
};

struct IMqtt5Protocol
{
    virtual ~IMqtt5Protocol() = default;
    virtual void setCallback(hybrid_ptr<IMqtt5ProtocolCallback> callback) = 0;
    virtual void sendConnect(const IStreamConnectionPtr& connection, const Mqtt5ConnectData& data) = 0;
    virtual void sendConnAck(const IStreamConnectionPtr& connection, const Mqtt5ConnAckData& data) = 0;
    virtual void sendPublish(const IStreamConnectionPtr& connection, Mqtt5PublishData& data, const IMessagePtr& message) = 0;
    virtual void sendSubscribe(const IStreamConnectionPtr& connection, const Mqtt5SubscribeData& data) = 0;
    virtual void sendUnsubscribe(const IStreamConnectionPtr& connection, const Mqtt5UnsubscribeData& data) = 0;
    virtual void sendPingReq(const IStreamConnectionPtr& connection) = 0;
    virtual void sendPingResp(const IStreamConnectionPtr& connection) = 0;
    virtual void sendDisconnect(const IStreamConnectionPtr& connection, const Mqtt5DisconnectData& data) = 0;
    virtual void sendAuth(const IStreamConnectionPtr& connection, const Mqtt5AuthData& data) = 0;
};


class Mqtt5Protocol : public IMqtt5Protocol
{
public:
    Mqtt5Protocol();

    bool receive(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead);

public:
    enum class State
    {
        WAITFORHEADER,
        WAITFORLENGTH,
        WAITFORPAYLOAD,
        MESSAGECOMPLETE,
    };

    bool receiveHeader(const SocketPtr& socket, int& bytesToRead);
    bool receiveRemainingSize(const IStreamConnectionPtr& connection, const SocketPtr& socket, int& bytesToRead);
    void setPayloadSize();
    bool receivePayload(const IStreamConnectionPtr& connection, const SocketPtr& socket, int& bytesToRead);
    bool processPayload(const IStreamConnectionPtr& connection);
    void clearState();

    unsigned int getPacketId();
    void resendMessages(const IStreamConnectionPtr& connection);
    void sendPendingMessages(const IStreamConnectionPtr& connection);
    void prepareForSendWithPacketId(const IMessagePtr& message, char* bufferPacketId, unsigned qos, unsigned int command, unsigned int packetId);
    bool prepareForSend(const IMessagePtr& message, char* bufferPacketId, unsigned qos, unsigned int command);
    bool handleAck(const IStreamConnectionPtr& connection, unsigned int command, unsigned int packetId, unsigned int reasoncode);
    void sendPubAck(const IStreamConnectionPtr& connection, const Mqtt5PubAckData& data);
    void sendPubRec(const IStreamConnectionPtr& connection, const Mqtt5PubAckData& data);
    void sendPubRel(const IStreamConnectionPtr& connection, const Mqtt5PubAckData& data, const IMessagePtr& message);
    void sendPubComp(const IStreamConnectionPtr& connection, const Mqtt5PubAckData& data);
    void sendSubAck(const IStreamConnectionPtr& connection, const Mqtt5SubAckData& data);
    void sendUnsubAck(const IStreamConnectionPtr& connection, const Mqtt5SubAckData& data);
    void sendPubAck(const IStreamConnectionPtr& connection, unsigned int command, const Mqtt5PubAckData& data);
    void sendSubAck(const IStreamConnectionPtr& connection, unsigned int command, const Mqtt5SubAckData& data);


    // IMqtt5Protocol
    virtual void setCallback(hybrid_ptr<IMqtt5ProtocolCallback> callback) override;
    virtual void sendConnect(const IStreamConnectionPtr& connection, const Mqtt5ConnectData& data) override;
    virtual void sendConnAck(const IStreamConnectionPtr& connection, const Mqtt5ConnAckData& data) override;
    virtual void sendPublish(const IStreamConnectionPtr& connection, Mqtt5PublishData& data, const IMessagePtr& message) override;
    virtual void sendSubscribe(const IStreamConnectionPtr& connection, const Mqtt5SubscribeData& data) override;
    virtual void sendUnsubscribe(const IStreamConnectionPtr& connection, const Mqtt5UnsubscribeData& data) override;
    virtual void sendPingReq(const IStreamConnectionPtr& connection) override;
    virtual void sendPingResp(const IStreamConnectionPtr& connection) override;
    virtual void sendDisconnect(const IStreamConnectionPtr& connection, const Mqtt5DisconnectData& data) override;
    virtual void sendAuth(const IStreamConnectionPtr& connection, const Mqtt5AuthData& data) override;

    unsigned char m_header;
    int         m_remainingSize = 0;
    int         m_remainingSizeShift = 0;
    State       m_state = State::WAITFORHEADER;
    ssize_t     m_sizeCurrent = 0;

    ssize_t     m_sizePayload = 0;
    IMessagePtr m_message;
    Bytes       m_messageBuffer;
    char*       m_buffer = nullptr;

    struct PendingMessage
    {
        IMessagePtr     message;
        char*           bufferPacketId = nullptr;
        unsigned int    qos = 0;
        unsigned int    command = 0;
    };
    struct MessageStatus
    {
        enum Status
        {
            SENDSTAT_NONE,
            SENDSTAT_WAITPUBACK,
            SENDSTAT_WAITPUBREC,
            SENDSTAT_WAITPUBCOMP,
            SENDSTAT_WAITSUBACK,
            SENDSTAT_WAITUNSUBACK,
        };

        Status                              status = SENDSTAT_NONE;
        std::list<IMessagePtr>::iterator    iterator;
    };

    bool                        m_connecting = true;
    std::uint16_t               m_sendMax = 0;              ///< max. messages that will wait for an ack. In case of exceed, the messages will wait in m_messagesPending.
    std::deque<PendingMessage>  m_messagesPending;          ///< messages that were not sent, yet, because of flow control
    std::list<IMessagePtr>      m_messagesWaitAck;          ///< keeps the messages in order that could be resent
    std::deque<MessageStatus>   m_messageIdsAllocated;      ///< the message status of messages that wait for ack, the index is the message id
    std::deque<std::uint16_t>   m_messageIdsFree;           ///< free message ids
    std::unordered_set<std::uint16_t> m_setExactlyOne;

    hybrid_ptr<IMqtt5ProtocolCallback>  m_callback;

    std::mutex                          m_mutex;
};



}   // namespace finalmq
