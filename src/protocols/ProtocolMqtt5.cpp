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


//!!! todo: the receivedConnAck callback has serverKeepAlive and sessionExpiryInterval, use these parameters for calling setActivityTimeout with: (serverKeepAlive*1000 + serverKeepAlive*1500)


namespace finalmq {

const int ProtocolMqtt5::PROTOCOL_ID = 5;
const std::string ProtocolMqtt5::PROTOCOL_NAME = "mqtt5";


ProtocolMqtt5::ProtocolMqtt5()
    : m_client(std::make_unique<Mqtt5Client>())
{
}




// IProtocol
void ProtocolMqtt5::setCallback(const std::weak_ptr<IProtocolCallback>& callback)
{
    m_callback = callback;
}

void ProtocolMqtt5::setConnection(const IStreamConnectionPtr& connection)
{
    IMqtt5Client::ConnectData data;
    m_client->startConnection(connection, data);

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
    assert(m_connection);
    m_connection->disconnect();
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

bool ProtocolMqtt5::sendMessage(IMessagePtr /*message*/) 
{
    return false;
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
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->connected();
    }
    return nullptr;
}

void ProtocolMqtt5::disconnected(const IStreamConnectionPtr& connection)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->disconnected();
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    m_connection = nullptr;
}

IMessagePtr ProtocolMqtt5::pollReply(std::deque<IMessagePtr>&& /*messages*/) 
{
    return {};
}

void ProtocolMqtt5::cycleTime()
{

}


// IMqtt5ClientCallback
void ProtocolMqtt5::receivedConnAck(const ConnAckData& data)
{

}

void ProtocolMqtt5::receivedPublish(const PublishData& data, const IMessagePtr& message)
{

}

void ProtocolMqtt5::receivedSubAck(const std::vector<std::uint8_t>& reasoncodes)
{

}

void ProtocolMqtt5::receivedUnsubAck(const std::vector<std::uint8_t>& reasoncodes)
{

}

void ProtocolMqtt5::receivedPingResp()
{

}

void ProtocolMqtt5::receivedDisconnect(const DisconnectData& data)
{

}

void ProtocolMqtt5::receivedAuth(const AuthData& data)
{

}

void ProtocolMqtt5::closeConnection()
{

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
IProtocolPtr ProtocolMqtt5Factory::createProtocol()
{
    return std::make_shared<ProtocolMqtt5>();
}

}   // namespace finalmq
