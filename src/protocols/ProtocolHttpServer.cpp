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


#include "finalmq/protocols/ProtocolHttpServer.h"
#include "finalmq/protocolsession/ProtocolMessage.h"
#include "finalmq/protocolsession/ProtocolRegistry.h"
#include "finalmq/protocolsession/ProtocolSession.h"
#include "finalmq/streamconnection/Socket.h"

//#include "finalmq/helpers/ModulenameFinalmq.h"

#include <algorithm>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>


namespace finalmq {

const int ProtocolHttpServer::PROTOCOL_ID = 4;
const std::string ProtocolHttpServer::PROTOCOL_NAME = "httpserver";


const std::string ProtocolHttpServer::FMQ_HTTP = "fmq_http";
const std::string ProtocolHttpServer::FMQ_METHOD = "fmq_method";
const std::string ProtocolHttpServer::FMQ_PROTOCOL = "fmq_protocol";
const std::string ProtocolHttpServer::FMQ_PATH = "fmq_path";
const std::string ProtocolHttpServer::FMQ_QUERY_PREFIX = "FMQQUERY_";
const std::string ProtocolHttpServer::FMQ_STATUS = "fmq_status";
const std::string ProtocolHttpServer::FMQ_STATUSTEXT = "fmq_statustext";
const std::string ProtocolHttpServer::HTTP_REQUEST = "request";
const std::string ProtocolHttpServer::HTTP_RESPONSE = "response";
    
static const std::string CONTENT_LENGTH = "Content-Length";
static const std::string FMQ_SESSIONID = "fmq_sessionid";
static const std::string HTTP_COOKIE = "Cookie";

static const std::string FMQ_CREATESESSION = "fmq_createsession";
static const std::string FMQ_SET_SESSION = "fmq_setsession";
static const std::string HTTP_SET_COOKIE = "Set-Cookie";
static const std::string COOKIE_PREFIX = "fmq=";

static const std::string FMQ_PATH_POLL = "/fmq/poll";
static const std::string FMQ_PATH_PING = "/fmq/ping";
static const std::string FMQ_PATH_CONFIG = "/fmq/config";
static const std::string FMQ_PATH_CREATESESSION = "/fmq/createsession";
static const std::string FMQ_PATH_REMOVESESSION = "/fmq/removesession";
static const std::string FMQ_MULTIPART_BOUNDARY = "B9BMAhxAhY.mQw1IDRBA";


//---------------------------------------
// ProtocolHttpServer
//---------------------------------------


std::atomic_int64_t ProtocolHttpServer::m_nextSessionNameCounter{ 1 };

ProtocolHttpServer::ProtocolHttpServer()
    : m_randomDevice()
    , m_randomGenerator(m_randomDevice())
{

}

ProtocolHttpServer::~ProtocolHttpServer()
{
}


// IProtocol
void ProtocolHttpServer::setCallback(const std::weak_ptr<IProtocolCallback>& callback)
{
    m_callback = callback;
    std::shared_ptr<IProtocolCallback> cb = callback.lock();
    if (cb)
    {
        // 5 minutes session timeout
        cb->setActivityTimeout(5 * 60000);
    }
}

void ProtocolHttpServer::setConnection(const IStreamConnectionPtr& connection)
{
    m_connection = connection;
}

IStreamConnectionPtr ProtocolHttpServer::getConnection() const
{
    return m_connection;
}

void ProtocolHttpServer::disconnect()
{
    assert(m_connection);
    m_connection->disconnect();
}

std::uint32_t ProtocolHttpServer::getProtocolId() const
{
    return PROTOCOL_ID;
}

bool ProtocolHttpServer::areMessagesResendable() const
{
    return false;
}

bool ProtocolHttpServer::doesSupportMetainfo() const
{
    return true;
}

bool ProtocolHttpServer::doesSupportSession() const
{
    return true;
}

bool ProtocolHttpServer::needsReply() const
{
    return true;
}

bool ProtocolHttpServer::isMultiConnectionSession() const
{
    return true;
}

bool ProtocolHttpServer::isSendRequestByPoll() const
{
    return true;
}

bool ProtocolHttpServer::doesSupportFileTransfer() const
{
    return true;
}

IProtocol::FuncCreateMessage ProtocolHttpServer::getMessageFactory() const
{
    return []() {
        return std::make_shared<ProtocolMessage>(PROTOCOL_ID);
    };
}


static void split(const std::string& src, ssize_t indexBegin, ssize_t indexEnd, char delimiter, std::vector<std::string>& dest)
{
    while (indexBegin < indexEnd)
    {
        size_t pos = src.find_first_of(delimiter, indexBegin);
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
    size_t pos = src.find_first_of(delimiter, indexBegin);
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




std::string ProtocolHttpServer::createSessionName()
{
    std::uint64_t sessionCounter = m_nextSessionNameCounter.fetch_add(1);
    std::uint64_t v1 = m_randomVariable(m_randomGenerator);
    std::uint64_t v2 = m_randomVariable(m_randomGenerator);

    std::ostringstream oss;
    oss << std::hex << v2 << v1 << '.' << std::dec <<sessionCounter;
    std::string sessionName = oss.str();
    return sessionName;
}



void ProtocolHttpServer::checkSessionName()
{
    if (m_path && *m_path == FMQ_PATH_CREATESESSION)
    {
        m_createSession = true;
    }

    auto callback = m_callback.lock();
    if (callback)
    {
        bool found = false;
        if (!m_createSession)
        {
            std::vector<std::string> sessionMatched;
            for (size_t i = 0; i < m_sessionNames.size(); ++i)
            {
                assert(m_connection);
                bool foundInNames = callback->findSessionByName(m_sessionNames[i], shared_from_this());
//                streamInfo << this << " findSessionByName: " << foundInNames;
                if (foundInNames)
                {
                    sessionMatched.push_back(std::move(m_sessionNames[i]));
                }
            }
            std::string sessionFound;
            if (sessionMatched.size() == 1)
            {
                sessionFound = std::move(sessionMatched[0]);
            }
            else
            {
                std::int64_t counterMax = 0;
                for (size_t i = 0; i < sessionMatched.size(); ++i)
                {
                    std::string& session = sessionMatched[i];
                    size_t pos = session.find_first_of('.');
                    if (pos != std::string::npos)
                    {
                        std::int64_t counter = atoll(&session[pos + 1]);
                        if (counter > counterMax)
                        {
                            counterMax = counter;
                            sessionFound = std::move(session);
                        }
                    }
                }
            }
            if (!sessionFound.empty())
            {
                found = true;
                if (m_sessionName != sessionFound)
                {
//                    streamInfo << this << " name before: " << m_sessionName;
                    m_sessionName = std::move(sessionFound);
                }
            }
        }
//        streamInfo << this << " name: " << m_sessionName;
        if (m_createSession || !found)
        {
            m_sessionName = createSessionName();
//            streamInfo << this << " create session: " << m_sessionName;
            assert(m_connection);
            callback->setSessionName(m_sessionName, shared_from_this(), m_connection);
            m_headerSendNext[FMQ_SET_SESSION] = m_sessionName;
            m_headerSendNext[HTTP_SET_COOKIE] = COOKIE_PREFIX + m_sessionName + "; path=/";
        }
    }
    m_sessionNames.clear();
}



void ProtocolHttpServer::cookiesToSessionIds(const std::string& cookies)
{
    m_sessionNames.clear();
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

        size_t pos = cookie.find("fmq=");
        if (pos != std::string::npos)
        {
            pos += 4;
            std::string sessionId = { cookie, pos };
            if (!sessionId.empty())
            {
                m_sessionNames.emplace_back(std::move(sessionId));
            }
        }
    }
}




bool ProtocolHttpServer::receiveHeaders(ssize_t bytesReceived)
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
                                Variant& controlData = m_message->getControlData();
                                controlData.add(FMQ_HTTP, std::string(HTTP_RESPONSE));
                                controlData.add(FMQ_PROTOCOL, std::move(lineSplit[0]));
                                controlData.add(FMQ_STATUS, std::move(lineSplit[1]));
                                controlData.add(FMQ_STATUSTEXT, std::move(lineSplit[2]));
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
                                IMessage::Metainfo& metainfo = m_message->getAllMetainfo();
                                metainfo[FMQ_HTTP] = HTTP_REQUEST;
                                metainfo[FMQ_METHOD] = std::move(lineSplit[0]);
                                metainfo[FMQ_PROTOCOL] = std::move(lineSplit[2]);
                                if (pathquerySplit.size() >= 1)
                                {
                                    std::string path;
                                    decode(path, pathquerySplit[0]);
                                    m_path = &metainfo[FMQ_PATH];
                                    *m_path = std::move(path);
                                }
                                if (pathquerySplit.size() >= 2)
                                {
                                    std::vector<std::string> querySplit;
                                    split(pathquerySplit[1], 0, pathquerySplit[1].size(), '&', querySplit);
                                    for (size_t i = 0; i < querySplit.size(); ++i)
                                    {
                                        std::string queryTotal;
                                        decode(queryTotal, querySplit[i]);
                                        std::vector<std::string> queryNameValue;
                                        split(queryTotal, 0, queryTotal.size(), '=', queryNameValue);
                                        if (queryNameValue.size() == 1)
                                        {
                                            metainfo[FMQ_QUERY_PREFIX + queryNameValue[0]] = std::string();
                                        }
                                        else if (queryNameValue.size() >= 2)
                                        {
                                            metainfo[FMQ_QUERY_PREFIX + queryNameValue[0]] = std::move(queryNameValue[1]);
                                        }
                                    }
                                }
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
                        if (m_contentLength == 0)
                        {
                            m_state = STATE_CONTENT_DONE;
                        }
                        else
                        {
                            m_state = STATE_CONTENT;
                            m_message->resizeReceiveBuffer(m_contentLength);
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
                            else if (lineSplit[0] == FMQ_CREATESESSION)
                            {
                                m_createSession = true;
                            }
                            else if (lineSplit[0] == FMQ_SESSIONID)
                            {
                                m_sessionNames.clear();
                                if (!value.empty())
                                {
                                    m_sessionNames.push_back(value);
                                }
                                m_stateSessionId = SESSIONID_FMQ;
                            }
                            else if (lineSplit[0] == HTTP_COOKIE)
                            {
                                if (m_stateSessionId == SESSIONID_NONE)
                                {
                                    cookiesToSessionIds(value);
//                                    if (!m_sessionNames.empty())
//                                    {
//                                        streamInfo << this << " input cookie: " << m_sessionNames[0];
//                                    }
                                    m_stateSessionId = SESSIONID_COOKIE;
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


void ProtocolHttpServer::reset()
{
    m_offsetRemaining = 0;
    m_sizeRemaining = 0;
    m_contentLength = 0;
    m_indexFilled = 0;
    m_message = nullptr;
    m_state = STATE_FIND_FIRST_LINE;
    m_stateSessionId = SESSIONID_NONE;
    m_createSession = false;
    m_sessionNames.clear();
    m_path = nullptr;
}



static std::string HEADER_KEEP_ALIVE = "Connection: keep-alive\r\n";

bool ProtocolHttpServer::sendMessage(IMessagePtr message)
{
    assert(!message->wasSent());
    std::string firstLine;
    const Variant& controlData = message->getControlData();
    bool pollStop = false;
    if (m_chunkedState)
    {
        const bool* pPollStop = controlData.getData<bool>("fmq_poll_stop");
        if (pPollStop && *pPollStop)
        {
            pollStop = *pPollStop;
        }
    }
    const std::string* filename = controlData.getData<std::string>("filetransfer");
    ssize_t filesize = -1;
    if (filename)
    {
        struct stat statdata;
        memset(&statdata, 0, sizeof(statdata));
        int res = OperatingSystem::instance().stat(filename->c_str(), &statdata);
        if (res == 0)
        {
            filesize = statdata.st_size;
            message->downsizeLastSendPayload(0);
        }
    }
    const std::string* http = controlData.getData<std::string>(FMQ_HTTP);
    if (m_chunkedState < 2)
    {
        if (http && *http == HTTP_REQUEST)
        {
            if (filename && filesize == -1)
            {
                filesize = 0;
            }
            const std::string* method = controlData.getData<std::string>(FMQ_METHOD);
            const std::string* path = controlData.getData<std::string>(FMQ_PATH);
            if (method && path)
            {
                std::string pathEncode;
                encode(pathEncode, *path);
                firstLine = *method;
                firstLine += ' ';
                firstLine += pathEncode;

                const VariantStruct* queries = controlData.getData<VariantStruct>("queries");
                if (queries)
                {
                    for (auto it = queries->begin(); it != queries->end(); ++it)
                    {
                        if (it == queries->begin())
                        {
                            firstLine += '?';
                        }
                        else
                        {
                            firstLine += '&';
                        }
                        std::string key;
                        std::string value;

                        encode(key, it->first);
                        encode(value, it->second);
                        firstLine += key;
                        firstLine += '=';
                        firstLine += value;
                    }
                }

                firstLine += " HTTP/1.1";
            }
        }
        else
        {
            std::string status = controlData.getDataValue<std::string>(FMQ_STATUS);
            const std::string* statustext = controlData.getData<std::string>(FMQ_STATUSTEXT);
            if (filename && filesize == -1)
            {
                status = "404";
                static const std::string FILE_NOT_FOUND = "file not found";
                statustext = &FILE_NOT_FOUND;
            }
            firstLine = "HTTP/1.1 ";
            if (!status.empty())
            {
                firstLine += status;
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
    }

    size_t sumHeaderSize = 0;
    if (m_chunkedState < 2)
    {
        sumHeaderSize += firstLine.size() + 2 + 2;   // 2 = '\r\n' and 2 = last empty line
        sumHeaderSize += HEADER_KEEP_ALIVE.size();  // Connection: keep-alive\r\n
    }
    if (http && *http == HTTP_REQUEST)
    {
        sumHeaderSize += m_headerHost.size();   // Host: hostname\r\n
    }

    ssize_t sizeBody = message->getTotalSendPayloadSize();
    if (filesize != -1)
    {
        sizeBody = filesize;
    }

    ProtocolMessage::Metainfo& metainfo = message->getAllMetainfo();
    if (m_chunkedState == 0)
    {
        metainfo[CONTENT_LENGTH] = std::to_string(sizeBody);
    }
    if (!m_headerSendNext.empty())
    {
        metainfo.insert(m_headerSendNext.begin(), m_headerSendNext.end());
        m_headerSendNext.clear();
    }
    for (auto it = metainfo.begin(); it != metainfo.end(); ++it)
    {
        const std::string& key = it->first;
        const std::string& value = it->second;
        if (!key.empty())
        {
            sumHeaderSize += key.size() + value.size() + 4;    // 4 = ': ' and '\r\n'
        }
    }

    char* headerBuffer = nullptr;
    size_t index = 0;
    if (!pollStop || m_multipart)
    {
        if (m_chunkedState >= 2)
        {
            assert(sumHeaderSize == 0);
            message->addSendPayload("\r\n");
            int sizeChunkedData = static_cast<int>(sizeBody);
            firstLine.clear();
            if (m_chunkedState == 2 || m_chunkedState == 3)
            {
                m_chunkedState = 4;
            }
            else
            {
//                firstLine = "\r\n";
            }
            char buffer[50];
#ifdef WIN32
            _itoa_s(sizeChunkedData, buffer, sizeof(buffer), 16);
#else
            snprintf(buffer, sizeof(buffer), "%X", sizeChunkedData);
#endif
            firstLine += buffer;
            for (auto& c : firstLine)
            {
                c = toupper(c);
            }
            sumHeaderSize += firstLine.size() + 2;  // "\r\n"
        }

        headerBuffer = message->addSendHeader(sumHeaderSize);
        index = 0;
        assert(index + firstLine.size() + 2 <= sumHeaderSize);
        memcpy(headerBuffer + index, firstLine.data(), firstLine.size());
        index += firstLine.size();
        memcpy(headerBuffer + index, "\r\n", 2);
        index += 2;
    }
    else
    {
        headerBuffer = message->addSendHeader(sumHeaderSize);
        index = 0;
    }

    if (http && *http == HTTP_REQUEST)
    {
        // Host: hostname\r\n
        assert(index + m_headerHost.size() <= sumHeaderSize);
        memcpy(headerBuffer + index, m_headerHost.data(), m_headerHost.size());
        index += m_headerHost.size();
    }

    // Connection: keep-alive\r\n
    if (m_chunkedState < 2)
    {
        assert(index + HEADER_KEEP_ALIVE.size() <= sumHeaderSize);
        memcpy(headerBuffer + index, HEADER_KEEP_ALIVE.data(), HEADER_KEEP_ALIVE.size());
        index += HEADER_KEEP_ALIVE.size();
    }

    for (auto it = metainfo.begin(); it != metainfo.end(); ++it)
    {
        const std::string& key = it->first;
        const std::string& value = it->second;
        if (!key.empty())
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
    if (m_chunkedState < 2)
    {
        assert(index + 2 == sumHeaderSize);
        memcpy(headerBuffer + index, "\r\n", 2);
    }

    if (pollStop)
    {
        message->addSendPayload("0\r\n\r\n");
        m_chunkedState = 0;
        m_multipart = false;
    }

    message->prepareMessageToSend();

    assert(m_connection);
    bool ok = m_connection->sendMessage(message);


    if (ok && filename)
    {
        std::string file = *filename;
        std::weak_ptr<ProtocolHttpServer> pThisWeak = shared_from_this();
        GlobalExecutorWorker::instance().addAction([pThisWeak, file, filesize]() {
            std::shared_ptr<ProtocolHttpServer> pThis = pThisWeak.lock();
            if (!pThis)
            {
                return;
            }
            int flags = O_RDONLY;
#ifdef WIN32
            flags |= O_BINARY;
#endif
            int fd = OperatingSystem::instance().open(file.c_str(), flags);
            if (fd != -1)
            {
                int len = static_cast<int>(filesize);
                int err = 0;
                int lenReceived = 0;
                bool ex = false;
                while (!ex)
                {
                    int size = std::min(1024, len);
                    IMessagePtr messageData = std::make_shared<ProtocolMessage>(0);
                    char* buf = messageData->addSendPayload(size);
                    do
                    {
                        err = OperatingSystem::instance().read(fd, buf, size);
                    } while (err == -1 && OperatingSystem::instance().getLastError() == SOCKETERROR(EINTR));

                    if (err > 0)
                    {
                        assert(err <= size);
                        if (err < size)
                        {
                            messageData->downsizeLastSendPayload(err);
                        }
                        bool ok = pThis->m_connection->sendMessage(messageData);
                        buf += err;
                        len -= err;
                        lenReceived += err;
                        err = 0;
                        assert(len >= 0);
                        if (len == 0 || !ok)
                        {
                            ex = true;
                        }
                    }
                    else
                    {
                        ex = true;
                    }
                }
                if (lenReceived < filesize)
                {
                    pThis->m_connection->disconnect();
                }
            }
            else
            {
                pThis->m_connection->disconnect();
            }
        });
    }

    return ok;
}


void ProtocolHttpServer::moveOldProtocolState(IProtocol& /*protocolOld*/)
{
    //assert(protocolOld.getProtocolId() == PROTOCOL_ID);
    //if (protocolOld.getProtocolId() == PROTOCOL_ID)
    //{
    //    ProtocolHttpServer& old = static_cast<ProtocolHttpServer&>(protocolOld);
    //}
}




bool ProtocolHttpServer::handleInternalCommands(const std::shared_ptr<IProtocolCallback>& callback)
{
    assert(callback);
    bool handled = false;
    if (m_path)
    {
        if (*m_path == FMQ_PATH_POLL)
        {
            m_chunkedState = 1;
            assert(m_message);
            handled = true;
            std::int32_t timeout = -1;
            std::int32_t pollCountMax = 1;
            const std::string* strTimeout = m_message->getMetainfo("FMQQUERY_timeout");
            const std::string* strCount = m_message->getMetainfo("FMQQUERY_count");
            const std::string* strMultipart = m_message->getMetainfo("FMQQUERY_multipart");
            if (strTimeout)
            {
                timeout = std::atoi(strTimeout->c_str());
            }
            if (strCount)
            {
                pollCountMax = std::atoi(strCount->c_str());
            }
            if (strMultipart)
            {
                m_multipart = (*strMultipart == "true") ? true : false;
            }
            if (strTimeout && !strCount)
            {
                pollCountMax = -1;
            }
            if (!strTimeout && strCount)
            {
                timeout = -1;
            }
            IMessagePtr message = getMessageFactory()();
            std::string contentType;
            if (m_multipart)
            {
                contentType = "multipart/x-mixed-replace; boundary=";
                contentType += FMQ_MULTIPART_BOUNDARY;
            }
            else
            {
                contentType = "text/event-stream";
            }
            message->addMetainfo("Content-Type", contentType);
            message->addMetainfo("Transfer-Encoding", "chunked");
            sendMessage(message);
            m_chunkedState = 2;
            callback->pollRequest(m_connectionId, timeout, pollCountMax);
        }
        else if (*m_path == FMQ_PATH_PING)
        {
            handled = true;
            assert(m_connection);
            sendMessage(getMessageFactory()());
            callback->activity();
        }
        else if (*m_path == FMQ_PATH_CONFIG)
        {
            handled = true;
            const std::string* timeout = m_message->getMetainfo("FMQQUERY_activitytimeout");
            if (timeout)
            {
                callback->setActivityTimeout(std::atoi(timeout->c_str()));
            }
            const std::string* pollMaxRequests = m_message->getMetainfo("FMQQUERY_pollmaxrequests");
            if (pollMaxRequests)
            {
                callback->setPollMaxRequests(std::atoi(pollMaxRequests->c_str()));
            }
            sendMessage(getMessageFactory()());
            callback->activity();
        }
        else if (*m_path == FMQ_PATH_CREATESESSION)
        {
            handled = true;
            sendMessage(getMessageFactory()());
            callback->activity();
        }
        else if (*m_path == FMQ_PATH_REMOVESESSION)
        {
            handled = true;
            callback->disconnected();
            sendMessage(getMessageFactory()());
        }
    }

    return handled;
}



bool ProtocolHttpServer::received(const IStreamConnectionPtr& /*connection*/, const SocketPtr& socket, int bytesToRead)
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
            res = socket->receive(const_cast<char*>(m_receiveBuffer.data() + bytesReceived + m_sizeRemaining), static_cast<int>(bytesToRead - bytesReceived));
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
                res = socket->receive(const_cast<char*>(payload.first + bytesReceived + m_indexFilled), static_cast<int>(bytesToRead - bytesReceived));
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
            checkSessionName();
            auto callback = m_callback.lock();
            if (callback)
            {
                bool handled = handleInternalCommands(callback);
                if (!handled)
                {
                    callback->received(m_message, m_connectionId);
                }
            }
            reset();
        }
    }
    return ok;
}




hybrid_ptr<IStreamConnectionCallback> ProtocolHttpServer::connected(const IStreamConnectionPtr& connection)
{
    ConnectionData connectionData = connection->getConnectionData();
    m_headerHost = "Host: " + connectionData.hostname + ":" + std::to_string(connectionData.port) + "\r\n";
    m_connectionId = connectionData.connectionId;
    // for incomming connection -> do the connection event after checking the session ID
    if (connectionData.incomingConnection)
    {
        //        m_checkSessionName = true;
    }
    return nullptr;
}

void ProtocolHttpServer::disconnected(const IStreamConnectionPtr& connection)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->disconnectedMultiConnection(connection);
    }
}




IMessagePtr ProtocolHttpServer::pollReply(std::deque<IMessagePtr>&& messages)
{
    IMessagePtr message = getMessageFactory()();

    if (m_multipart)
    {
        if (!messages.empty())
        {
            for (auto it = messages.begin(); it != messages.end(); ++it)
            {
                std::string payload;
                if (m_chunkedState == 2)
                {
                    m_chunkedState = 3;
                    payload += "--" + FMQ_MULTIPART_BOUNDARY;
                }
                payload += "\r\n\r\n";
                message->addSendPayload(payload);
                IMessagePtr& msg = *it;
                const std::list<BufferRef>& payloads = msg->getAllSendPayloads();
                std::list<std::string>& payloadBuffers = msg->getSendPayloadBuffers();
                message->moveSendBuffers(std::move(payloadBuffers), payloads);
                payload = "\r\n--" + FMQ_MULTIPART_BOUNDARY;
                message->addSendPayload(payload);
            }
        }
        else
        {
            std::string payload;
            if (m_chunkedState == 2)
            {
                m_chunkedState = 3;
                payload += "--" + FMQ_MULTIPART_BOUNDARY + "--\r\n";
            }
            else
            {
                payload += "--\r\n";
            }
            message->addSendPayload(payload);
        }
    }
    else
    {
        for (auto it = messages.begin(); it != messages.end(); ++it)
        {
            IMessagePtr& msg = *it;
            const std::list<BufferRef>& payloads = msg->getAllSendPayloads();
            std::list<std::string>& payloadBuffers = msg->getSendPayloadBuffers();
            message->moveSendBuffers(std::move(payloadBuffers), payloads);
        }
    }
    return message;
}


void ProtocolHttpServer::cycleTime()
{

}



//---------------------------------------
// ProtocolHttpFactory
//---------------------------------------


struct RegisterProtocolHttpServerFactory
{
    RegisterProtocolHttpServerFactory()
    {
        ProtocolRegistry::instance().registerProtocolFactory(ProtocolHttpServer::PROTOCOL_NAME, ProtocolHttpServer::PROTOCOL_ID, std::make_shared<ProtocolHttpServerFactory>());
    }
} g_registerProtocolHttpServerFactory;


// IProtocolFactory
IProtocolPtr ProtocolHttpServerFactory::createProtocol(const Variant& /*data*/)
{
    return std::make_shared<ProtocolHttpServer>();
}




}   // namespace finalmq
