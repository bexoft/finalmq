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
#include "finalmq/protocolsession/IProtocol.h"
#include "finalmq/helpers/FmqDefines.h"
#include "finalmq/protocols/mqtt5/Mqtt5CommandData.h"

#include <deque>
#include <functional>

namespace finalmq {

struct IMqtt5ProtocolCallback
{
    virtual ~IMqtt5ProtocolCallback() = default;
    virtual void receivedConnect(const Mqtt5ConnectData& data) = 0;
    virtual void receivedConnAck(const Mqtt5ConnAckData& data) = 0;
    virtual void receivedPublish(const Mqtt5PublishData& data, const IMessagePtr& message) = 0;
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
    virtual void connected(const IStreamConnectionPtr& connection) = 0;
    virtual void disconnected() = 0;
    virtual bool sendConnect(const Mqtt5ConnectData& data) = 0;
    virtual bool sendConnAck(const Mqtt5ConnAckData& data) = 0;
    virtual bool sendPublish(const Mqtt5PublishData& data) = 0;
    virtual bool sendSubscribe(const Mqtt5SubscribeData& data) = 0;
    virtual bool sendSubAck(const Mqtt5SubAckData& data) = 0;
    virtual bool sendUnsubscribe(const Mqtt5UnsubscribeData& data) = 0;
    virtual bool sendUnsubAck(const Mqtt5SubAckData& data) = 0;
    virtual bool sendPingReq() = 0;
    virtual bool sendPingResp() = 0;
    virtual bool sendDisconnect(const Mqtt5DisconnectData& data) = 0;
    virtual bool sendAuth(const Mqtt5AuthData& data) = 0;
};


class Mqtt5Protocol : public IMqtt5Protocol
{
public:
    Mqtt5Protocol();

    bool receive(const SocketPtr& socket, int bytesToRead);

public:
    enum class State
    {
        WAITFORHEADER,
        WAITFORLENGTH,
        WAITFORPAYLOAD,
        MESSAGECOMPLETE,
    };

    bool receiveHeader(const SocketPtr& socket, int& bytesToRead);
    bool receiveRemainingSize(const SocketPtr& socket, int& bytesToRead);
    void setPayloadSize();
    bool receivePayload(const SocketPtr& socket, int& bytesToRead);
    bool processPayload();
    void clearState();

    // IMqtt5Protocol
    virtual void setCallback(hybrid_ptr<IMqtt5ProtocolCallback> callback) override;
    virtual void connected(const IStreamConnectionPtr& connection) override;
    virtual void disconnected() override;
    virtual bool sendConnect(const Mqtt5ConnectData& data) override;
    virtual bool sendConnAck(const Mqtt5ConnAckData& data) override;
    virtual bool sendPublish(const Mqtt5PublishData& data) override;
    virtual bool sendSubscribe(const Mqtt5SubscribeData& data) override;
    virtual bool sendSubAck(const Mqtt5SubAckData& data) override;
    virtual bool sendUnsubscribe(const Mqtt5UnsubscribeData& data) override;
    virtual bool sendUnsubAck(const Mqtt5SubAckData& data) override;
    virtual bool sendPingReq() override;
    virtual bool sendPingResp() override;
    virtual bool sendDisconnect(const Mqtt5DisconnectData& data) override;
    virtual bool sendAuth(const Mqtt5AuthData& data) override;

    unsigned char m_header;
    int         m_remainingSize = 0;
    int         m_remainingSizeShift = 0;
    State       m_state = State::WAITFORHEADER;
    ssize_t     m_sizeCurrent = 0;

    ssize_t     m_sizePayload = 0;
    IMessagePtr m_message;
    Bytes       m_messageBuffer;
    char*       m_buffer = nullptr;

//    std::deque<IMessagePtr>* m_messages = nullptr;

    hybrid_ptr<IMqtt5ProtocolCallback> m_callback;
};



}   // namespace finalmq
