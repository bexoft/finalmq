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


#include "finalmq/protocols/mqtt5/Mqtt5Client.h"
#include "finalmq/protocols/mqtt5/Mqtt5Properties.h"
#include "finalmq/variant/VariantValues.h"


namespace finalmq {


Mqtt5Client::Mqtt5Client()
{
}


// IMqtt5Protocol
void Mqtt5Client::setCallback(hybrid_ptr<IMqtt5ClientCallback> callback)
{
    m_callback = callback;
}

void Mqtt5Client::startConnection(const IStreamConnectionPtr& connection, const ConnectData& data)
{
    Mqtt5ConnectData dataInternal;
    dataInternal.cleanStart = data.cleanStart;
    if (data.willMessage)
    {
        dataInternal.willMessage = std::make_unique<Mqtt5WillMessage>();
        dataInternal.willMessage->qos = data.willMessage->qos;
        if (data.willMessage->delayInterval != 0)
        {
            dataInternal.willMessage->properties[Mqtt5PropertyId::WillDelayInterval] = data.willMessage->delayInterval;
        }
        if (!data.willMessage->contentType.empty())
        {
            dataInternal.willMessage->properties[Mqtt5PropertyId::ContentType] = data.willMessage->contentType;
        }
        dataInternal.willMessage->topic = data.willMessage->topic;
        dataInternal.willMessage->payload = data.willMessage->payload;
    }
    dataInternal.username = data.username;
    dataInternal.password = data.password;
    dataInternal.keepAlive = data.keepAlive;
    if (data.sessionExpiryInterval != 0)
    {
        dataInternal.properties[Mqtt5PropertyId::SessionExpiryInterval] = data.sessionExpiryInterval;
    }
    if (data.receiveMaximum != 65535)
    {
        dataInternal.properties[Mqtt5PropertyId::ReceiveMaximum] = (std::uint32_t)data.receiveMaximum;
    }
    dataInternal.clientId = data.clientId;

    m_protocol->sendConnect(connection, dataInternal);
}

void Mqtt5Client::publish(const IStreamConnectionPtr& connection, PublishData&& data, const IMessagePtr& message)
{
    Mqtt5PublishData dataInternal;
    dataInternal.qos = data.qos;
    dataInternal.retain = data.retain;
    dataInternal.topic = std::move(data.topic);
    if (data.messageExpiryInterval != 0)
    {
        dataInternal.properties[Mqtt5PropertyId::MessageExpiryInterval] = data.messageExpiryInterval;
    }
    if (!data.responseTopic.empty())
    {
        dataInternal.properties[Mqtt5PropertyId::ResponseTopic] = std::move(data.responseTopic);
    }
    if (!data.correlationData.empty())
    {
        dataInternal.properties[Mqtt5PropertyId::CorrelationData] = std::move(data.correlationData);
    }
    if (!data.contentType.empty())
    {
        dataInternal.properties[Mqtt5PropertyId::ContentType] = std::move(data.contentType);
    }

    m_protocol->sendPublish(connection, dataInternal, message);
}

void Mqtt5Client::subscribe(const IStreamConnectionPtr& connection, const SubscribeData& data)
{
    Mqtt5SubscribeData dataInternal;
    dataInternal.subscriptions.resize(data.subscriptions.size());
    for (size_t i = 0; i < data.subscriptions.size(); ++i)
    {
        const SubscribeEntry& entry = data.subscriptions[i];
        Mqtt5SubscribeEntry dest = dataInternal.subscriptions[i];
        dest.topic = entry.topic;
        dest.retainHandling = entry.retainHandling;
        dest.retainAsPublished = entry.retainAsPublished;
        dest.noLocal = entry.noLocal;
        dest.qos = entry.qos;
    }

    m_protocol->sendSubscribe(connection, dataInternal);
}

void Mqtt5Client::unsubscribe(const IStreamConnectionPtr& connection, const std::vector<std::string>& topics)
{
    Mqtt5UnsubscribeData dataInternal;
    dataInternal.topics = topics;
    m_protocol->sendUnsubscribe(connection, dataInternal);
}


void Mqtt5Client::endConnection(const IStreamConnectionPtr& connection, const DisconnectData& data)
{
    Mqtt5DisconnectData dataInternal;
    dataInternal.reasoncode = data.reasoncode;
    if (!data.reasonString.empty())
    {
        dataInternal.properties[Mqtt5PropertyId::ReasonString] = data.reasonString;
    }
    if (!data.serverReference.empty())
    {
        dataInternal.properties[Mqtt5PropertyId::ServerReference] = data.serverReference;
    }
}


void Mqtt5Client::auth(const IStreamConnectionPtr& connection, const AuthData& data)
{
    Mqtt5AuthData dataInternal;
    dataInternal.reasoncode = data.reasoncode;
    if (!data.reasonString.empty())
    {
        dataInternal.properties[Mqtt5PropertyId::ReasonString] = data.reasonString;
    }
    if (!data.authenticationMethod.empty())
    {
        dataInternal.properties[Mqtt5PropertyId::AuthenticationMethod] = data.authenticationMethod;
    }
    if (!data.authenticationData.empty())
    {
        dataInternal.properties[Mqtt5PropertyId::AuthenticationData] = data.authenticationData;
    }
}



void Mqtt5Client::timerCyclus(const IStreamConnectionPtr& connection)
{

}


// IMqtt5ProtocolCallback
void Mqtt5Client::receivedConnect(const Mqtt5ConnectData& data)
{
    // a client should never receive this message
}

void Mqtt5Client::receivedConnAck(const Mqtt5ConnAckData& data)
{
    IMqtt5ClientCallback::ConnAckData dataDest;
    dataDest.sessionPresent = data.sessionPresent;
    dataDest.reasoncode = data.reasoncode;
    std::unordered_map<unsigned int, Variant>::const_iterator it;
    if ((it = data.properties.find(Mqtt5PropertyId::SessionExpiryInterval)) != data.properties.end())
    {
        dataDest.sessionExpiryInterval = it->second;
    }
    if ((it = data.properties.find(Mqtt5PropertyId::ReceiveMaximum)) != data.properties.end())
    {
        dataDest.receiveMaximum = static_cast<std::uint32_t>(it->second);
    }
    if ((it = data.properties.find(Mqtt5PropertyId::MaximumQoS)) != data.properties.end())
    {
        dataDest.maximumQoS = static_cast<std::uint32_t>(it->second);
    }
    if ((it = data.properties.find(Mqtt5PropertyId::RetainAvailable)) != data.properties.end())
    {
        dataDest.retainAvailable = it->second;
    }
    if ((it = data.properties.find(Mqtt5PropertyId::MaximumPacketSize)) != data.properties.end())
    {
        dataDest.maximumPacketSize = it->second;
    }
    if ((it = data.properties.find(Mqtt5PropertyId::AssignedClientIdentifier)) != data.properties.end())
    {
        std::string s = it->second;
        dataDest.assignedClientId = std::move(s);
    }
    if ((it = data.properties.find(Mqtt5PropertyId::ReasonString)) != data.properties.end())
    {
        std::string s = it->second;
        dataDest.reasonString = std::move(s);
    }
    if ((it = data.properties.find(Mqtt5PropertyId::WildcardSubscriptionAvailable)) != data.properties.end())
    {
        dataDest.wildcardSubscriptionAvailable = it->second;
    }
    if ((it = data.properties.find(Mqtt5PropertyId::SubscriptionIdentifiersAvailable)) != data.properties.end())
    {
        dataDest.subscriptionIdentifiersAvailable = it->second;
    }
    if ((it = data.properties.find(Mqtt5PropertyId::SharedSubscriptionAvailable)) != data.properties.end())
    {
        dataDest.sharedSubscriptionAvailable = it->second;
    }
    if ((it = data.properties.find(Mqtt5PropertyId::ServerKeepAlive)) != data.properties.end())
    {
        dataDest.serverKeepAlive = static_cast<std::uint32_t>(it->second);
    }
    if ((it = data.properties.find(Mqtt5PropertyId::ServerReference)) != data.properties.end())
    {
        std::string s = it->second;
        dataDest.serverReference = std::move(s);
    }
    if ((it = data.properties.find(Mqtt5PropertyId::AuthenticationMethod)) != data.properties.end())
    {
        std::string s = it->second;
        dataDest.authenticationMethod = std::move(s);
    }
    if ((it = data.properties.find(Mqtt5PropertyId::AuthenticationData)) != data.properties.end())
    {
        Bytes b = it->second;
        dataDest.authenticationData = std::move(b);
    }

    auto callback = m_callback.lock();
    if (callback)
    {
        callback->receivedConnAck(dataDest);
    }
}

void Mqtt5Client::receivedPublish(Mqtt5PublishData&& data, const IMessagePtr& message)
{
    IMqtt5ClientCallback::PublishData dataDest;

    dataDest.qos = data.qos;
    dataDest.retain = data.retain;
    dataDest.topic = data.topic;
    std::unordered_map<unsigned int, Variant>::iterator it;
    if ((it = data.properties.find(Mqtt5PropertyId::MessageExpiryInterval)) != data.properties.end())
    {
        dataDest.messageExpiryInterval = it->second;
    }
    if ((it = data.properties.find(Mqtt5PropertyId::ResponseTopic)) != data.properties.end())
    {
        std::string* s = it->second;
        if (s)
        {
            dataDest.responseTopic = std::move(*s);
        }
    }
    if ((it = data.properties.find(Mqtt5PropertyId::CorrelationData)) != data.properties.end())
    {
        Bytes* b = it->second;
        if (b)
        {
            dataDest.correlationData = std::move(*b);
        }
    }
    if ((it = data.properties.find(Mqtt5PropertyId::ContentType)) != data.properties.end())
    {
        std::string* s = it->second;
        if (s)
        {
            dataDest.contentType = std::move(*s);
        }
    }

    auto callback = m_callback.lock();
    if (callback)
    {
        callback->receivedPublish(dataDest, message);
    }
}

void Mqtt5Client::receivedSubscribe(const Mqtt5SubscribeData& data)
{
    // a client should never receive this message
}

void Mqtt5Client::receivedSubAck(const Mqtt5SubAckData& data)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->receivedSubAck(data.reasoncodes);
    }
}

void Mqtt5Client::receivedUnsubscribe(const Mqtt5UnsubscribeData& data)
{
    // a client should never receive this message
}

void Mqtt5Client::receivedUnsubAck(const Mqtt5SubAckData& data)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->receivedUnsubAck(data.reasoncodes);
    }
}

void Mqtt5Client::receivedPingReq()
{
    // a client should never receive this message
}

void Mqtt5Client::receivedPingResp()
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->receivedPingResp();
    }
}

void Mqtt5Client::receivedDisconnect(const Mqtt5DisconnectData& data)
{
    IMqtt5ClientCallback::DisconnectData dataDest;

    dataDest.reasoncode = data.reasoncode;
    std::unordered_map<unsigned int, Variant>::const_iterator it;
    if ((it = data.properties.find(Mqtt5PropertyId::ReasonString)) != data.properties.end())
    {
        const std::string* s = it->second;
        if (s)
        {
            dataDest.reasonString = *s;
        }
    }
    if ((it = data.properties.find(Mqtt5PropertyId::ServerReference)) != data.properties.end())
    {
        const std::string* s = it->second;
        if (s)
        {
            dataDest.serverReference = *s;
        }
    }

    auto callback = m_callback.lock();
    if (callback)
    {
        callback->receivedDisconnect(dataDest);
    }
}

void Mqtt5Client::receivedAuth(const Mqtt5AuthData& data)
{
    IMqtt5ClientCallback::AuthData dataDest;

    dataDest.reasoncode = data.reasoncode;
    std::unordered_map<unsigned int, Variant>::const_iterator it;
    if ((it = data.properties.find(Mqtt5PropertyId::ReasonString)) != data.properties.end())
    {
        const std::string* s = it->second;
        if (s)
        {
            dataDest.reasonString = *s;
        }
    }
    if ((it = data.properties.find(Mqtt5PropertyId::AuthenticationMethod)) != data.properties.end())
    {
        const std::string* s = it->second;
        if (s)
        {
            dataDest.authenticationMethod = *s;
        }
    }
    if ((it = data.properties.find(Mqtt5PropertyId::AuthenticationData)) != data.properties.end())
    {
        const Bytes* b = it->second;
        if (b)
        {
            dataDest.authenticationData = *b;
        }
    }

    auto callback = m_callback.lock();
    if (callback)
    {
        callback->receivedAuth(dataDest);
    }
}


}   // namespace finalmq
