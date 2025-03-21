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

#include "finalmq/protocols/ProtocolMqtt5Client.h"

#include "finalmq/helpers/ModulenameFinalmq.h"
#include "finalmq/protocolsession/ProtocolMessage.h"
#include "finalmq/protocolsession/ProtocolRegistry.h"
#include "finalmq/streamconnection/Socket.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValues.h"

#ifdef WIN32
#include <rpcdce.h>
#else
#ifdef __QNX__
//#include <uuid.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#if defined(USE_OPENSSL)
#include <openssl/rand.h>
#endif
#else
#include <uuid/uuid.h>
#endif
#endif

namespace finalmq
{
const std::uint32_t ProtocolMqtt5Client::PROTOCOL_ID = 5;
const std::string ProtocolMqtt5Client::PROTOCOL_NAME = "mqtt5client";

const std::string ProtocolMqtt5Client::KEY_USERNAME = "username";
const std::string ProtocolMqtt5Client::KEY_PASSWORD = "password";
const std::string ProtocolMqtt5Client::KEY_SESSIONEXPIRYINTERVAL = "sessionexpiryinterval";
const std::string ProtocolMqtt5Client::KEY_KEEPALIVE = "keepalive";
const std::string ProtocolMqtt5Client::KEY_TOPIC_PREFIX = "topicprefix";

static const std::string FMQ_PATH = "fmq_path";
static const std::string FMQ_CORRID = "fmq_corrid";
static const std::string FMQ_TYPE = "fmq_type";
static const std::string FMQ_VIRTUAL_SESSION_ID = "fmq_virtsessid";
static const std::string FMQ_DESTNAME = "fmq_destname";
static const std::string FMQ_SUBPATH = "fmq_subpath";

static const std::string TOPIC_WILLMESSAGE = "/fmq_willmsg";
static const std::string SESSIONID_PREFIX = "/_id/";

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
#ifdef __QNX__
//static std::string getUuid()
//{
//	uuid_t* uuid = nullptr;
//	uuid_create(&uuid);
//	char buffer[50];     // 37 is the exact number
//	size_t sizeStrUuid = sizeof(buffer);
//	uuid_export(uuid, UUID_FMT_STR, buffer, &sizeStrUuid);
//	std::string strUuidRet(buffer, sizeStrUuid);
//	uuid_destroy(uuid);
//	return strUuidRet;
//}

/** @brief raw uuid type */
typedef unsigned char uuid_t[16];

/**
 * @brief generates a uuid, compatible with RFC 4122, version 4 (random)
 * @note Uses a very insecure algorithm but no external dependencies
 */
void uuid_generate(uuid_t out)
{
#if defined(USE_OPENSSL)
    int rc = RAND_bytes(out, sizeof(uuid_t));
    if (!rc)
#endif
    {
        /* very insecure, but generates a random uuid */
        int i;
        srand(time(NULL));
        for (i = 0; i < 16; ++i)
            out[i] = (unsigned char)(rand() % UCHAR_MAX);
        out[6] = (out[6] & 0x0f) | 0x40;
        out[8] = (out[8] & 0x3F) | 0x80;
    }
}

/** @brief converts a uuid to a string */
void uuid_unparse(uuid_t uu, char* out)
{
    int i;
    for (i = 0; i < 16; ++i)
    {
        if (i == 4 || i == 6 || i == 8 || i == 10)
        {
            *out = '-';
            ++out;
        }
        out += sprintf(out, "%02x", uu[i]);
    }
    *out = '\0';
}
static std::string getUuid()
{
    std::string strUuid;
    strUuid.resize(50); // 37 is the exact number
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, const_cast<char*>(strUuid.data()));
    std::string strUuidRet = strUuid.data();
    return strUuidRet;
}
#else
static std::string getUuid()
{
    std::string strUuid;
    strUuid.resize(50); // 37 is the exact number
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, const_cast<char*>(strUuid.data()));
    std::string strUuidRet = strUuid.data();
    return strUuidRet;
}
#endif
#endif

ProtocolMqtt5Client::ProtocolMqtt5Client(const Variant& data)
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
    m_topicPrefix = data.getDataValue<std::string>(KEY_TOPIC_PREFIX);
    if (!m_topicPrefix.empty())
    {
        if (m_topicPrefix[0] != '/')
        {
            m_topicPrefix = "/" + m_topicPrefix;
        }
    }
    m_clientId = getUuid();
    m_sessionPrefix = m_topicPrefix + '/' + SESSIONID_PREFIX;
    m_virtualSessionId = m_sessionPrefix + m_clientId;

    m_client->setCallback(this);
}

ProtocolMqtt5Client::~ProtocolMqtt5Client()
{
    if (m_connection)
    {
        m_connection->disconnect();
    }
}

// IProtocol
void ProtocolMqtt5Client::setCallback(const std::weak_ptr<IProtocolCallback>& callback)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_callback = callback;
}

void ProtocolMqtt5Client::setConnection(const IStreamConnectionPtr& connection)
{
    IMqtt5Client::ConnectData data;
    data.cleanStart = m_firstConnection;
    data.clientId = m_clientId;
    data.receiveMaximum = 128;
    data.username = m_username;
    data.password = m_password;
    data.keepAlive = static_cast<std::uint16_t>(m_keepAlive);
    data.sessionExpiryInterval = m_sessionExpiryInterval;
    data.willMessage = std::make_unique<IMqtt5Client::WillMessage>();
    data.willMessage->payload.insert(data.willMessage->payload.end(), m_virtualSessionId.begin(), m_virtualSessionId.end());
    data.willMessage->qos = 2;
    data.willMessage->topic = TOPIC_WILLMESSAGE;
    data.willMessage->retain = false;
    data.willMessage->delayInterval = m_sessionExpiryInterval + 1000; // the sessionExpiryInterval has priority

    m_client->startConnection(connection, data);
    m_client->subscribe(connection, {{{m_virtualSessionId + "/#", 2, false, true, 2}}});
    m_client->subscribe(connection, {{{TOPIC_WILLMESSAGE, 2, false, true, 2}}});

    std::unique_lock<std::mutex> lock(m_mutex);
    m_connection = connection;
}

IStreamConnectionPtr ProtocolMqtt5Client::getConnection() const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_connection;
}

void ProtocolMqtt5Client::disconnect()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    IStreamConnectionPtr connection = m_connection;
    lock.unlock();

    // send will message, manually, because ReasonCodeDisconnectWithWillMessage does not really work
    IMqtt5Client::PublishData dataWill;
    dataWill.qos = 0;
    dataWill.topic = TOPIC_WILLMESSAGE;
    IMessagePtr message = std::make_shared<ProtocolMessage>(0);
    message->addSendPayload(m_virtualSessionId);
    m_client->publish(connection, std::move(dataWill), message);

    IMqtt5Client::DisconnectData data = {ReasonCodeDisconnectWithWillMessage, "", {}};
    m_client->endConnection(connection, data);

    if (connection)
    {
        connection->disconnect();
    }
}

std::uint32_t ProtocolMqtt5Client::getProtocolId() const
{
    return PROTOCOL_ID;
}

bool ProtocolMqtt5Client::areMessagesResendable() const
{
    return false;
}

bool ProtocolMqtt5Client::doesSupportMetainfo() const
{
    return true;
}

bool ProtocolMqtt5Client::doesSupportSession() const
{
    return true;
}

bool ProtocolMqtt5Client::needsReply() const
{
    return false;
}

bool ProtocolMqtt5Client::isMultiConnectionSession() const
{
    return false;
}

bool ProtocolMqtt5Client::isSendRequestByPoll() const
{
    return false;
}

bool ProtocolMqtt5Client::doesSupportFileTransfer() const
{
    return false;
}

bool ProtocolMqtt5Client::isSynchronousRequestReply() const
{
    return false;
}

IProtocol::FuncCreateMessage ProtocolMqtt5Client::getMessageFactory() const
{
    return []() {
        return std::make_shared<ProtocolMessage>(PROTOCOL_ID);
    };
}

void ProtocolMqtt5Client::sendMessage(IMessagePtr message)
{
    if (message == nullptr)
    {
        return;
    }
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
    data.responseTopic = m_virtualSessionId;

    std::string topic = message->getControlData().getDataValue<std::string>(FMQ_VIRTUAL_SESSION_ID);

    std::string* destname = message->getControlData().getData<std::string>(FMQ_DESTNAME);
    if (topic.empty() && destname && !destname->empty())
    {
        topic = m_topicPrefix;

        if ((*destname)[0] != '/')
        {
            topic += '/';
        }
        topic += *destname;

        std::string* path = message->getMetainfo(FMQ_SUBPATH);
        if (path && !path->empty())
        {
            if ((*path)[0] != '/')
            {
                topic += '/';
            }
            topic += *path;
        }
        else
        {
            std::string* type = message->getMetainfo(FMQ_TYPE);
            if (type && !type->empty())
            {
                topic += '/';
                topic += *type;
            }
        }
    }

    if (!topic.empty())
    {
        data.topic = std::move(topic);
        m_client->publish(connection, std::move(data), message);
    }
}

void ProtocolMqtt5Client::moveOldProtocolState(IProtocol& /*protocolOld*/)
{
}

bool ProtocolMqtt5Client::received(const IStreamConnectionPtr& connection, const SocketPtr& socket, int bytesToRead)
{
    bool ok = m_client->receive(connection, socket, bytesToRead);
    return ok;
}

hybrid_ptr<IStreamConnectionCallback> ProtocolMqtt5Client::connected(const IStreamConnectionPtr& /*connection*/)
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

void ProtocolMqtt5Client::disconnected(const IStreamConnectionPtr& /*connection*/)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto callback = m_callback.lock();
    m_connection = nullptr;
    m_timerReconnect.setTimeout(1000);
    lock.unlock();

    if (callback)
    {
        callback->socketDisconnected();
    }
}

IMessagePtr ProtocolMqtt5Client::pollReply(std::deque<IMessagePtr>&& /*messages*/)
{
    return {};
}

void ProtocolMqtt5Client::subscribe(const std::vector<std::string>& subscribtions)
{
    if (subscribtions.empty())
    {
        return;
    }
    IMqtt5Client::SubscribeData data;
    data.subscriptions.reserve(subscribtions.size());
    for (size_t i = 0; i < subscribtions.size(); ++i)
    {
        const std::string& subscription = subscribtions[i];
        std::string topic = m_topicPrefix + '/' + subscription;
        data.subscriptions.push_back({topic, 2, false, false, 2});
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    IStreamConnectionPtr connection = m_connection;
    lock.unlock();
    m_client->subscribe(connection, data);
}

void ProtocolMqtt5Client::cycleTime()
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
void ProtocolMqtt5Client::receivedConnAck(const ConnAckData& data)
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
        //        IStreamConnectionPtr connection = m_connection;
        auto callback = m_callback.lock();
        lock.unlock();

        if (callback)
        {
            if (sessionGone)
            {
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
        streamError << "Connection failed, reason code: " << static_cast<unsigned int>(data.reasoncode) << " (" << data.reasonString << ")";
    }
}

void ProtocolMqtt5Client::receivedPublish(const PublishData& data, const IMessagePtr& message)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto callback = m_callback.lock();
    lock.unlock();

    if (callback)
    {
        if (data.topic == TOPIC_WILLMESSAGE)
        {
            BufferRef buffer = message->getReceivePayload();
            std::string virtualSessionId(buffer.first, buffer.first + buffer.second);
            callback->disconnectedVirtualSession(virtualSessionId);
        }
        else
        {
            if (data.topic.compare(0, m_sessionPrefix.size(), m_sessionPrefix.c_str()) == 0)
            {
                size_t pos = data.topic.find_first_of('/', m_sessionPrefix.size());
                if (pos != std::string::npos)
                {
                    std::string path = &data.topic[pos];
                    message->addMetainfo(FMQ_PATH, std::move(path));
                }
            }
            else if (data.topic.compare(0, m_topicPrefix.size(), m_topicPrefix.c_str()) == 0)
            {
                std::string path = &data.topic[m_topicPrefix.size()];
                message->addMetainfo(FMQ_PATH, std::move(path));
            }
            else
            {
                message->addMetainfo(FMQ_PATH, data.topic);
            }
            message->addMetainfo(FMQ_CORRID, std::string(data.correlationData.begin(), data.correlationData.end()));
            message->addMetainfo(FMQ_VIRTUAL_SESSION_ID, data.responseTopic);
            callback->received(message);
        }
    }
}

void ProtocolMqtt5Client::receivedSubAck(const std::vector<std::uint8_t>& /*reasoncodes*/)
{
}

void ProtocolMqtt5Client::receivedUnsubAck(const std::vector<std::uint8_t>& /*reasoncodes*/)
{
}

void ProtocolMqtt5Client::receivedPingResp()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto callback = m_callback.lock();
    lock.unlock();
    if (callback)
    {
        callback->activity();
    }
}

void ProtocolMqtt5Client::receivedDisconnect(const DisconnectData& data)
{
    streamInfo << "Disconnect from mqtt5 broker. Reason Code: " << static_cast<unsigned int>(data.reasoncode) << " (" << data.reasonString << ")";
}

void ProtocolMqtt5Client::receivedAuth(const AuthData& /*data*/)
{
}

void ProtocolMqtt5Client::closeConnection()
{
    disconnect();
}

IProtocolSessionDataPtr ProtocolMqtt5Client::createProtocolSessionData()
{
    return nullptr;
}

void ProtocolMqtt5Client::setProtocolSessionData(const IProtocolSessionDataPtr& /*protocolSessionData*/)
{
}

//---------------------------------------
// ProtocolMqtt5Factory
//---------------------------------------

struct RegisterProtocolMqtt5ClientFactory
{
    RegisterProtocolMqtt5ClientFactory()
    {
        ProtocolRegistry::instance().registerProtocolFactory(ProtocolMqtt5Client::PROTOCOL_NAME, ProtocolMqtt5Client::PROTOCOL_ID, std::make_shared<ProtocolMqtt5ClientFactory>());
    }
} g_registerProtocolMqtt5ClientFactory;

// IProtocolFactory
IProtocolPtr ProtocolMqtt5ClientFactory::createProtocol(const Variant& data)
{
    return std::make_shared<ProtocolMqtt5Client>(data);
}

} // namespace finalmq
