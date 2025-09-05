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

#include "finalmq/protocols/protocolhelpers/ProtocolDelimiter.h"

#include "finalmq/protocolsession/ProtocolMessage.h"
#include "finalmq/streamconnection/Socket.h"

namespace finalmq
{
//---------------------------------------
// ProtocolDelimiter
//---------------------------------------

ProtocolDelimiter::ProtocolDelimiter(const std::string& delimiter)
    : m_delimiter(delimiter), m_delimiterStart((!delimiter.empty()) ? delimiter[0] : ' ')
{
    assert(!delimiter.empty());
}

ProtocolDelimiter::~ProtocolDelimiter()
{
    if (m_connection)
    {
        m_connection->disconnect();
    }
}

// IProtocol
void ProtocolDelimiter::setCallback(const std::weak_ptr<IProtocolCallback>& callback)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_callback = callback;
}

void ProtocolDelimiter::setConnection(const IStreamConnectionPtr& connection)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_connection = connection;
}

IStreamConnectionPtr ProtocolDelimiter::getConnection() const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_connection;
}

void ProtocolDelimiter::disconnect()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    IStreamConnectionPtr conn = m_connection;
    lock.unlock();
    if (conn)
    {
        conn->disconnect();
    }
}

bool ProtocolDelimiter::areMessagesResendable() const
{
    return true;
}

bool ProtocolDelimiter::doesSupportMetainfo() const
{
    return false;
}

bool ProtocolDelimiter::doesSupportSession() const
{
    return false;
}

bool ProtocolDelimiter::needsReply() const
{
    return false;
}

bool ProtocolDelimiter::isMultiConnectionSession() const
{
    return false;
}

bool ProtocolDelimiter::isSendRequestByPoll() const
{
    return false;
}

bool ProtocolDelimiter::doesSupportFileTransfer() const
{
    return false;
}

bool ProtocolDelimiter::isSynchronousRequestReply() const
{
    return false;
}

IProtocol::FuncCreateMessage ProtocolDelimiter::getMessageFactory() const
{
    size_t sizeDelimiter = m_delimiter.size();
    int protocolId = getProtocolId();
    return [protocolId, sizeDelimiter]() {
        return std::make_shared<ProtocolMessage>(protocolId, 0, sizeDelimiter);
    };
}

void ProtocolDelimiter::sendMessage(IMessagePtr message)
{
    if (message == nullptr)
    {
        return;
    }
    if (!message->wasSent())
    {
        const std::list<BufferRef>& buffers = message->getAllSendBuffers();
        if (!buffers.empty() && !m_delimiter.empty())
        {
            const BufferRef& buffer = *buffers.rbegin();
            assert(buffer.second >= static_cast<ssize_t>(m_delimiter.size()));
            memcpy(buffer.first + buffer.second - m_delimiter.size(), m_delimiter.data(), m_delimiter.size());
            message->prepareMessageToSend();
        }
    }
    assert(m_connection);
    m_connection->sendMessage(message);
}

void ProtocolDelimiter::moveOldProtocolState(IProtocol& /*protocolOld*/)
{
}

bool ProtocolDelimiter::received(const IStreamConnectionPtr& /*connection*/, const SocketPtr& socket, int bytesToRead)
{
    std::shared_ptr<std::string> receiveBufferOld = m_receiveBuffer;
    ssize_t sizeDelimiterPrefix = 0;
    ssize_t indexStart = 0;
    if (receiveBufferOld != nullptr)
    {
        indexStart = m_indexStartMessage;
        ssize_t indexEnd = m_bufferSize;
        sizeDelimiterPrefix = indexEnd - indexStart;
        m_indexStartMessage = 0;
    }

    m_receiveBuffer = std::make_shared<std::string>();
    m_receiveBuffer->resize(sizeDelimiterPrefix + bytesToRead);
    if (sizeDelimiterPrefix > 0)
    {
        memcpy(m_receiveBuffer->data(), receiveBufferOld->data() + indexStart, sizeDelimiterPrefix);
    }
    int res = socket->receive(m_receiveBuffer->data() + sizeDelimiterPrefix, bytesToRead);
    if (res > 0)
    {
        auto callback = m_callback.lock();
        const char* buffer = m_receiveBuffer->data();
        ssize_t bytesReceived = res;
        m_bufferSize = sizeDelimiterPrefix + bytesReceived;
        ssize_t offsetEnd = m_bufferSize - (m_delimiter.size() - 1);
        assert(bytesReceived <= bytesToRead);
        for (ssize_t i = m_indexStartMessage; i < offsetEnd; ++i)
        {
            const char c = buffer[i];
            if (c == m_delimiterStart)
            {
                bool match = true;
                for (ssize_t j = 1; j < static_cast<ssize_t>(m_delimiter.size()); ++j)
                {
                    if (buffer[i + j] != m_delimiter[j])
                    {
                        match = false;
                        break;
                    }
                }
                if (match)
                {
                    IMessagePtr message = std::make_shared<ProtocolMessage>(0);
                    if (m_receiveBuffers.empty())
                    {
                        ssize_t size = i - m_indexStartMessage;
                        assert(size >= 0);
                        message->setReceiveBuffer(m_receiveBuffer, m_indexStartMessage, size);
                    }
                    else
                    {
                        assert(m_indexStartMessage == 0);
                        ssize_t sizeLast = i - m_indexStartMessage;
                        assert(sizeLast >= 0);
                        m_receiveBuffersTotal += sizeLast;
                        assert(m_receiveBuffersTotal >= 0);
                        std::shared_ptr<std::string> receiveBufferHelper = std::make_shared<std::string>();
                        receiveBufferHelper->resize(m_receiveBuffersTotal);
                        ssize_t offset = 0;
                        for (size_t n = 0; n < m_receiveBuffers.size(); ++n)
                        {
                            const auto& receiveBufferStore = m_receiveBuffers[n];
                            ssize_t sizeCopy = receiveBufferStore.indexEndMessage - receiveBufferStore.indexStartMessage;
                            memcpy(receiveBufferHelper->data() + offset,
                                   receiveBufferStore.receiveBuffer->data() + receiveBufferStore.indexStartMessage,
                                   sizeCopy);
                            offset += sizeCopy;
                        }
                        memcpy(receiveBufferHelper->data() + offset, m_receiveBuffer->data(), sizeLast);
                        message->setReceiveBuffer(receiveBufferHelper, 0, m_receiveBuffersTotal);
                        m_receiveBuffersTotal = 0;
                        m_receiveBuffers.clear();
                    }
                    if (callback)
                    {
                        callback->received(message);
                    }
                    i += m_delimiter.size();
                    m_indexStartMessage = i;
                    --i;
                }
            }
        }
        if (m_indexStartMessage == m_bufferSize)
        {
            m_receiveBuffer = nullptr;
            m_bufferSize = 0;
            m_indexStartMessage = 0;
        }
        else
        {
            if (m_indexStartMessage < offsetEnd)
            {
                m_receiveBuffers.emplace_back(ReceiveBufferStore{m_receiveBuffer, m_indexStartMessage, offsetEnd});
                m_receiveBuffersTotal += offsetEnd - m_indexStartMessage;
                m_indexStartMessage = offsetEnd;
            }
        }
    }
    return (res > 0);
}

hybrid_ptr<IStreamConnectionCallback> ProtocolDelimiter::connected(const IStreamConnectionPtr& /*connection*/)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->connected();
    }
    return nullptr;
}

void ProtocolDelimiter::disconnected(const IStreamConnectionPtr& /*connection*/)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->disconnected();
    }
}

IMessagePtr ProtocolDelimiter::pollReply(std::deque<IMessagePtr>&& /*messages*/)
{
    return {};
}

void ProtocolDelimiter::subscribe(const std::vector<std::string>& /*subscribtions*/)
{
}

void ProtocolDelimiter::cycleTime()
{
}

IProtocolSessionDataPtr ProtocolDelimiter::createProtocolSessionData()
{
    return nullptr;
}

void ProtocolDelimiter::setProtocolSessionData(const IProtocolSessionDataPtr& /*protocolSessionData*/)
{
}

} // namespace finalmq
