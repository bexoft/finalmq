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

#include "remoteentity/RemoteEntityContainer.h"
#include "remoteentity/FmqRegistryClient.h"
#include "protocols/ProtocolHeaderBinarySize.h"
#include "protocols/ProtocolDelimiter.h"
#include "protocolconnection/ProtocolMessage.h"
#include "logger/Logger.h"

#include <thread>
#include <random>
#include <iostream>

#include <fcgiapp.h>

#define MODULENAME "fmqfcgi"

//using finalmq::RemoteEntity;
using finalmq::RemoteEntityContainer;
using finalmq::IRemoteEntityContainerPtr;
using finalmq::IRemoteEntityPtr;
using finalmq::RemoteEntity;
using finalmq::PEERID_INVALID;
using finalmq::ENTITYID_INVALID;
using finalmq::PeerId;
using finalmq::EntityId;
using finalmq::IProtocolSessionPtr;
using finalmq::IProtocolPtr;
using finalmq::ProtocolDelimiter;
using finalmq::ProtocolHeaderBinarySize;
using finalmq::Logger;
using finalmq::LogContext;
using finalmq::FmqRegistryClient;
using finalmq::remoteentity::Status;
using finalmq::fmqreg::GetServiceReply;
using finalmq::fmqreg::Endpoint;
using finalmq::remoteentity::GenericMessage;
using finalmq::RemoteEntityContentType;


static const int LONGPOLL_RELEASE_INTERVAL = 20000;

static const std::string KEY_REQUEST            = "request";
static const std::string KEY_CREATESESSION      = "createsession";
static const std::string KEY_SESSIONID          = "sessionid";


class Request
{
public:
    inline Request()
    {
    }
    inline Request(Request&& rhs)
        : m_request(rhs.m_request)
    {
        rhs.m_request = nullptr;
    }
    inline const Request& operator =(Request&& rhs)
    {
        if (this != &rhs)
        {
            m_request = rhs.m_request;
            rhs.m_request = nullptr;
        }
        return *this;
    }
    inline ~Request()
    {
        finish();
    }

    inline void finish()
    {
        if (m_request)
        {
            FCGX_Finish_r(m_request);
            m_request = nullptr;
        }
    }

    inline int accept()
    {
        if (!m_request)
        {
            m_request = new FCGX_Request;
            FCGX_InitRequest(m_request, 0, 0);
        }
        return FCGX_Accept_r(m_request);
    }

    inline operator bool() const
    {
        return (m_request != nullptr);
    }

    inline void putstr(const char* data, ssize_t size)
    {
        assert(m_request);
        FCGX_PutStr(data, size, m_request->out);
    }

    inline void putstr(const std::string& data)
    {
        assert(m_request);
        FCGX_PutStr(data.c_str(), data.size(), m_request->out);
    }

    inline FCGX_Request* operator ->()
    {
        assert(m_request);
        return m_request;
    }

private:
    Request(const Request&) = delete;
    const Request& operator =(const Request&) = delete;

    FCGX_Request*   m_request = nullptr;
};
typedef std::shared_ptr<Request>    RequestPtr;

struct SessionAndEntity
{
    struct EntityAndPeerId
    {
        IRemoteEntityPtr    entity;
        PeerId              peerId{PEERID_INVALID};
    };
    IProtocolSessionPtr             session;
    EntityId                        entityId{ENTITYID_INVALID};
    std::string                     entityName;
    std::deque<EntityAndPeerId>     entities;
};


class CallbackChannel
{
public:
    CallbackChannel()
        : m_lastActivityTime(std::chrono::system_clock::now())
    {
    }

    void setRequest(const std::shared_ptr<Request>& request)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_lastActivityTime = std::chrono::system_clock::now();
        m_request = request;
    }

    void addRequestString(std::string&& str)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_requestStrings.push_back(std::move(str));
    }

    std::deque<std::string> getAllRequestStrings()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return std::move(m_requestStrings);
    }

    std::shared_ptr<Request> getRequest()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return std::move(m_request);
    }

    bool isActivityTimerExpired()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        bool expired = false;
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        std::chrono::duration<double> dur = now - m_lastActivityTime;
        int delta = static_cast<int>(dur.count() * 1000);
        if (delta < 0 || delta >= LONGPOLL_RELEASE_INTERVAL)
        {
            m_lastActivityTime = now;
            expired = true;
        }

        return expired;
    }

private:
    std::shared_ptr<Request>                            m_request;
    std::deque<std::string>                             m_requestStrings;
    std::chrono::time_point<std::chrono::system_clock>  m_lastActivityTime;
    std::mutex                                          m_mutex;
};


class HttpSession
{
public:
    HttpSession(const std::string& httpSessionId)
        : m_httpSessionId(httpSessionId)
    {
    }

    void setEntity(const IRemoteEntityPtr& entity)
    {
        m_entity = entity;
    }

    IRemoteEntityPtr getEntity() const
    {
        return m_entity;
    }

    bool getPeerId(const std::string& objectName, PeerId& peerId)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto it = m_objectName2PeerId.find(objectName);
        if (it != m_objectName2PeerId.end())
        {
            peerId = it->second;

            // check connection status. if disconnected -> reconnect
            IProtocolSessionPtr session = m_entity->getSession(peerId);
            if (session && session->getConnectionData().connectionState != finalmq::ConnectionState::CONNECTIONSTATE_DISCONNECTED)
            {
                return true;
            }
        }

        peerId = m_entity->createPeer();
        assert(peerId != PEERID_INVALID);

        m_objectName2PeerId[objectName] = peerId;

        return false;
    }

    std::string createCallbackChannel()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        int callbackChannelId = m_nextCallbackChannelId;
        ++m_nextCallbackChannelId;
        std::string strCallbackChannelId = std::to_string(callbackChannelId);
        m_callbackChannels[strCallbackChannelId];
        return strCallbackChannelId;
    }

    const std::string& getSessionId() const
    {
        return m_httpSessionId;
    }

private:
    IRemoteEntityPtr                        m_entity;
    std::unordered_map<std::string, PeerId> m_objectName2PeerId;
    std::unordered_map<std::string, CallbackChannel> m_callbackChannels;
    std::uint64_t                           m_nextCallbackChannelId = 1;
    std::string                             m_httpSessionId;
    std::mutex                              m_mutex;
};

typedef std::shared_ptr<HttpSession>    HttpSessionPtr;





class RequestManager
{
public:
    RequestManager()
        : m_randomDevice()
        , m_randomGenerator(m_randomDevice())
        , m_entityContainer(std::make_shared<RemoteEntityContainer>())
        , m_fmqRegistryClient(m_entityContainer)
    {

    }

    ~RequestManager()
    {
        m_thread.join();
    }

    void init()
    {
        m_entityContainer->init(100, 1000, [this] () {
            timeout();
        });
        m_thread = std::thread([this] () {
            m_entityContainer->run();
        });
    }

    void timeout()
    {

    }

    static void decode(finalmq::BufferRef& str)
    {
        const char* src = str.first;
        char* dest = str.first;
        char code[3] = {0};
        unsigned long c = 0;

        for (int i = 0; i < str.second; ++i)
        {
            if (*src == '%')
            {
                ++src;
                memcpy(code, src, 2);
                c = strtoul(code, NULL, 16);
                *dest = (char)c;
                src += 2;
                dest++;
            }
            else
            {
                *dest = *src;
                ++dest;
                ++src;
            }
        }
        str.second -= src - dest;
    }

    void getQueries(const char* querystring, std::deque<std::pair<std::string, finalmq::BufferRef>>& listQuery)
    {
        const char* str = nullptr;
        do
        {
            str = strchr(querystring, '&');
            finalmq::BufferRef query;
            if (str)
            {
                query = {const_cast<char*>(querystring), str - querystring};
                querystring = str + 1;
            }
            else
            {
                query = {const_cast<char*>(querystring), strlen(querystring)};
            }
            decode(query);
            if (query.second > 0)
            {
                char* eq = static_cast<char*>(memchr(query.first, '=', query.second));
                if (eq)
                {
                    int posEq = eq - query.first;
                    listQuery.emplace_back(std::string{query.first, eq}, finalmq::BufferRef{eq+1, query.second - posEq - 1});
                }
                else
                {
                    listQuery.emplace_back(std::string{query.first, query.first + query.second}, finalmq::BufferRef{nullptr, 0});
                }
            }
        } while (str != nullptr);
    }

    void getQueriesFromData(const char* querystring, std::deque<std::pair<std::string, finalmq::BufferRef>>& listQuery)
    {
        const char* str = nullptr;
        do
        {
            str = strchr(querystring, '\n');
            finalmq::BufferRef query;
            if (str)
            {
                query = {const_cast<char*>(querystring), str - querystring};
                querystring = str + 1;
            }
            else
            {
                query = {const_cast<char*>(querystring), strlen(querystring)};
            }
            if (query.second > 0)
            {
                listQuery.emplace_back("request", query);
            }
        } while (str != nullptr);
    }

    void getData(Request& request, std::string& data)
    {
        static const int DATABLOCKSIZE = 1000;
        int cnt = 0;
        do
        {
            int startIx = data.size();
            data.resize(startIx + DATABLOCKSIZE);
            cnt = FCGX_GetStr(&data[startIx], DATABLOCKSIZE, request->in);
        } while (cnt == DATABLOCKSIZE);
        data.resize(data.size() - DATABLOCKSIZE + cnt);
    }

    HttpSessionPtr findSession(const std::string& httpSessionId)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto it = m_httpSessions.find(httpSessionId);
        if (it != m_httpSessions.end())
        {
            return it->second;
        }
        return nullptr;
    }
    HttpSessionPtr findCookieSession(const char* cookies)
    {
        HttpSessionPtr session;
        if (cookies)
        {
            const char* str = nullptr;
            do
            {
                str = strchr(cookies, ';');
                std::string cookie;
                if (str)
                {
                    cookie = {cookies, str};
                    cookies = str + 1;
                }
                else
                {
                    cookie = cookies;
                }
                const char* pos = strstr(cookie.c_str(), "sessionid-fmqfcgi");
                if (pos)
                {
                    size_t p = cookie.find('=');
                    if (p != std::string::npos)
                    {
                        std::string httpSessionId = &cookie.c_str()[p + 1];
                        session = findSession(httpSessionId);
                    }
                }
            } while (str != nullptr && !session);
        }
        return session;
    }

    std::string createHttpSessionId()
    {
        std::uint64_t sessionCounter = m_nextSessionCounter;
        ++m_nextSessionCounter;
        std::uint64_t v1 = m_randomVariable(m_randomGenerator);
        std::uint64_t v2 = m_randomVariable(m_randomGenerator);

        v1 &= 0xffffffff00000000ull;
        v1 |= (sessionCounter & 0x00000000ffffffffull);

        std::ostringstream oss;
        oss << std::hex << v2 << v1;
        std::string httpSessionId = oss.str();
        return httpSessionId;
    }

    HttpSessionPtr createHttpSession()
    {
        streamInfo << "create session";
        std::string httpSessionId = createHttpSessionId();

        HttpSessionPtr httpSession = std::make_shared<HttpSession>(httpSessionId);
        std::unique_lock<std::mutex> lock(m_mutex);
        m_httpSessions[httpSessionId] = httpSession;
        lock.unlock();

        IRemoteEntityPtr entity = std::make_shared<RemoteEntity>();
        httpSession->setEntity(entity);
        m_entityContainer->registerEntity(entity);

        return httpSession;
    }

    HttpSessionPtr getHttpSession(Request& request, bool createSession, const std::string& httpSessionId, const char* cookies)
    {
        HttpSessionPtr httpSession;
        if (createSession)
        {
            httpSession = createHttpSession();
            assert(httpSession);
        }
        else if (!httpSessionId.empty())
        {
            httpSession = findSession(httpSessionId);
        }
        if (!httpSession)
        {
            httpSession = findCookieSession(cookies);
            if (!httpSession)
            {
                httpSession = createHttpSession();
                assert(httpSession);
                FCGX_FPrintF(request->out, "Set-Cookie: sessionid-fmqfcgi=%s\r\n", httpSession->getSessionId().c_str());
            }
        }
        return httpSession;
    }

    static ssize_t getRequestData(const finalmq::BufferRef& value, std::string& objectName, std::string& type)
    {
        // 01234567890123456789012345678901234
        // MyServer/test.TestRequest{}
        char* strEndHeader = static_cast<char*>(memchr(value.first, '{', value.second));   //25
        ssize_t ixEndHeader = value.second;
        if (strEndHeader != nullptr)
        {
            ixEndHeader = strEndHeader - value.first;
        }

        char* strStartCommand = static_cast<char*>(memrchr(value.first, '/', ixEndHeader));
        if (strStartCommand != nullptr)
        {
            objectName = {value.first, strStartCommand};
            type = {strStartCommand + 1, value.first + ixEndHeader};
        }

        return ixEndHeader;
    }

    void connectEntity(const std::string& objectName, PeerId peerId, const IRemoteEntityPtr& entity)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto it = m_objectName2sessionAndEntity.find(objectName);
        bool found = (it != m_objectName2sessionAndEntity.end());
        if (found && it->second.session && it->second.session->getConnectionData().connectionState == finalmq::ConnectionState::CONNECTIONSTATE_DISCONNECTED)
        {
            found = false;
            m_objectName2sessionAndEntity.erase(objectName);
            it = m_objectName2sessionAndEntity.end();
        }
        SessionAndEntity& sessionAndEntity = (found) ? it->second : m_objectName2sessionAndEntity[objectName];

        if (sessionAndEntity.session)
        {
            entity->connect(peerId, sessionAndEntity.session, sessionAndEntity.entityName, sessionAndEntity.entityId);
        }
        else
        {
            sessionAndEntity.entities.push_back({entity, peerId});
            lock.unlock();

            if (!found)
            {
                m_fmqRegistryClient.getService(objectName, [this, objectName] (Status status, const std::shared_ptr<GetServiceReply>& reply) {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    IProtocolPtr protocol;
                    std::string endpoint;
                    auto it = m_objectName2sessionAndEntity.find(objectName);
                    if (it != m_objectName2sessionAndEntity.end())
                    {
                        SessionAndEntity& sessionAndEntity = it->second;
                        if (reply)
                        {
                            sessionAndEntity.entityId = reply->service.entityid;
                            sessionAndEntity.entityName = reply->service.entityname;
                            std::vector<finalmq::fmqreg::Endpoint>& endpointEntries = reply->service.endpoints;
                            for (size_t i = 0; i < endpointEntries.size() && !protocol; ++i)
                            {
                                const Endpoint& endpointEntry = endpointEntries[i];
                                endpoint = endpointEntry.endpoint;
                                if (!endpointEntry.ssl &&
                                    endpointEntry.contenttype == finalmq::RemoteEntityContentType::CONTENTTYPE_JSON)
                                {
                                    if (endpointEntry.framingprotocol == ProtocolDelimiter::PROTOCOL_ID)
                                    {
                                        protocol = std::make_shared<ProtocolDelimiter>("\n");
                                    }
                                    else if (endpointEntry.framingprotocol == ProtocolHeaderBinarySize::PROTOCOL_ID)
                                    {
                                        protocol = std::make_shared<ProtocolHeaderBinarySize>();
                                    }
                                }
                            }
                            if (protocol)
                            {
                                IProtocolSessionPtr& session = m_endpoint2session[endpoint];
                                if (!session || session->getConnectionData().connectionState == finalmq::ConnectionState::CONNECTIONSTATE_DISCONNECTED)
                                {
                                    // replace * by hostname in endpoint
                                    std::string::size_type pos = endpoint.find("*");
                                    if (pos != std::string::npos)
                                    {
                                        endpoint.replace(pos, std::string("*").length(), "127.0.0.1");
                                    }
                                    session = m_entityContainer->connect(endpoint, protocol, finalmq::RemoteEntityContentType::CONTENTTYPE_JSON, {{}, 5000, 0});
                                }
                                sessionAndEntity.session = session;
                                for (size_t n = 0; n < sessionAndEntity.entities.size(); n++)
                                {
                                    const SessionAndEntity::EntityAndPeerId& entityAndPeerId = sessionAndEntity.entities[n];
                                    entityAndPeerId.entity->connect(entityAndPeerId.peerId, session, reply->service.entityname, reply->service.entityid);
                                }
                                sessionAndEntity.entities.clear();
                            }
                        }
                        for (size_t n = 0; n < sessionAndEntity.entities.size(); n++)
                        {
                            const SessionAndEntity::EntityAndPeerId& entityAndPeerId = sessionAndEntity.entities[n];
                            entityAndPeerId.entity->disconnect(entityAndPeerId.peerId);
                        }
                        sessionAndEntity.entities.clear();
                    }
                });
            }
        }
    }

    void sendReply(Request& request, const std::string& type, Status status, const char* data, int size)
    {
        static const std::string STR_HEADER_BEGIN = "[{\"mode\":\"MSG_REPLY\",\"type\":\"";
        static const std::string STR_HEADER_STATUS = "\",\"status\":\"";
        static const std::string STR_HEADER_END_PAYLOADWILLFOLLOW = "\"},\t";
        static const std::string STR_HEADER_END = "\"},\t{}]\n";
        request.putstr(STR_HEADER_BEGIN);
        request.putstr(type);
        request.putstr(STR_HEADER_STATUS);
        request.putstr(status.toString());

        if (data)
        {
            request.putstr(STR_HEADER_END_PAYLOADWILLFOLLOW);
            request.putstr(data, size);
            request.putstr("]\n", 2);
        }
        else
        {
            request.putstr(STR_HEADER_END);
        }
    }


    void executeRequest(const HttpSessionPtr& httpSession, const RequestPtr& requestPtr, const finalmq::BufferRef& value)
    {
        std::string objectName;
        std::string typeName;

        ssize_t posParameters = getRequestData(value, objectName, typeName);
        PeerId peerId;
        bool found = httpSession->getPeerId(objectName, peerId);
        if (!found)
        {
            connectEntity(objectName, peerId, httpSession->getEntity());
        }

        GenericMessage message;
        message.type = typeName;
        message.contenttype = RemoteEntityContentType::CONTENTTYPE_JSON;
        message.data.resize(value.second - posParameters);
        if (!message.data.empty())
        {
            memcpy(message.data.data(), value.first + posParameters, message.data.size());
        }
        else
        {
            message.data = {'{','}'};
        }
        httpSession->getEntity()->requestReply<GenericMessage>(peerId, message, [this, requestPtr] (PeerId peerId, Status status, const std::shared_ptr<GenericMessage>& reply) {
            assert(requestPtr);
            Request& request = *requestPtr;
            if (reply && reply->contenttype != RemoteEntityContentType::CONTENTTYPE_JSON)
            {
                status = Status::STATUS_WRONG_CONTENTTYPE;
            }
            if (reply && status == Status::STATUS_OK)
            {
                sendReply(request, reply->type, status, reply->data.data(), reply->data.size());
            }
            else
            {
                sendReply(request, "", status, nullptr, 0);
            }
        });
    }

    bool getSessionInfo(const std::deque<std::pair<std::string, finalmq::BufferRef>>& listQuery, std::string& httpSessionId)
    {
        httpSessionId.clear();
        for (auto it = listQuery.begin(); it != listQuery.end(); ++it)
        {
            if (it->first == KEY_CREATESESSION)
            {
                return true;
            }
            else if (it->first == KEY_SESSIONID)
            {
                httpSessionId = {it->second.first, it->second.first + it->second.second};
            }
        }
        return false;
    }

    void handleRequest(const RequestPtr& requestPtr)
    {
        // GET http://localhost/root.fmq/MyService/helloworld.getGreetings%7B%22hello%22:%22world%22%7D?call=hello.world{%22hey%22:5}
        // REQUEST_URI: /root.fmq/MyService/helloworld.getGreetings%7B%22hello%22:%22world%22%7D?call=hello.world{%22hey%22:5}
        // PATH_INFO: /MyService/helloworld.getGreetings{"hello":"world"}
        // QUERY_STRING: call=hello.world%7B%22hey%22:5%7D

        Request& request = *requestPtr;

        const char* cookies = FCGX_GetParam("HTTP_COOKIE", request->envp);
        const char* querystring = FCGX_GetParam("QUERY_STRING", request->envp);
        const char* pathinfo = FCGX_GetParam("PATH_INFO", request->envp);
        const char* uriEscaped = FCGX_GetParam("REQUEST_URI", request->envp);
        const char* origin = FCGX_GetParam("HTTP_ORIGIN", request->envp);

        if (uriEscaped)
        {
            streamInfo << "REQUEST_URI: " << uriEscaped;
        }
        if (pathinfo)
        {
            streamInfo << "PATH_INFO: " << pathinfo;
        }
        if (querystring)
        {
            streamInfo << "QUERY_STRING: " << querystring;
        }
        if (cookies)
        {
            streamInfo << "HTTP_COOKIE: " << cookies;
        }
        if (origin)
        {
            streamInfo << "HTTP_ORIGIN: " << origin;
        }

        FCGX_FPrintF(request->out, "Content-type: text/plain; charset=utf-8\r\n");
        FCGX_FPrintF(request->out, "Pragma: no-cache\r\n");
        FCGX_FPrintF(request->out, "Cache-Control: no-cache\r\n");
        FCGX_FPrintF(request->out, "Expires: -1\r\n");
        FCGX_FPrintF(request->out, "Access-Control-Allow-Credentials: true\r\n");
        if (origin && origin[0] != 0)
        {
            std::string strACAO = "Access-Control-Allow-Origin: ";
            strACAO += origin;
            strACAO += "\r\n";
            FCGX_FPrintF(request->out, strACAO.c_str());
        }
        else
        {
            FCGX_FPrintF(request->out, "Access-Control-Allow-Origin: null\r\n");
        }

        std::deque<std::pair<std::string, finalmq::BufferRef>> listQuery;
        if (querystring && querystring[0])
        {
            getQueries(querystring, listQuery);
        }

        std::string data;
        getData(request, data);
        getQueriesFromData(data.c_str(), listQuery);

        std::string httpSessionId;
        bool createSession = getSessionInfo(listQuery, httpSessionId);

        HttpSessionPtr httpSession = getHttpSession(request, createSession, httpSessionId, cookies);
        assert(httpSession);

        // end of header
        FCGX_FPrintF(request->out, "\r\n");

        for (size_t i = 0; i < listQuery.size(); ++i)
        {
            const std::string& key = listQuery[i].first;
            const finalmq::BufferRef& value = listQuery[i].second;

            if (key == KEY_REQUEST)
            {
                executeRequest(httpSession, requestPtr, value);
            }
            else
            {

            }
        }
    }

private:
    std::random_device                              m_randomDevice;
    std::mt19937                                    m_randomGenerator;
    std::uniform_int_distribution<std::uint64_t>    m_randomVariable;
    std::unordered_map<std::string, HttpSessionPtr> m_httpSessions;
    std::uint64_t                                   m_nextSessionCounter = 1;

    IRemoteEntityContainerPtr                       m_entityContainer;
    FmqRegistryClient                               m_fmqRegistryClient;
    std::thread                                     m_thread;
    std::unordered_map<std::string, SessionAndEntity> m_objectName2sessionAndEntity;
    std::unordered_map<std::string, IProtocolSessionPtr> m_endpoint2session;

    std::mutex                                      m_mutex;
};


int main(void)
{
    // display log traces
    Logger::instance().registerConsumer([] (const LogContext& context, const char* text) {
        std::cerr << context.filename << "(" << context.line << ") " << text << std::endl;
    });

    FCGX_Init();
    RequestManager requestManager;
    requestManager.init();

    RequestPtr request = std::make_shared<Request>();
    while (request->accept() == 0)
    {
        requestManager.handleRequest(request);
        request = std::make_shared<Request>();
    }

    return 0;
}

