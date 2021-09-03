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


#include "finalmq/protocols/ProtocolMqtt5.h"
#include "finalmq/streamconnection/Socket.h"
#include "finalmq/protocolsession/ProtocolMessage.h"
#include "finalmq/protocolsession/ProtocolRegistry.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValues.h"

#include "finalmq/helpers/ModulenameFinalmq.h"

#ifdef WIN32
#include <rpcdce.h>
#else
#include <uuid/uuid.h>
#endif



namespace finalmq {

const int ProtocolMqtt5::PROTOCOL_ID = 5;
const std::string ProtocolMqtt5::PROTOCOL_NAME = "mqtt5client";

const std::string ProtocolMqtt5::KEY_USERNAME = "username";
const std::string ProtocolMqtt5::KEY_PASSWORD = "password";
const std::string ProtocolMqtt5::KEY_SESSIONEXPIRYINTERVAL = "sessionexpiryinterval";
const std::string ProtocolMqtt5::KEY_KEEPALIVE = "keepalive";

static const std::string FMQ_CORRID = "fmq_corrid";
static const std::string FMQ_TYPE = "fmq_type";
static const std::string FMQ_VIRTUAL_SESSION_ID = "fmq_virtsessid";
static const std::string FMQ_DESTNAME = "fmq_destname";

static const std::uint8_t ReasonCodeDisconnectWithWillMessage = 0x04;


#ifdef WIN32
static std::string getUuid()
{
    UUID uuid = {};
    ::UuidCreate(&uuid);
    std::string strUuidRet;
    RPC_CSTR strUuid = nullptr;
    if (::UuidToStringA(&uuid, &strUuid) == RPC_S_OK)
    {
        strUuidRet = (char*)strUuid;
        ::RpcStringFreeA(&strUuid);
    }
    return strUuidRet;
}
#else
static std::string getUuid()
{
    std::string strUuid;   
    strUuid.resize(50);     // 37 is the exact number
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, const_cast<char*>(strUuid.data()));
    std::string strUuidRet = strUuid.data();
    return strUuidRet;
}
#endif



ProtocolMqtt5::ProtocolMqtt5(const Variant& data)
    : m_client(std::make_unique<Mqtt5Client>())
{
    m_username = data.getDataValue<std::string>(KEY_USERNAME);
    m_password = data.getDataValue<std::string>(KEY_PASSWORD);
    const Variant* entry = data.getVariant(KEY_SESSIONEXPIRYINTERVAL);
    if (entry)
    {
        m_sessionExpiryInterval = *entry;
    }
    entry = data.getVariant(KEY_KEEPALIVE);
    if (entry)
    {
        m_keepAlive = *entry;
    }
    m_clientId = getUuid();

    m_client->setCallback(this);
}




// IProtocol
void ProtocolMqtt5::setCallback(const std::weak_ptr<IProtocolCallback>& callback)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_callback = callback;
}

void ProtocolMqtt5::setConnection(const IStreamConnectionPtr& connection)
{
    IMqtt5Client::ConnectData data;
    data.cleanStart = m_firstConnection;
    data.clientId = m_clientId;
    data.receiveMaximum = 128;
    data.username = m_username;
    data.password = m_password;
    data.keepAlive = m_keepAlive;
    data.sessionExpiryInterval = m_sessionExpiryInterval;
    data.willMessage = std::make_unique<IMqtt5Client::WillMessage>();
    data.willMessage->payload.insert(data.willMessage->payload.end(), m_clientId.begin(), m_clientId.end());
    data.willMessage->qos = 2;
    data.willMessage->topic = "fmq_willmessages";
    data.willMessage->retain = false;
    data.willMessage->delayInterval = m_sessionExpiryInterval + 1000;   // the sessionExpiryInterval has priority

    m_client->startConnection(connection, data);
    m_client->subscribe(connection, { {{"/" + m_clientId + "/#", 2, false, true, 2}} });
    m_client->subscribe(connection, { {{"/fmq_willmessages", 2, false, true, 2}} });

    std::unique_lock<std::mutex> lock(m_mutex);
    m_connection = connection;

}

IStreamConnectionPtr ProtocolMqtt5::getConnection() const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_connection;
}

void ProtocolMqtt5::disconnect()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    IStreamConnectionPtr connection = m_connection;
    lock.unlock();
    if (connection)
    {
        connection->disconnect();
    }
}

std::uint32_t ProtocolMqtt5::getProtocolId() const
{
    return PROTOCOL_ID;
}

bool ProtocolMqtt5::areMessagesResendable() const 
{
    return false;
}

bool ProtocolMqtt5::doesSupportMetainfo() const 
{
    return true;
}

bool ProtocolMqtt5::doesSupportSession() const 
{
    return true;
}

bool ProtocolMqtt5::needsReply() const 
{
    return false;
}

bool ProtocolMqtt5::isMultiConnectionSession() const 
{
    return false;
}

bool ProtocolMqtt5::isSendRequestByPoll() const 
{
    return false;
}

bool ProtocolMqtt5::doesSupportFileTransfer() const 
{
    return false;
}

IProtocol::FuncCreateMessage ProtocolMqtt5::getMessageFactory() const
{
    return []() {
        return std::make_shared<ProtocolMessage>(PROTOCOL_ID);
    };
}

bool ProtocolMqtt5::sendMessage(IMessagePtr message)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    IStreamConnectionPtr connection = m_connection;
    lock.unlock();
    IMqtt5Client::PublishData data;
    data.qos = 2;
    std::string* corrid = message->getMetainfo(FMQ_CORRID);
    if (corrid)
    {
        data.correlationData = Bytes(corrid->begin(), corrid->end());
    }
    data.responseTopic = "/" + m_clientId;

    data.topic = '/';
    data.topic += message->getEchoData().getDataValue<std::string>(FMQ_VIRTUAL_SESSION_ID);

    if (data.topic.empty())
    {
        std::string* destname = message->getControlData().getData<std::string>(FMQ_DESTNAME);
        if (destname)
        {
            data.topic += *destname;
        }
    }

    if (data.topic.empty())
    {
        data.topic += "fmqevents/";
        data.topic += m_clientId;
    }

    std::string* type = message->getMetainfo(FMQ_TYPE);
    if (type && !type->empty())
    {
        data.topic += '/';
        data.topic += *type;
    }

    m_client->publish(connection, std::move(data), message);
    return true;
}

void ProtocolMqtt5::moveOldProtocolState(IProtocol& /*protocolOld*/) 
{
}

bool ProtocolMqtt5::received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead) 
{
    bool ok = m_client->receive(connection, socket, bytesToRead);
    return ok;
}

hybrid_ptr<IStreamConnectionCallback> ProtocolMqtt5::connected(const IStreamConnectionPtr& /*connection*/) 
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto callback = m_callback.lock();
    lock.unlock();
    if (callback)
    {
        callback->socketConnected();
    }
    return nullptr;
}

void ProtocolMqtt5::disconnected(const IStreamConnectionPtr& /*connection*/)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto callback = m_callback.lock();
    m_connection = nullptr;
    m_timerReconnect.setTimeout(5000);
    lock.unlock();

    if (callback)
    {
        callback->socketDisconnected();
    }
}

IMessagePtr ProtocolMqtt5::pollReply(std::deque<IMessagePtr>&& /*messages*/) 
{
    return {};
}

void ProtocolMqtt5::subscribe(const std::vector<std::string>& subscribtions)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    IMqtt5Client::SubscribeData data;
    data.subscriptions.reserve(subscribtions.size());
    for (size_t i = 0; i < subscribtions.size(); ++i)
    {
        std::string topic = "/" + subscribtions[i];
        data.subscriptions.push_back({ topic, 2, false, false, 2});
    }
    IStreamConnectionPtr connection = m_connection;
    lock.unlock();
    m_client->subscribe(connection, data);
}

void ProtocolMqtt5::cycleTime()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    IStreamConnectionPtr connection = m_connection;
    bool isReconnectTimerExpired = m_timerReconnect.isExpired();
    auto callback = m_callback.lock();
    lock.unlock();

    m_client->cycleTime(connection);

    if (isReconnectTimerExpired && callback)
    {
        callback->reconnect();
    }
}


// IMqtt5ClientCallback
void ProtocolMqtt5::receivedConnAck(const ConnAckData& data)
{
    if (data.reasoncode < 0x80)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        bool sessionGone = false;
        bool isFirstConnection = m_firstConnection;
        m_firstConnection = false;
        if (!isFirstConnection && !data.sessionPresent)
        {
            sessionGone = true;
        }
        else
        {
            m_keepAlive = data.serverKeepAlive;
            if (data.sessionExpiryInterval < m_sessionExpiryInterval)
            {
                m_sessionExpiryInterval = data.sessionExpiryInterval;
            }
        }
        IStreamConnectionPtr connection = m_connection;
        auto callback = m_callback.lock();
        lock.unlock();

        if (callback)
        {
            if (sessionGone)
            {
                IMqtt5Client::DisconnectData data = { ReasonCodeDisconnectWithWillMessage, "", {} };
                m_client->endConnection(connection, data);
                disconnect();
                callback->disconnected();
            }
            else
            {
                if (isFirstConnection)
                {
                    callback->connected();
                }
                callback->setActivityTimeout(m_keepAlive * 1500 + m_sessionExpiryInterval * 1000);
            }
        }
    }
    else
    {
        streamError << "Connection failed, reason code: " << (unsigned int)data.reasoncode << " (" << data.reasonString << ")";
    }
}

void ProtocolMqtt5::receivedPublish(const PublishData& data, const IMessagePtr& message)
{
    message->addMetainfo(FMQ_CORRID, std::string(data.correlationData.begin(), data.correlationData.end()));
    message->addMetainfo(FMQ_VIRTUAL_SESSION_ID, data.responseTopic);

    std::unique_lock<std::mutex> lock(m_mutex);
    auto callback = m_callback.lock();
    lock.unlock();

    if (callback)
    {
        callback->received(message);
    }
}

void ProtocolMqtt5::receivedSubAck(const std::vector<std::uint8_t>& /*reasoncodes*/)
{

}

void ProtocolMqtt5::receivedUnsubAck(const std::vector<std::uint8_t>& /*reasoncodes*/)
{

}

void ProtocolMqtt5::receivedPingResp()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto callback = m_callback.lock();
    lock.unlock();
    if (callback)
    {
        callback->activity();
    }
}

void ProtocolMqtt5::receivedDisconnect(const DisconnectData& data)
{
    streamInfo << "Disconnect from mqtt5 broker. Reason Code: " << (unsigned int)data.reasoncode << " (" << data.reasonString << ")";
}

void ProtocolMqtt5::receivedAuth(const AuthData& /*data*/)
{

}

void ProtocolMqtt5::closeConnection()
{
    disconnect();
}




//---------------------------------------
// ProtocolMqtt5Factory
//---------------------------------------

struct RegisterProtocolMqtt5Factory
{
    RegisterProtocolMqtt5Factory()
    {
        ProtocolRegistry::instance().registerProtocolFactory(ProtocolMqtt5::PROTOCOL_NAME, ProtocolMqtt5::PROTOCOL_ID, std::make_shared<ProtocolMqtt5Factory>());
    }
} g_registerProtocolMqtt5Factory;




// IProtocolFactory
IProtocolPtr ProtocolMqtt5Factory::createProtocol(const Variant& data)
{
    return std::make_shared<ProtocolMqtt5>(data);
}

}   // namespace finalmq
