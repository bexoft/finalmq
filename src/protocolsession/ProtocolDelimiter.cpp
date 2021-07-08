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


#include "finalmq/protocolsession/ProtocolDelimiter.h"
#include "finalmq/protocolsession/ProtocolMessage.h"
#include "finalmq/streamconnection/Socket.h"


namespace finalmq {

//---------------------------------------
// ProtocolDelimiter
//---------------------------------------


ProtocolDelimiter::ProtocolDelimiter(const std::string& delimiter)
    : m_delimiter(delimiter)
{
}


// IProtocol
void ProtocolDelimiter::setCallback(const std::weak_ptr<IProtocolCallback>& callback)
{
    m_callback = callback;
}

void ProtocolDelimiter::setConnection(const IStreamConnectionPtr& connection)
{
    m_connection = connection;
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

IProtocol::FuncCreateMessage ProtocolDelimiter::getMessageFactory() const
{
    size_t sizeDelimiter = m_delimiter.size();
    int protocolId = getProtocolId();
    return [protocolId, sizeDelimiter]() {
        return std::make_shared<ProtocolMessage>(protocolId, 0, sizeDelimiter);
    };
}

std::vector<ssize_t> ProtocolDelimiter::findEndOfMessage(const char* buffer, ssize_t size)
{
    if (m_delimiter.empty())
    {
        return {size};
    }

    ssize_t sizeDelimiterPartial = m_delimiterPartial.size();

    std::vector<ssize_t> positions;
    ssize_t indexStartBuffer = 0;
    // was the delimiter not finished in last buffer?
    if (m_indexDelimiter != -1)
    {
        ssize_t a = 0;
        ssize_t b = m_indexDelimiter;
        for ( ; a < size && b < static_cast<ssize_t>(m_delimiter.size()); ++a, ++b)
        {
            if (buffer[a] != m_delimiter[b])
            {
                break;
            }
        }
        if (b == static_cast<ssize_t>(m_delimiter.size()))
        {
            positions.push_back(0 - m_indexDelimiter - sizeDelimiterPartial);
            indexStartBuffer = a;
            m_indexDelimiter = -1;
            m_delimiterPartial.clear();
        }
        else if (a == size)
        {
            m_indexDelimiter = b;
            indexStartBuffer = a;
            m_delimiterPartial += buffer;
        }
        else
        {
            // missmatch
            m_delimiterPartial.clear();
        }
    }

    char c = m_delimiter[0];
    for (ssize_t i = indexStartBuffer; i < size; ++i)
    {
        m_delimiterPartial.clear();
        if (buffer[i] == c)
        {
            ssize_t a = i + 1;
            size_t b = 1;
            for ( ; a < size && b < m_delimiter.size(); ++a, ++b)
            {
                if (buffer[a] != m_delimiter[b])
                {
                    break;
                }
            }
            if (b == m_delimiter.size())
            {
                positions.push_back(i - sizeDelimiterPartial);
                i = a - 1;  // -1, because ot the ++i in the for loop
            }
            else if (a == size)
            {
                m_indexDelimiter = b;
                m_delimiterPartial = std::string(&buffer[i], a - i);
                break;
            }
        }
    }
    return positions;
}




bool ProtocolDelimiter::sendMessage(IMessagePtr message)
{
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
    return m_connection->sendMessage(message);
}


void ProtocolDelimiter::moveOldProtocolState(IProtocol& /*protocolOld*/)
{

}



void ProtocolDelimiter::received(const IStreamConnectionPtr& /*connection*/, const SocketPtr& socket, int bytesToRead)
{
    std::string receiveBuffer;
    ssize_t sizeDelimiterPartial = m_delimiterPartial.size();
    receiveBuffer.resize(sizeDelimiterPartial + bytesToRead);
    if (sizeDelimiterPartial > 0)
    {
        memcpy(const_cast<char*>(receiveBuffer.data()), &m_delimiterPartial[0], sizeDelimiterPartial);
    }
    ssize_t bytesReceived = 0;
    int res = 0;
    do
    {
        res = socket->receive(const_cast<char*>(receiveBuffer.data() + bytesReceived + sizeDelimiterPartial), static_cast<int>(bytesToRead - bytesReceived));
        if (res > 0)
        {
            bytesReceived += res;
        }
    } while (res > 0 && bytesReceived < bytesToRead);
    if (res >= 0)
    {
        assert(bytesReceived <= bytesToRead);
        ssize_t sizeOfReceiveBuffer = sizeDelimiterPartial + bytesReceived;
        std::vector<ssize_t> positions = findEndOfMessage(receiveBuffer.data(), sizeOfReceiveBuffer);
        if (!positions.empty())
        {
            ssize_t pos = positions[0];
            IMessagePtr message = std::make_shared<ProtocolMessage>(0);
            message->resizeReceivePayload(m_characterCounter + pos);
            m_characterCounter += bytesReceived;
            assert(m_characterCounter >= 0);

            ssize_t offset = 0;

            if (pos < 0)
            {
                for (auto it = m_receiveBuffers.begin(); it != m_receiveBuffers.end(); )
                {
                    const std::string& buffer = *it;
                    ++it;
                    ssize_t size = buffer.size() - m_indexStartBuffer;
                    if (it == m_receiveBuffers.end())
                    {
                        size += pos;
                    }
                    memcpy(message->getReceivePayload().first + offset, buffer.c_str() + m_indexStartBuffer, size);
                    offset += size;
                    m_characterCounter -= size;
                    assert(m_characterCounter >= 0);
                    m_indexStartBuffer = 0;
                }
                m_characterCounter -= m_delimiter.size();
            }
            else
            {
                for (auto it = m_receiveBuffers.begin(); it != m_receiveBuffers.end(); ++it)
                {
                    const std::string& buffer = *it;
                    ssize_t sizeBuffer = buffer.size();
                    ssize_t size = sizeBuffer - m_indexStartBuffer;
                    assert(size >= 0);
                    memcpy(message->getReceivePayload().first + offset, buffer.c_str() + m_indexStartBuffer, size);
                    offset += size;
                    m_characterCounter -= size;
                    assert(m_characterCounter >= 0);
                    m_indexStartBuffer = 0;
                }
                memcpy(message->getReceivePayload().first + offset, receiveBuffer.c_str() + sizeDelimiterPartial, pos);
                m_characterCounter -= pos + m_delimiter.size();
                assert(m_characterCounter >= 0);
            }
            m_receiveBuffers.clear();
            auto callback = m_callback.lock();
            if (callback)
            {
                callback->received(message);
            }

            m_indexStartBuffer = pos + m_delimiter.size();

            for (size_t i = 1; i < positions.size(); ++i)
            {
                pos = positions[i];
                message = std::make_shared<ProtocolMessage>(0);
                ssize_t size = pos - m_indexStartBuffer;
                message->resizeReceivePayload(size);
                memcpy(message->getReceivePayload().first, receiveBuffer.c_str() + m_indexStartBuffer + sizeDelimiterPartial, size);
                m_characterCounter -= size + m_delimiter.size();
                assert(m_characterCounter >= 0);
                m_indexStartBuffer = pos + m_delimiter.size();
                if (callback)
                {
                    callback->received(message);
                }
            }
        }
        else
        {
            m_characterCounter += bytesReceived;
        }

        if (m_characterCounter == 0)
        {
            m_indexStartBuffer = 0;
        }
        else
        {
            if (sizeDelimiterPartial == 0)
            {
                m_receiveBuffers.push_back(std::move(receiveBuffer));
            }
            else
            {
                m_receiveBuffers.emplace_back(&receiveBuffer[sizeDelimiterPartial], receiveBuffer.size() - sizeDelimiterPartial);
            }
        }
    }
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


}   // namespace finalmq
