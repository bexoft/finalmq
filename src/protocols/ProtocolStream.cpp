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


#include "finalmq/protocols/ProtocolStream.h"
#include "finalmq/protocolsession/ProtocolMessage.h"
#include "finalmq/protocolsession/ProtocolRegistry.h"
#include "finalmq/streamconnection/Socket.h"

#include <assert.h>

namespace finalmq {

//---------------------------------------
// ProtocolStream
//---------------------------------------

const int ProtocolStream::PROTOCOL_ID = 1;
const std::string ProtocolStream::PROTOCOL_NAME = "stream";


ProtocolStream::ProtocolStream()
{

}


// IProtocol
void ProtocolStream::setCallback(const std::weak_ptr<IProtocolCallback>& callback)
{
    m_callback = callback;
}

void ProtocolStream::setConnection(const IStreamConnectionPtr& connection)
{
    m_connection = connection;
}

void ProtocolStream::disconnect()
{
    assert(m_connection);
    m_connection->disconnect();
}

std::uint32_t ProtocolStream::getProtocolId() const
{
    return PROTOCOL_ID;
}

bool ProtocolStream::areMessagesResendable() const
{
    return true;
}

bool ProtocolStream::doesSupportMetainfo() const
{
    return false;
}

bool ProtocolStream::doesSupportSession() const
{
    return false;
}

bool ProtocolStream::needsReply() const
{
    return false;
}

bool ProtocolStream::isMultiConnectionSession() const
{
    return false;
}

bool ProtocolStream::isSendRequestByPoll() const
{
    return false;
}

bool ProtocolStream::doesSupportFileTransfer() const
{
    return false;
}

IProtocol::FuncCreateMessage ProtocolStream::getMessageFactory() const
{
    return []() {
        return std::make_shared<ProtocolMessage>(PROTOCOL_ID);
    };
}

bool ProtocolStream::sendMessage(IMessagePtr message)
{
    if (!message->wasSent())
    {
        message->prepareMessageToSend();
    }
    assert(m_connection);
    return m_connection->sendMessage(message);
}

void ProtocolStream::moveOldProtocolState(IProtocol& /*protocolOld*/)
{

}


bool ProtocolStream::received(const IStreamConnectionPtr& /*connection*/, const SocketPtr& socket, int bytesToRead)
{
    IMessagePtr message = std::make_shared<ProtocolMessage>(0);
    char* payload = message->resizeReceiveBuffer(bytesToRead);
    int res = socket->receive(payload, bytesToRead);
    if (res > 0)
    {
        int bytesReceived = res;
        assert(bytesReceived <= bytesToRead);
        message->resizeReceiveBuffer(bytesReceived);
        auto callback = m_callback.lock();
        if (callback)
        {
            callback->received(message);
        }
    }
    return true;
}


hybrid_ptr<IStreamConnectionCallback> ProtocolStream::connected(const IStreamConnectionPtr& /*connection*/)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->connected();
    }
    return nullptr;
}

void ProtocolStream::disconnected(const IStreamConnectionPtr& /*connection*/)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->disconnected();
    }
}


IMessagePtr ProtocolStream::pollReply(std::deque<IMessagePtr>&& /*messages*/)
{
    return {};
}

void ProtocolStream::cycleTime()
{

}



//---------------------------------------
// ProtocolStreamFactory
//---------------------------------------

struct RegisterProtocolStreamFactory
{
    RegisterProtocolStreamFactory()
    {
        ProtocolRegistry::instance().registerProtocolFactory(ProtocolStream::PROTOCOL_NAME, ProtocolStream::PROTOCOL_ID, std::make_shared<ProtocolStreamFactory>());
    }
} g_registerProtocolStreamFactory;




// IProtocolFactory
IProtocolPtr ProtocolStreamFactory::createProtocol()
{
    return std::make_shared<ProtocolStream>();
}

}   // namespace finalmq
