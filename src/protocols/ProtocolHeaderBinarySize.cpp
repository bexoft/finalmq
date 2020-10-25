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


#include "protocols/ProtocolHeaderBinarySize.h"
#include "protocolconnection/ProtocolMessage.h"
#include "streamconnection/Socket.h"

#include <atomic>

namespace finalmq {


static const int HEADERSIZE = 4;


//---------------------------------------
// ProtocolHeaderBinarySize
//---------------------------------------

static std::atomic<std::int64_t> g_protocolInstanceIdNext(1);


ProtocolHeaderBinarySize::ProtocolHeaderBinarySize()
    : m_headerHelper(HEADERSIZE, [] (const std::string& header) {
            assert(header.size() == 4);
            int sizePayload = 0;
            for (int i = 0; i < 4; ++i)
            {
                sizePayload += (int)header[i] << (8 * i);
            }
            return sizePayload;
      })
{

}


// IProtocol
void ProtocolHeaderBinarySize::setCallback(const std::weak_ptr<IProtocolCallback>& callback)
{
    m_callback = callback;
}

int ProtocolHeaderBinarySize::getProtocolId() const
{
    return m_protocolId;
}

bool ProtocolHeaderBinarySize::areMessagesResendable() const
{
    return true;
}

IMessagePtr ProtocolHeaderBinarySize::createMessage() const
{
    return std::make_shared<ProtocolMessage>(m_protocolId, HEADERSIZE);
}

void ProtocolHeaderBinarySize::receive(const SocketPtr& socket, int bytesToRead)
{
    std::vector<IMessagePtr> messages = m_headerHelper.receive(socket, bytesToRead);
    auto callback = m_callback.lock();
    if (callback)
    {
        for (size_t i = 0; i < messages.size(); ++i)
        {
            callback->received(messages[i]);
        }
    }
}

void ProtocolHeaderBinarySize::prepareMessageToSend(IMessagePtr message)
{
    if (!message->wasSent())
    {
        int sizePayload = message->getTotalSendPayloadSize();
        const std::list<BufferRef>& buffers = message->getAllSendBuffers();
        assert(!buffers.empty());
        assert(buffers.begin()->second >= 4);
        char* header = buffers.begin()->first;
        for (int i = 0; i < 4; ++i)
        {
            header[i] = (sizePayload >> (i * 8)) & 0xff;
        }
        message->prepareMessageToSend();
    }
}

void ProtocolHeaderBinarySize::socketConnected()
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->connected();
    }
}

void ProtocolHeaderBinarySize::socketDisconnected()
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->disconnected();
    }
}



//---------------------------------------
// ProtocolHeaderBinarySizeFactory
//---------------------------------------



// IProtocolFactory
IProtocolPtr ProtocolHeaderBinarySizeFactory::createProtocol()
{
    return std::make_shared<ProtocolHeaderBinarySize>();
}

}   // namespace finalmq
