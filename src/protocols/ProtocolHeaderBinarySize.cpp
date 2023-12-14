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


#include "finalmq/protocols/ProtocolHeaderBinarySize.h"
#include "finalmq/protocolsession/ProtocolMessage.h"
#include "finalmq/protocolsession/ProtocolRegistry.h"
#include "finalmq/streamconnection/Socket.h"

#include <cassert>

namespace finalmq {

const std::uint32_t ProtocolHeaderBinarySize::PROTOCOL_ID = 2;
const std::string ProtocolHeaderBinarySize::PROTOCOL_NAME = "headersize";


static const int HEADERSIZE = 4;


//---------------------------------------
// ProtocolHeaderBinarySize
//---------------------------------------


ProtocolHeaderBinarySize::ProtocolHeaderBinarySize()
    : m_headerHelper(HEADERSIZE, [] (const std::string& header) {
            assert(header.size() == 4);
            int sizePayload = 0;
            for (int i = 0; i < 4; ++i)
            {
                sizePayload |= ((unsigned int)header[i] & 0xff) << (8 * i);
            }
            return sizePayload;
      })
{

}


ProtocolHeaderBinarySize::~ProtocolHeaderBinarySize()
{
    if (m_connection)
    {
        m_connection->disconnect();
    }
}


// IProtocol
void ProtocolHeaderBinarySize::setCallback(const std::weak_ptr<IProtocolCallback>& callback)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_callback = callback;
}

void ProtocolHeaderBinarySize::setConnection(const IStreamConnectionPtr& connection)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_connection = connection;
}

IStreamConnectionPtr ProtocolHeaderBinarySize::getConnection() const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_connection;
}

void ProtocolHeaderBinarySize::disconnect()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    assert(m_connection);
    m_connection->disconnect();
}

std::uint32_t ProtocolHeaderBinarySize::getProtocolId() const
{
    return PROTOCOL_ID;
}

bool ProtocolHeaderBinarySize::areMessagesResendable() const
{
    return true;
}

bool ProtocolHeaderBinarySize::doesSupportMetainfo() const
{
    return false;
}

bool ProtocolHeaderBinarySize::doesSupportSession() const
{
    return false;
}

bool ProtocolHeaderBinarySize::needsReply() const
{
    return false;
}

bool ProtocolHeaderBinarySize::isMultiConnectionSession() const
{
    return false;
}

bool ProtocolHeaderBinarySize::isSendRequestByPoll() const
{
    return false;
}

bool ProtocolHeaderBinarySize::doesSupportFileTransfer() const
{
    return false;
}

bool ProtocolHeaderBinarySize::isSynchronousRequestReply() const
{
    return false;
}

IProtocol::FuncCreateMessage ProtocolHeaderBinarySize::getMessageFactory() const
{
    return []() {
        return std::make_shared<ProtocolMessage>(PROTOCOL_ID, HEADERSIZE);
    };
}

void ProtocolHeaderBinarySize::sendMessage(IMessagePtr message)
{
    if (message == nullptr)
    {
        return;
    }
    if (!message->wasSent())
    {
        ssize_t sizePayload = message->getTotalSendPayloadSize();
        const std::list<BufferRef>& buffers = message->getAllSendBuffers();
        assert(!buffers.empty());
        assert(buffers.begin()->second >= HEADERSIZE);
        char* header = buffers.begin()->first;
        for (int i = 0; i < HEADERSIZE; ++i)
        {
            header[i] = (sizePayload >> (i * 8)) & 0xff;
        }
        message->prepareMessageToSend();
    }
    assert(m_connection);
    m_connection->sendMessage(message);
}

void ProtocolHeaderBinarySize::moveOldProtocolState(IProtocol& /*protocolOld*/)
{

}

bool ProtocolHeaderBinarySize::received(const IStreamConnectionPtr& /*connection*/, const SocketPtr& socket, int bytesToRead)
{
    std::deque<IMessagePtr> messages;
    bool ok = m_headerHelper.receive(socket, bytesToRead, messages);
    auto callback = m_callback.lock();
    if (callback)
    {
        for (const auto &message : messages)
        {
            callback->received(message);
        }
    }
    return ok;
}


hybrid_ptr<IStreamConnectionCallback> ProtocolHeaderBinarySize::connected(const IStreamConnectionPtr& /*connection*/)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->connected();
    }
    return nullptr;
}

void ProtocolHeaderBinarySize::disconnected(const IStreamConnectionPtr& /*connection*/)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->disconnected();
    }
}


IMessagePtr ProtocolHeaderBinarySize::pollReply(std::deque<IMessagePtr>&& /*messages*/)
{
    return {};
}

void ProtocolHeaderBinarySize::subscribe(const std::vector<std::string>& /*subscribtions*/)
{

}

void ProtocolHeaderBinarySize::cycleTime()
{

}

IProtocolSessionDataPtr ProtocolHeaderBinarySize::createProtocolSessionData()
{
    return nullptr;
}

void ProtocolHeaderBinarySize::setProtocolSessionData(const IProtocolSessionDataPtr& /*protocolSessionData*/)
{
}



//---------------------------------------
// ProtocolHeaderBinarySizeFactory
//---------------------------------------

struct RegisterProtocolHeaderBinarySizeFactory
{
    RegisterProtocolHeaderBinarySizeFactory()
    {
        ProtocolRegistry::instance().registerProtocolFactory(ProtocolHeaderBinarySize::PROTOCOL_NAME, ProtocolHeaderBinarySize::PROTOCOL_ID, std::make_shared<ProtocolHeaderBinarySizeFactory>());
    }
} g_registerProtocolHeaderBinarySizeFactory;



// IProtocolFactory
IProtocolPtr ProtocolHeaderBinarySizeFactory::createProtocol(const Variant& /*data*/)
{
    return std::make_shared<ProtocolHeaderBinarySize>();
}



}   // namespace finalmq
