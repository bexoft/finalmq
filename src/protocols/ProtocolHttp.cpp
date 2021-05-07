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


#include "finalmq/protocols/ProtocolHttp.h"
#include "finalmq/protocolconnection/ProtocolMessage.h"
#include "finalmq/protocolconnection/ProtocolRegistry.h"
#include "finalmq/protocolconnection/ProtocolSession.h"
#include "finalmq/streamconnection/Socket.h"


namespace finalmq {


const std::string ProtocolHttp::FMQ_HTTP = "_fmq_http";
const std::string ProtocolHttp::FMQ_METHOD = "_fmq_method";
const std::string ProtocolHttp::FMQ_PROTOCOL = "_fmq_protocol";
const std::string ProtocolHttp::FMQ_PATH = "_fmq_path";
const std::string ProtocolHttp::FMQ_QUERY = "_fmq_query";
const std::string ProtocolHttp::FMQ_STATUS = "_fmq_status";
const std::string ProtocolHttp::FMQ_STATUSTEXT = "_fmq_statustext";
const std::string ProtocolHttp::HTTP_REQUEST = "request";
const std::string ProtocolHttp::HTTP_RESPONSE = "response";
    
static const std::string CONTENT_LENGTH = "Content-Length";
static const std::string HTTP_FMQ_SESSIONID = "HTTP_FMQ_SESSIONID";
static const std::string HTTP_COOKIE = "HTTP_COOKIE";

static const std::string HTTP_SET_FMQ_SESSION = "Set-FmqSession";
static const std::string HTTP_SET_COOKIE = "Set-Cookie";
static const std::string COOKIE_PREFIX = "fmq=";


//---------------------------------------
// ProtocolStream
//---------------------------------------


ProtocolHttp::ProtocolHttp()
    : m_randomDevice()
    , m_randomGenerator(m_randomDevice())
{

}



// IProtocol
void ProtocolHttp::setCallback(const std::weak_ptr<IProtocolCallback>& callback)
{
    m_callback = callback;
}

std::uint32_t ProtocolHttp::getProtocolId() const
{
    return PROTOCOL_ID;
}

bool ProtocolHttp::areMessagesResendable() const
{
    return false;
}

bool ProtocolHttp::doesSupportMetainfo() const
{
    return true;
}

bool ProtocolHttp::doesSupportSession() const
{
    return true;;
}

bool ProtocolHttp::needsReply() const
{
    return true;
}

IProtocol::FuncCreateMessage ProtocolHttp::getMessageFactory() const
{
    return []() {
        return std::make_shared<ProtocolMessage>(PROTOCOL_ID);
    };
}


static void split(const std::string& src, ssize_t indexBegin, ssize_t indexEnd, char delimiter, std::vector<std::string>& dest)
{
    const char delimiterstring[] = { delimiter, 0 };
    while (indexBegin < indexEnd)
    {
        size_t pos = src.find_first_of(delimiterstring, indexBegin);
        if (pos == std::string::npos || static_cast<ssize_t>(pos) > indexEnd)
        {
            pos = indexEnd;
        }
        ssize_t len = pos - indexBegin;
        assert(len >= 0);
        dest.emplace_back(&src[indexBegin], len);
        indexBegin += len + 1;
    }
}

static void splitOnce(const std::string& src, ssize_t indexBegin, ssize_t indexEnd, char delimiter, std::vector<std::string>& dest)
{
    const char delimiterstring[] = { delimiter, 0 };

    size_t pos = src.find_first_of(delimiterstring, indexBegin);
    if (pos == std::string::npos || static_cast<ssize_t>(pos) > indexEnd)
    {
        pos = indexEnd;
    }
    ssize_t len = pos - indexBegin;
    assert(len >= 0);
    dest.emplace_back(&src[indexBegin], len);
    indexBegin += len + 1;

    if (indexBegin < indexEnd)
    {
        ssize_t len = indexEnd - indexBegin;
        assert(len >= 0);
        dest.emplace_back(&src[indexBegin], len);
        indexBegin += len + 1;
    }
}


static const char tabDecToHex[] = {
    '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
};

static void encode(std::string& dest, const std::string& src)
{
    dest.reserve(src.size() * 3);
    char c;
    unsigned char uc;

    for (size_t i = 0; i < src.size(); ++i) 
    {
        c = src[i];
        if (isalnum(c) || c == '/' || c == '-' || c == '_' || c == '.' || c == '~')
        {
            dest += c;
        }
        else 
        {
            uc = c;
            int first = (uc >> 4) & 0x0f;
            int second = uc & 0x0f;
            assert(0 <= first && first < 16);
            assert(0 <= second && second < 16);
            dest += '%';
            dest += tabDecToHex[first];
            dest += tabDecToHex[second];
        }
    }
}


static void decode(std::string& dest, const std::string& src)
{
    dest.reserve(src.size());
    char code[3] = { 0 };
    unsigned long c = 0;

    for (size_t i = 0; i < src.size(); ++i)
    {
        if (src[i] == '%')
        {
            ++i;
            memcpy(code, &src[i], 2);
            c = strtoul(code, NULL, 16);
            dest += (char)c;
            ++i;
        }
        else
        {
            dest += src[i];
        }
    }
}




std::string ProtocolHttp::createSessionName()
{
    std::uint64_t sessionCounter = m_nextSessionNameCounter;
    ++m_nextSessionNameCounter;
    std::uint64_t v1 = m_randomVariable(m_randomGenerator);
    std::uint64_t v2 = m_randomVariable(m_randomGenerator);

    v1 &= 0xffffffff00000000ull;
    v1 |= (sessionCounter & 0x00000000ffffffffull);

    std::ostringstream oss;
    oss << std::hex << v2 << v1;
    std::string sessionName = oss.str();
    return sessionName;
}



void ProtocolHttp::checkSessionName()
{
    if (m_checkSessionName)
    {
        m_checkSessionName = false;
        auto callback = m_callback.lock();
        if (callback)
        {
            bool found = false;
            for (size_t i = 0; i < m_sessionIds.size() && !found; ++i)
            {
                found = callback->findSessionByName(m_sessionIds[i]);
            }
            if (!found)
            {
                m_sessionName = createSessionName();
                callback->setSessionName(m_sessionName);
                m_headerSendNext.emplace_back(HTTP_SET_FMQ_SESSION);
                m_headerSendNext.emplace_back(m_sessionName);
                m_headerSendNext.emplace_back(HTTP_SET_COOKIE);
                m_headerSendNext.push_back(COOKIE_PREFIX + m_sessionName);
            }
            callback->connected();
        }
    }
}



void ProtocolHttp::cookiesToSessionIds(const std::string& cookies)
{
    m_sessionIds.clear();
    size_t posStart = 0;
    while (posStart != std::string::npos)
    {
        size_t posEnd = cookies.find_first_of(';', posStart);
        std::string cookie;
        if (posEnd != std::string::npos)
        {
            cookie = std::string(cookies, posStart, posEnd - posStart);
            posStart = posEnd + 1;
        }
        else
        {
            cookie = std::string(cookies, posStart);
            posStart = posEnd;
        }

        size_t pos = cookie.find_first_of("fmq=");
        if (pos != std::string::npos)
        {
            pos += 4;
            std::string sessionId = { cookie, pos };
            if (!sessionId.empty())
            {
                m_sessionIds.emplace_back(std::move(sessionId));
            }
        }
    }
}




bool ProtocolHttp::receiveHeaders(ssize_t bytesReceived)
{
    bool ok = true;
    bytesReceived += m_sizeRemaining;
    assert(bytesReceived <= static_cast<ssize_t>(m_receiveBuffer.size()));
    while (m_offsetRemaining < bytesReceived && ok)
    {
        size_t index = m_receiveBuffer.find_first_of('\n', m_offsetRemaining);
        if (index != std::string::npos)
        {
            ssize_t indexEndLine = index;
            --indexEndLine; // goto '\r'
            ssize_t len = indexEndLine - m_offsetRemaining;
            if (len < 0 || m_receiveBuffer[indexEndLine] != '\r')
            {
                ok = false;
            }
            if (ok)
            {
                if (m_state == STATE_FIND_FIRST_LINE)
                {
                    m_contentLength = 0;
                    if (len < 4)
                    {
                        ok = false;
                    }
                    else
                    {
                        // is response
                        if (m_receiveBuffer[m_offsetRemaining] == 'H' && m_receiveBuffer[m_offsetRemaining + 1] == 'T')
                        {
                            std::vector<std::string> lineSplit;
                            split(m_receiveBuffer, m_offsetRemaining, indexEndLine, ' ', lineSplit);
                            if (lineSplit.size() == 3)
                            {
                                m_message = std::make_shared<ProtocolMessage>(0);
                                m_message->addMetainfo(FMQ_HTTP, std::string(HTTP_RESPONSE));
                                m_message->addMetainfo(FMQ_PROTOCOL, std::move(lineSplit[0]));
                                m_message->addMetainfo(FMQ_STATUS, std::move(lineSplit[1]));
                                m_message->addMetainfo(FMQ_STATUSTEXT, std::move(lineSplit[2]));
                                m_state = STATE_FIND_HEADERS;
                            }
                            else
                            {
                                ok = false;
                            }
                        }
                        else
                        {
                            std::vector<std::string> lineSplit;
                            split(m_receiveBuffer, m_offsetRemaining, indexEndLine, ' ', lineSplit);
                            if (lineSplit.size() == 3)
                            {
                                std::vector<std::string> pathquerySplit;
                                split(lineSplit[1], 0, lineSplit[1].size(), '?', pathquerySplit);
                                m_message = std::make_shared<ProtocolMessage>(0);
                                m_message->addMetainfo(FMQ_HTTP, std::string(HTTP_REQUEST));
                                m_message->addMetainfo(FMQ_METHOD, std::move(lineSplit[0]));
                                if (pathquerySplit.size() >= 1)
                                {
                                    std::string path;
                                    decode(path, pathquerySplit[0]);
                                    m_message->addMetainfo(FMQ_PATH, std::move(path));
                                }
                                if (pathquerySplit.size() >= 2)
                                {
                                    std::vector<std::string> querySplit;
                                    split(pathquerySplit[1], 0, pathquerySplit[1].size(), '&', querySplit);
                                    for (size_t i = 0; i < querySplit.size(); ++i)
                                    {
                                        m_message->addMetainfo(FMQ_QUERY + std::to_string(i), std::move(querySplit[i]));
                                    }
                                }
                                m_message->addMetainfo(FMQ_PROTOCOL, std::move(lineSplit[2]));
                                m_state = STATE_FIND_HEADERS;
                            }
                            else
                            {
                                ok = false;
                            }
                        }
                    }
                }
                else if (m_state == STATE_FIND_HEADERS)
                {
                    if (len == 0)
                    {
                        checkSessionName();
                        if (m_contentLength == 0)
                        {
                            m_state = STATE_CONTENT_DONE;
                        }
                        else
                        {
                            m_state = STATE_CONTENT;
                            m_message->resizeReceivePayload(m_contentLength);
                        }
                        m_indexFilled = 0;
                        m_offsetRemaining += 2;
                        break;
                    }
                    else
                    {
                        std::vector<std::string> lineSplit;
                        splitOnce(m_receiveBuffer, m_offsetRemaining, indexEndLine, ':', lineSplit);
                        if (lineSplit.size() == 2)
                        {
                            std::string& value = lineSplit[1];
                            if (!value.empty() && value[0] == ' ')
                            {
                                value.erase(value.begin());
                            }
                            if (lineSplit[0] == CONTENT_LENGTH)
                            {
                                m_contentLength = std::atoll(value.c_str());
                            }
                            if (m_checkSessionName)
                            {
                                if (lineSplit[0] == HTTP_FMQ_SESSIONID)
                                {
                                    m_sessionIds.clear();
                                    if (!value.empty())
                                    {
                                        m_sessionIds.push_back(value);
                                    }
                                    m_stateSessionId = SESSIONID_FMQ;
                                }
                                else if (lineSplit[0] == HTTP_COOKIE)
                                {
                                    if (m_stateSessionId == SESSIONID_NONE)
                                    {
                                        cookiesToSessionIds(value);
                                        m_stateSessionId = SESSIONID_COOKIE;
                                    }
                                }
                            }
                            m_message->addMetainfo(std::move(lineSplit[0]), std::move(lineSplit[1]));
                        }
                        else if (lineSplit.size() == 1)
                        {
                            m_message->addMetainfo(std::move(lineSplit[0]), "");
                        }
                        else
                        {
                            assert(false);
                            ok = false;
                        }
                    }
                }
                m_offsetRemaining += len + 2;
            }
        }
        else
        {
            break;
        }
    }
    m_sizeRemaining = bytesReceived - m_offsetRemaining;
    assert(m_sizeRemaining >= 0);
    return ok;
}


void ProtocolHttp::reset()
{
    m_offsetRemaining = 0;
    m_sizeRemaining = 0;
    m_contentLength = 0;
    m_indexFilled = 0;
    m_message = nullptr;
    m_state = STATE_FIND_FIRST_LINE;
}


void ProtocolHttp::receive(const SocketPtr& socket, int bytesToRead)
{
    bool ok = true;

    if (m_state != STATE_CONTENT)
    {
        if (m_offsetRemaining == 0)
        {
            m_receiveBuffer.resize(m_sizeRemaining + bytesToRead);
        }
        else
        {
            std::string temp;
            if (m_sizeRemaining > 0)
            {
                temp = std::move(m_receiveBuffer);
            }
            m_receiveBuffer.resize(m_sizeRemaining + bytesToRead);
            if (m_sizeRemaining > 0)
            {
                memcpy(&m_receiveBuffer[0], &temp[m_offsetRemaining], m_sizeRemaining);
            }
            m_offsetRemaining = 0;
        }

        ssize_t bytesReceived = 0;
        int res = 0;
        do
        {
            res = socket->receive(const_cast<char*>(m_receiveBuffer.data() + bytesReceived + m_sizeRemaining), bytesToRead - bytesReceived);
            if (res > 0)
            {
                bytesReceived += res;
            }
        } while (res > 0 && bytesReceived < bytesToRead);
        if (res >= 0)
        {
            assert(bytesReceived <= bytesToRead);
            ok = receiveHeaders(bytesReceived);
            if (ok && m_state == STATE_CONTENT)
            {
                BufferRef payload = m_message->getReceivePayload();
                assert(payload.second == m_contentLength);
                if (m_sizeRemaining <= m_contentLength)
                {
                    memcpy(payload.first, m_receiveBuffer.data() + m_offsetRemaining, m_sizeRemaining);
                    m_indexFilled = m_sizeRemaining;
                    assert(m_indexFilled <= m_contentLength);
                    if (m_indexFilled == m_contentLength)
                    {
                        m_state = STATE_CONTENT_DONE;
                    }
                }
                else
                {
                    // too much content
                    ok = false;
                }
            }
        }
    }
    else
    {
        BufferRef payload = m_message->getReceivePayload();
        assert(payload.second == m_contentLength);
        ssize_t remainingContent = m_contentLength - m_indexFilled;
        if (bytesToRead <= remainingContent)
        {
            ssize_t bytesReceived = 0;
            int res = 0;
            do
            {
                res = socket->receive(const_cast<char*>(payload.first + bytesReceived + m_indexFilled), bytesToRead - bytesReceived);
                if (res > 0)
                {
                    bytesReceived += res;
                }
            } while (res > 0 && bytesReceived < bytesToRead);
            if (res >= 0)
            {
                m_indexFilled += bytesReceived;
                assert(m_indexFilled <= m_contentLength);
                if (m_indexFilled == m_contentLength)
                {
                    m_state = STATE_CONTENT_DONE;
                }
            }
        }
        else
        {
            // too much content
            ok = false;
        }
    }

    if (ok)
    {
        if (m_state == STATE_CONTENT_DONE)
        {
            auto callback = m_callback.lock();
            if (callback)
            {
                callback->received(m_message);
            }
            reset();
        }
    }
    else
    {
        reset();
        auto callback = m_callback.lock();
        if (callback)
        {
            callback->disconnected();
        }
    }
}


static std::string HEADER_KEEP_ALIVE = "Connection: keep-alive\r\n";

void ProtocolHttp::prepareMessageToSend(IMessagePtr message)
{
    std::string firstLine;
    const std::string* http = message->getMetainfo(FMQ_HTTP);
    if (http && *http == HTTP_REQUEST)
    {
        const std::string* method = message->getMetainfo(FMQ_METHOD);
        const std::string* path = message->getMetainfo(FMQ_PATH);
        if (method && path)
        {
            std::string pathEncode;
            encode(pathEncode, *path);
            firstLine = *method;
            firstLine += ' ';
            firstLine += pathEncode;

            int i = 0;
            const std::string* query = message->getMetainfo(FMQ_QUERY + std::to_string(i));
            while (query != nullptr)
            {
                if (i == 0)
                {
                    firstLine += '?';
                }
                else
                {
                    firstLine += '&';
                }
                std::string queryEncode;
                encode(queryEncode, *query);
                firstLine += queryEncode;
                ++i;
                query = message->getMetainfo(FMQ_QUERY + std::to_string(i));
            }
            if (query && !query->empty())
            {
                firstLine += '?';
                firstLine += *query;
            }
            firstLine += " HTTP/1.1";
        }
    }
    else
    {
        const std::string* status = message->getMetainfo(FMQ_STATUS);
        const std::string* statustext = message->getMetainfo(FMQ_STATUSTEXT);
        firstLine = "HTTP/1.1 ";
        if (status)
        {
            firstLine += *status;
            firstLine += ' ';
        }
        else
        {
            firstLine += "200 ";
        }
        if (statustext)
        {
            firstLine += *statustext;
        }
        else
        {
            firstLine += "OK";
        }
    }

    int sumHeaderSize = firstLine.size() + 4;   // 2 = '\r\n' and last empty line
    if (http && *http == HTTP_REQUEST)
    {
        sumHeaderSize += m_headerHost.size();   // Host: hostname\r\n
    }
    sumHeaderSize += HEADER_KEEP_ALIVE.size();  // Connection: keep-alive\r\n

    ssize_t sizeBody = message->getTotalSendPayloadSize();
    ProtocolMessage::Metainfo& metainfo = message->getAllMetainfo();
    metainfo.emplace_back(CONTENT_LENGTH);
    metainfo.push_back(std::to_string(sizeBody));
    if (!m_headerSendNext.empty())
    {
        metainfo.insert(metainfo.end(), m_headerSendNext.begin(), m_headerSendNext.end());
        m_headerSendNext.clear();
    }
    static const std::string PREFIX_PRIVATE_HEADER = "_fmq_";
    for (size_t i = 0; i < metainfo.size(); i += 2)
    {
        const std::string& key = metainfo[i];
        const std::string& value = metainfo[i+1];
        if (!key.empty() && key.find_first_of(PREFIX_PRIVATE_HEADER) != 0)
        {
            sumHeaderSize += key.size() + value.size() + 4;    // 4 = ': ' and '\r\n'
        }
    }

    char* headerBuffer = message->addSendHeader(sumHeaderSize);
    int index = 0;
    assert(index + firstLine.size() + 2 <= sumHeaderSize);
    memcpy(headerBuffer + index, firstLine.data(), firstLine.size());
    index += firstLine.size();
    memcpy(headerBuffer + index, "\r\n", 2);
    index += 2;

    if (http && *http == HTTP_REQUEST)
    {
        // Host: hostname\r\n
        assert(index + m_headerHost.size() <= sumHeaderSize);
        memcpy(headerBuffer + index, m_headerHost.data(), m_headerHost.size());
        index += m_headerHost.size();
    }

    // Connection: keep-alive\r\n
    assert(index + HEADER_KEEP_ALIVE.size() <= sumHeaderSize);
    memcpy(headerBuffer + index, HEADER_KEEP_ALIVE.data(), HEADER_KEEP_ALIVE.size());
    index += HEADER_KEEP_ALIVE.size();

    for (size_t i = 0; i < metainfo.size(); i += 2)
    {
        const std::string& key = metainfo[i];
        const std::string& value = metainfo[i+1];
        if (!key.empty() && key.find_first_of(PREFIX_PRIVATE_HEADER) != 0)
        {
            assert(index + key.size() + value.size() + 4 <= sumHeaderSize);
            memcpy(headerBuffer + index, key.data(), key.size());
            index += key.size();
            memcpy(headerBuffer + index, ": ", 2);
            index += 2;
            memcpy(headerBuffer + index, value.data(), value.size());
            index += value.size();
            memcpy(headerBuffer + index, "\r\n", 2);
            index += 2;
        }
    }
    assert(index + 2 == sumHeaderSize);
    memcpy(headerBuffer + index, "\r\n", 2);
    index += 2;
    assert(index == sumHeaderSize);

    message->prepareMessageToSend();
}

void ProtocolHttp::socketConnected(IProtocolSession& session)
{
    ConnectionData connectionData = session.getConnectionData();
    m_headerHost = "Host: " + connectionData.hostname + ":" + std::to_string(connectionData.port) + "\r\n";
    // for incomming connection -> do the connection event after checking the session ID
    if (connectionData.incomingConnection)
    {
        m_checkSessionName = true;
    }
    else
    {
        auto callback = m_callback.lock();
        if (callback)
        {
            callback->connected();
        }
    }
}

void ProtocolHttp::socketDisconnected()
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->disconnected();
    }
}

void ProtocolHttp::moveOldProtocolState(IProtocol& /*protocolOld*/)
{
    //assert(protocolOld.getProtocolId() == PROTOCOL_ID);
    //if (protocolOld.getProtocolId() == PROTOCOL_ID)
    //{
    //    ProtocolHttp& old = static_cast<ProtocolHttp&>(protocolOld);
    //}
}




//---------------------------------------
// ProtocolHttpFactory
//---------------------------------------


struct RegisterProtocolHttpFactory
{
    RegisterProtocolHttpFactory()
    {
        ProtocolRegistry::instance().registerProtocolFactory(ProtocolHttp::PROTOCOL_ID, std::make_shared<ProtocolHttpFactory>());
    }
} g_registerProtocolHttpFactory;


// IProtocolFactory
IProtocolPtr ProtocolHttpFactory::createProtocol()
{
    return std::make_shared<ProtocolHttp>();
}


}   // namespace finalmq
