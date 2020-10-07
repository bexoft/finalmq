
#include "protocols/ProtocolHeaderBinarySize.h"
#include "protocolconnection/ProtocolMessage.h"
#include "streamconnection/Socket.h"

#include <atomic>

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

