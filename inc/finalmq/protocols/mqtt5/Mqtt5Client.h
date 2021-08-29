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
#include "finalmq/streamconnection/StreamConnection.h"
#include "finalmq/metadata/MetaType.h"
#include "finalmq/protocols/mqtt5/Mqtt5Protocol.h"
#include "finalmq/helpers/PollingTimer.h"


namespace finalmq {


struct IMqtt5ClientCallback
{
    virtual ~IMqtt5ClientCallback() = default;

    struct ConnAckData
    {
        bool sessionPresent = false;
        std::uint8_t reasoncode = 0;
        std::uint32_t sessionExpiryInterval = 0;
        std::uint16_t receiveMaximum = 65535;
        std::uint8_t maximumQoS = 2;
        bool retainAvailable = true;
        std::uint32_t maximumPacketSize = 268435455;
        std::string assignedClientId;
        std::string reasonString;
        bool wildcardSubscriptionAvailable = true;
        bool subscriptionIdentifiersAvailable = true;
        bool sharedSubscriptionAvailable = true;
        std::uint16_t serverKeepAlive = 0;
        std::string serverReference;
        std::string authenticationMethod;
        Bytes authenticationData;
    };
    virtual void receivedConnAck(const ConnAckData& data) = 0;

    struct PublishData
    {
        std::uint8_t qos = 0;
        bool retain = false;
        std::string topic;
        std::uint32_t messageExpiryInterval = 0;
        std::string responseTopic;
        Bytes correlationData;
        std::string contentType;
    };
    virtual void receivedPublish(const PublishData& data, const IMessagePtr& message) = 0;

    virtual void receivedSubAck(const std::vector<std::uint8_t>& reasoncodes) = 0;
    virtual void receivedUnsubAck(const std::vector<std::uint8_t>& reasoncodes) = 0;
    virtual void receivedPingResp() = 0;

    struct DisconnectData
    {
        std::uint8_t reasoncode = 0;
        std::string reasonString;
        std::string serverReference;
    };
    virtual void receivedDisconnect(const DisconnectData& data) = 0;

    struct AuthData
    {
        std::uint8_t reasoncode = 0;
        std::string authenticationMethod;
        Bytes authenticationData;
        std::string reasonString;
    };
    virtual void receivedAuth(const AuthData& data) = 0;

    virtual void closeConnection() = 0;
};

struct IMqtt5Client
{
    virtual ~IMqtt5Client() = default;
    virtual void setCallback(hybrid_ptr<IMqtt5ClientCallback> callback) = 0;

    struct WillMessage
    {
        std::uint8_t qos = 0;
        bool retain = false;
        std::uint32_t delayInterval = 0;
        std::string contentType;
        std::string topic;
        Bytes payload;
    };
    struct ConnectData
    {
        bool cleanStart = false;
        std::unique_ptr<WillMessage> willMessage;
        std::string username;
        std::string password;
        std::uint16_t keepAlive = 0;
        std::uint32_t sessionExpiryInterval = 0;
        std::uint16_t receiveMaximum = 65535;
        std::string clientId;
    };
    virtual void startConnection(const IStreamConnectionPtr& connection, const ConnectData& data) = 0;

    struct PublishData
    {
        std::uint8_t qos = 0;
        bool retain = false;
        std::string topic;
        std::uint32_t messageExpiryInterval = 0;
        std::string responseTopic;
        Bytes correlationData;
        std::string contentType;
    };
    virtual void publish(const IStreamConnectionPtr& connection, PublishData&& data, const IMessagePtr& message) = 0;

    struct SubscribeEntry
    {
        std::string topic;
        unsigned int retainHandling = 0;
        bool retainAsPublished = false;
        bool noLocal = false;
        unsigned int qos = 0;
    };
    struct SubscribeData
    {
        std::vector<SubscribeEntry> subscriptions;
    };
    virtual void subscribe(const IStreamConnectionPtr& connection, const SubscribeData& data) = 0;
    virtual void unsubscribe(const IStreamConnectionPtr& connection, const std::vector<std::string>& topics) = 0;

    struct DisconnectData
    {
        std::uint8_t reasoncode = 0;
        std::string reasonString;
        std::string serverReference;
    };
    virtual void endConnection(const IStreamConnectionPtr& connection, const DisconnectData& data) = 0;

    struct AuthData
    {
        std::uint8_t reasoncode = 0;
        std::string authenticationMethod;
        Bytes authenticationData;
        std::string reasonString;
    };
    virtual void auth(const IStreamConnectionPtr& connection, const AuthData& data) = 0;

    virtual void cycleTime(const IStreamConnectionPtr& connection) = 0;
};



class Mqtt5Client : public IMqtt5Client
                  , private IMqtt5ProtocolCallback
{
public:
    Mqtt5Client();

private:
    // IMqtt5Protocol
    virtual void setCallback(hybrid_ptr<IMqtt5ClientCallback> callback) override;
    virtual void startConnection(const IStreamConnectionPtr& connection, const ConnectData& data) override;
    virtual void publish(const IStreamConnectionPtr& connection, PublishData&& data, const IMessagePtr& message) override;
    virtual void subscribe(const IStreamConnectionPtr& connection, const SubscribeData& data) override;
    virtual void unsubscribe(const IStreamConnectionPtr& connection, const std::vector<std::string>& topics) override;
    virtual void endConnection(const IStreamConnectionPtr& connection, const DisconnectData& data) override;
    virtual void auth(const IStreamConnectionPtr& connection, const AuthData& data) override;
    virtual void cycleTime(const IStreamConnectionPtr& connection) override;

    // IMqtt5ProtocolCallback
    virtual void receivedConnect(const Mqtt5ConnectData& data) override;
    virtual void receivedConnAck(const Mqtt5ConnAckData& data) override;
    virtual void receivedPublish(Mqtt5PublishData&& data, const IMessagePtr& message) override;
    virtual void receivedSubscribe(const Mqtt5SubscribeData& data) override;
    virtual void receivedSubAck(const Mqtt5SubAckData& data) override;
    virtual void receivedUnsubscribe(const Mqtt5UnsubscribeData& data) override;
    virtual void receivedUnsubAck(const Mqtt5SubAckData& data) override;
    virtual void receivedPingReq() override;
    virtual void receivedPingResp() override;
    virtual void receivedDisconnect(const Mqtt5DisconnectData& data) override;
    virtual void receivedAuth(const Mqtt5AuthData& data) override;

    void setReceiveActivity();

    hybrid_ptr<IMqtt5ClientCallback> m_callback;
    std::uint16_t m_keepAlive = 0;
    PollingTimer  m_timerPing;
    PollingTimer  m_timerReceiveActivity;

    std::uint32_t m_sessionExpiryInterval = 0;

    std::unique_ptr<IMqtt5Protocol> m_protocol = std::make_unique<Mqtt5Protocol>();
};



}   // namespace finalmq
