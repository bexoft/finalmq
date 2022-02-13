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


#include "finalmq/remoteentity/RemoteEntityContainer.h"
#include "finalmq/remoteentity/RemoteEntityFormatJson.h"
#include "finalmq/remoteentity/FmqRegistryClient.h"
#include "finalmq/protocolsession/ProtocolMessage.h"
//#include "finalmq/protocolsession/ProtocolRegistry.h"
#include "finalmq/logger/Logger.h"

#include <thread>
#include <random>
#include <iostream>


#include <fcgiapp.h>

#define MODULENAME "fmqfcgi"

//using finalmq::RemoteEntity;
using finalmq::RemoteEntityContainer;
using finalmq::RemoteEntityFormatJson;
using finalmq::IRemoteEntityContainerPtr;
using finalmq::IRemoteEntityPtr;
using finalmq::RemoteEntity;
using finalmq::PEERID_INVALID;
using finalmq::ENTITYID_INVALID;
using finalmq::PeerId;
using finalmq::EntityId;
using finalmq::CorrelationId;
using finalmq::IProtocolSessionPtr;
using finalmq::IProtocolPtr;
using finalmq::RequestContextPtr;
using finalmq::Logger;
using finalmq::LogContext;
using finalmq::FmqRegistryClient;
using finalmq::Status;
using finalmq::fmqreg::GetServiceReply;
using finalmq::fmqreg::Endpoint;
using finalmq::RawDataMessage;
using finalmq::PeerEvent;
using finalmq::StructBase;
using finalmq::IMessage;


static const int LONGPOLL_RELEASE_INTERVAL = 20000;

static const std::string KEY_REQUEST                = "request";
static const std::string KEY_REPLY                  = "reply";
static const std::string KEY_REMOVESESSION          = "removesession";
static const std::string KEY_SETEXPIRATIONDURATION  = "setexpirationduration";
static const std::string KEY_LONGPOLL               = "longpoll";


static const int CYCLETIME                  = 1000;
static const int CHECK_RECONNECT_INTERVAL   = 1000;
static const int RECONNECT_INTERVAL         = 5000;

static const int DEFAULT_SESSION_EXPIRATION_DURATION = 5 * 60000;   // 5 minutes of inactivity will remove a session.

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
            streamInfo << "Request finish";
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

    inline void headerToMetainfo(IMessage::Metainfo& metainfo)
    {
        for (int i = 0; m_request->envp[i] != NULL; i += 1)
        {
            const char* line = m_request->envp[i];
            const char* pos = strchr(line, '=');
            if (pos)
            {
                metainfo[std::string(line, (size_t)(pos - line))] = std::string(pos + 1);
            }
            else
            {
                metainfo[std::string(line)] = "";
            }
        }
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



class HttpSession : public RemoteEntity
{
public:
    HttpSession(const std::string& httpSessionId)
        : m_httpSessionId(httpSessionId)
    {
        streamInfo << "HTTP session created";

        // register peer events to see when a remote entity connects or disconnects.
        registerPeerEvent([this] (PeerId peerId, PeerEvent peerEvent, bool incoming) {
            if (peerEvent == PeerEvent::PEER_DISCONNECTED)
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                auto it = m_peerId2ObjectName.find(peerId);
                if (it != m_peerId2ObjectName.end())
                {
                    static const std::string DATA = "{}";
                    RawDataMessage rawDataMessage;
                    rawDataMessage.setRawData("finalmq.disconnected", RemoteEntityFormatJson::CONTENT_TYPE, DATA.data(), DATA.size());
                    putRequestEntry(peerId, finalmq::CORRELATIONID_NONE, rawDataMessage);
                }
            }
        });

        registerCommandFunction("*", [this] (RequestContextPtr& requestContext, const finalmq::StructBasePtr& structBase) {
            assert(structBase);
            std::unique_lock<std::mutex> lock(m_mutex);
            finalmq::CorrelationId correlationIdHttp = finalmq::CORRELATIONID_NONE;
            if (requestContext->correlationId() != finalmq::CORRELATIONID_NONE)
            {
                correlationIdHttp = m_nextCorrelationId;
                ++m_nextCorrelationId;
                m_pendingEntityReplies[correlationIdHttp] = std::move(requestContext);
            }
            putRequestEntry(requestContext->peerId(), correlationIdHttp, *structBase);
        });
    }

    ~HttpSession()
    {
        for (auto it = m_objectName2PeerId.begin(); it != m_objectName2PeerId.end(); ++it)
        {
            disconnect(it->second);
        }
        streamInfo << "HTTP session destroyed " << m_httpSessionId;
    }

    void putRequestEntry(PeerId peerId, CorrelationId correlationIdHttp, const StructBase& structBase)
    {
        std::string entry;
        auto it = m_peerId2ObjectName.find(peerId);
        if (it != m_peerId2ObjectName.end())
        {
            std::string objectName = it->second;
            const std::string* type = structBase.getRawType();
            const std::string* rawData = structBase.getRawData();
            assert(type && rawData);
            createRequestEntry(objectName, *type, correlationIdHttp, rawData->data(), rawData->size(), entry);
            m_requestEntries.push_back(entry);

            if (m_longpoll)
            {
                triggerRequest(m_longpoll);
                m_longpoll = nullptr;
            }
        }
    }

    void triggerRequest(const RequestPtr& longpoll)
    {
        streamInfo << "trigger requests";
        for (size_t i = 0; i < m_requestEntries.size(); ++i)
        {
            longpoll->putstr(m_requestEntries[i]);
        }
        m_requestEntries.clear();
    }

    void setLongPoll(const RequestPtr& longpoll, long long durationSecond)
    {
        streamInfo << "Set longpoll";
        std::unique_lock<std::mutex> lock(m_mutex);
        m_longpollDurationMs = durationSecond * 1000;
        m_longpoll = nullptr;
        m_longpollTimer = std::chrono::steady_clock::now();
        if (m_requestEntries.empty() && m_longpollDurationMs > 0)
        {
            streamInfo << "Save longpoll";
            m_longpoll = longpoll;
        }
        else
        {
            triggerRequest(longpoll);
        }
    }

    bool isLongPollExpired()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
        std::chrono::duration<double> dur = now - m_longpollTimer;
        long long delta = static_cast<long long>(dur.count() * 1000);
        bool expired = false;
        if (delta > m_longpollDurationMs)
        {
            m_longpollTimer = now;
            expired = true;
        }
        else if (delta < 0)
        {
            m_longpollTimer = now;
        }
        return expired;
    }

    void releaseLongpoll()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_longpoll = nullptr;
    }

    void createRequestEntry(const std::string& objectName, const std::string& type, CorrelationId correlationId, const char* data, int size, std::string& entry)
    {
        static const std::string STR_HEADER_BEGIN = "[{\"mode\":\"MSG_REQUEST\",\"type\":\"";
        static const std::string STR_HEADER_SRC = "\",\"src\":\"";
        static const std::string STR_HEADER_CORRELATIONID = "\",\"corrid\":\"";
        static const std::string STR_HEADER_END_PAYLOADWILLFOLLOW = "\"},\t";
        static const std::string STR_HEADER_END_NOPAYLOAD = "\"},\t{}]\n";
        static const std::string STR_HEADER_END = "]\n";
        entry = STR_HEADER_BEGIN;
        entry += type;
        entry += STR_HEADER_SRC;
        entry += objectName;
        if (correlationId != finalmq::CORRELATIONID_NONE)
        {
            entry += STR_HEADER_CORRELATIONID;
            entry += std::to_string(correlationId);
        }
        if (size > 0)
        {
            entry += STR_HEADER_END_PAYLOADWILLFOLLOW;
            entry.insert(entry.end(), data, data + size);
            entry += STR_HEADER_END;
        }
        else
        {
            entry += STR_HEADER_END_NOPAYLOAD;
        }
    }

    
    bool getPeerId(const std::string& objectName, PeerId& peerId)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto it = m_objectName2PeerId.find(objectName);
        if (it != m_objectName2PeerId.end())
        {
            peerId = it->second;

            // check connection status. if disconnected -> reconnect
            IProtocolSessionPtr session = getSession(peerId);
            if (session && session->getConnectionData().connectionState != finalmq::ConnectionState::CONNECTIONSTATE_DISCONNECTED)
            {
                return true;
            }
        }

        peerId = createPeer();
        assert(peerId != PEERID_INVALID);

        m_objectName2PeerId[objectName] = peerId;
        m_peerId2ObjectName[peerId] = objectName;

        return false;
    }

    const std::string& getSessionId() const
    {
        return m_httpSessionId;
    }

    void setActivity()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_lastActivityTime = std::chrono::steady_clock::now();
    }

    void setExpirationDuration(long long expirationDurationSecond)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_sessionExpirationDurationMs = expirationDurationSecond * 1000;
    }

    bool isSessionTimeExpired()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
        std::chrono::duration<double> dur = now - m_lastActivityTime;
        long long delta = static_cast<long long>(dur.count() * 1000);
        bool expired = false;
        if (delta > m_sessionExpirationDurationMs)
        {
            m_lastActivityTime = now;
            expired = true;
        }
        else if (delta < 0)
        {
            m_lastActivityTime = now;
        }
        return expired;
    }

    RequestContextPtr getRequestContext(CorrelationId correlationId)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto it = m_pendingEntityReplies.find(correlationId);
        if (it != m_pendingEntityReplies.end())
        {
            return std::move(it->second);
        }
        return nullptr;
    }

private:
    std::unordered_map<std::string, PeerId> m_objectName2PeerId;
    std::unordered_map<PeerId, std::string> m_peerId2ObjectName;
    finalmq::CorrelationId                  m_nextCorrelationId = 1;
    std::string                             m_httpSessionId;

    std::unordered_map<finalmq::CorrelationId, RequestContextPtr> m_pendingEntityReplies;
    std::deque<std::string>                 m_requestEntries;
    RequestPtr                              m_longpoll;
    long long                               m_longpollDurationMs;
    std::chrono::time_point<std::chrono::steady_clock> m_longpollTimer = std::chrono::steady_clock::now();

    std::chrono::time_point<std::chrono::steady_clock> m_lastActivityTime = std::chrono::steady_clock::now();
    long long                               m_sessionExpirationDurationMs = DEFAULT_SESSION_EXPIRATION_DURATION;

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
        m_entityContainer->init(CYCLETIME, CHECK_RECONNECT_INTERVAL, [this] () {
            timeout();
        }, nullptr, true);  // storeRawDataInReceiveStruct = true -> we would like to receive raw data

        m_thread = std::thread([this] () {
            m_entityContainer->run();
        });
    }

    void timeout()
    {
        std::vector<std::string> sessionIdsToRemove;
        std::vector<HttpSessionPtr> sessionToReleaseLongpoll;
        std::unique_lock<std::mutex> lock(m_mutex);
        for (auto it = m_httpSessions.begin(); it != m_httpSessions.end(); ++it)
        {
            const HttpSessionPtr& session = it->second;
            assert(session);
            if (session->isSessionTimeExpired())
            {
                sessionIdsToRemove.push_back(it->first);
            }
            if (session->isLongPollExpired())
            {
                sessionToReleaseLongpoll.push_back(it->second);
            }
        }
        lock.unlock();

        for (size_t i = 0; i < sessionIdsToRemove.size(); ++i)
        {
            removeSession(sessionIdsToRemove[i]);
        }
        for (size_t i = 0; i < sessionToReleaseLongpoll.size(); ++i)
        {
            sessionToReleaseLongpoll[i]->releaseLongpoll();
        }
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


    template <char SEPARATOR, bool DECODE>
    void getQueries(const char* querystring, std::deque<std::pair<std::string, finalmq::BufferRef>>& listQuery)
    {
        const char* str = nullptr;
        do
        {
            str = strchr(querystring, SEPARATOR);
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
            if (DECODE)
            {
                decode(query);
            }
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
            streamInfo << "session found";
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
                const char* pos = strstr(cookie.c_str(), "fmqfcgi");
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

        m_entityContainer->registerEntity(httpSession);

        return httpSession;
    }

    void removeSession(const std::string& httpSessionId)
    {
        streamInfo << "remove session";
        EntityId entityId = ENTITYID_INVALID;
        std::unique_lock<std::mutex> lock(m_mutex);
        auto it = m_httpSessions.find(httpSessionId);
        if (it != m_httpSessions.end())
        {
            entityId = it->second->getEntityId();
            m_httpSessions.erase(it);
        }
        lock.unlock();
        if (entityId != ENTITYID_INVALID)
        {
            m_entityContainer->unregisterEntity(entityId);
        }
    }

    HttpSessionPtr getHttpSession(Request& request, const char* httpHeaderCreateSession, const char* httpHeaderSessionId, const char* cookies)
    {
        HttpSessionPtr httpSession;

        if (httpHeaderSessionId)
        {
            httpSession = findSession(httpHeaderSessionId);
        }
        if (httpHeaderCreateSession || (httpHeaderSessionId && !httpSession))
        {
            if (httpSession)
            {
                removeSession(httpSession->getSessionId());
            }
            httpSession = createHttpSession();
            assert(httpSession);
            FCGX_FPrintF(request->out, "Set-fmqfcgi-Sessionid: %s\r\n", httpSession->getSessionId().c_str());
        }

        if (!httpSession)
        {
            httpSession = findCookieSession(cookies);
            if (!httpSession)
            {
                httpSession = createHttpSession();
                assert(httpSession);
                FCGX_FPrintF(request->out, "Set-Cookie: fmqfcgi=%s\r\n", httpSession->getSessionId().c_str());
            }
        }
        return httpSession;
    }


#if WIN32
    static void* memrchr(const void* m, int c, size_t n)
    {
        const unsigned char* s = (unsigned char*)m;
        c = (unsigned char)c;
        while (n--) if (s[n] == c) return (void*)(s + n);
        return 0;
    }
#endif

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
        streamInfo << "Connect entity " << objectName;
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
                    std::string endpoint;
                    auto it = m_objectName2sessionAndEntity.find(objectName);
                    if (it != m_objectName2sessionAndEntity.end())
                    {
                        SessionAndEntity& sessionAndEntity = it->second;
                        if (reply && reply->found)
                        {
                            IProtocolPtr protocol;
                            sessionAndEntity.entityId = reply->service.entityid;
                            sessionAndEntity.entityName = reply->service.entityname;
                            std::vector<finalmq::fmqreg::Endpoint>& endpointEntries = reply->service.endpoints;
                            for (size_t i = 0; i < endpointEntries.size() && endpoint.empty(); ++i)
                            {
                                const Endpoint& endpointEntry = endpointEntries[i];
                                if (!endpointEntry.ssl &&
                                    endpointEntry.contenttype == RemoteEntityFormatJson::CONTENT_TYPE)
                                {
                                    endpoint = endpointEntry.endpoint;
                                }
                            }
                            if (!endpoint.empty())
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
                                    session = m_entityContainer->connect(endpoint, { {}, {RECONNECT_INTERVAL, 0} });
                                }
                                sessionAndEntity.session = session;
                            }
                        }
                        for (size_t n = 0; n < sessionAndEntity.entities.size(); n++)
                        {
                            const SessionAndEntity::EntityAndPeerId& entityAndPeerId = sessionAndEntity.entities[n];
                            entityAndPeerId.entity->disconnect(entityAndPeerId.peerId);
                        }
                        sessionAndEntity.entities.clear();
                        if (!reply || !reply->found)
                        {
                            m_objectName2sessionAndEntity.erase(it);
                        }
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
        static const std::string STR_HEADER_END_NOPAYLOAD = "\"},\t{}]\n";
        static const std::string STR_HEADER_END = "]\n";
        request.putstr(STR_HEADER_BEGIN);
        request.putstr(type);
        request.putstr(STR_HEADER_STATUS);
        request.putstr(status.toString());

        if (size > 0)
        {
            request.putstr(STR_HEADER_END_PAYLOADWILLFOLLOW);
            request.putstr(data, size);
            request.putstr(STR_HEADER_END);
        }
        else
        {
            request.putstr(STR_HEADER_END_NOPAYLOAD);
        }
    }


    void executeRequest(const HttpSessionPtr& httpSession, const RequestPtr& requestPtr, const finalmq::BufferRef& value)
    {
        streamInfo << "Execute request " << std::string(value.first, value.first + value.second);
        std::string objectName;
        std::string typeName;

        ssize_t posParameters = getRequestData(value, objectName, typeName);
        PeerId peerId;
        bool found = httpSession->getPeerId(objectName, peerId);
        if (!found)
        {
            connectEntity(objectName, peerId, httpSession);
        }

        if (typeName.empty())
        {
            // no type means only connect, but do not execute a request.
            Request& request = *requestPtr;
            sendReply(request, "", Status::STATUS_OK, nullptr, 0);
        }
        else
        {
            RawDataMessage message;
            ssize_t size = value.second - posParameters;
            if (size > 0)
            {
                message.setRawData(typeName, RemoteEntityFormatJson::CONTENT_TYPE, value.first + posParameters, size);
            }
            else
            {
                message.setRawData(typeName, RemoteEntityFormatJson::CONTENT_TYPE, "{}", 2);
            }
            IMessage::Metainfo metainfo;
            requestPtr->headerToMetainfo(metainfo);
            httpSession->sendRequest(peerId, std::move(metainfo), message, [this, requestPtr] (PeerId peerId, Status status, IMessage::Metainfo& metainfo, const std::shared_ptr<StructBase>& reply) {
                assert(requestPtr);
                Request& request = *requestPtr;
                if (reply && reply->getRawContentType() != RemoteEntityFormatJson::CONTENT_TYPE)
                {
                    status = Status::STATUS_WRONG_CONTENTTYPE;
                }
                if (reply && status == Status::STATUS_OK)
                {
                    const std::string* type = reply->getRawType();
                    const std::string* data = reply->getRawData();
                    assert(type && data);
                    sendReply(request, *type, status, data->data(), data->size());
                }
                else
                {
                    sendReply(request, "", status, nullptr, 0);
                }
            });
        }
    }

    void executeReply(const HttpSessionPtr& httpSession, const RequestPtr& requestPtr, const finalmq::BufferRef& value)
    {
        streamInfo << "Execute reply " << std::string(value.first, value.first + value.second);
        std::string strCorrelationId;
        std::string typeName;

        ssize_t posParameters = getRequestData(value, strCorrelationId, typeName);
        CorrelationId correclationId = std::atoll(strCorrelationId.c_str());

        RequestContextPtr requestContext = httpSession->getRequestContext(correclationId);
        if (requestContext)
        {
            RawDataMessage message;
            ssize_t size = value.second - posParameters;
            if (size > 0)
            {
                message.setRawData(typeName, RemoteEntityFormatJson::CONTENT_TYPE, value.first + posParameters, size);
            }
            else
            {
                message.setRawData(typeName, RemoteEntityFormatJson::CONTENT_TYPE, "{}", 2);
            }
            requestContext->reply(message);
        }
    }

    void setExpirationDuration(const HttpSessionPtr& httpSession, const finalmq::BufferRef& value)
    {
        long long duration = std::atoll(std::string(value.first, value.first + value.second).c_str());
        streamInfo << "set expiration time: " << duration << "s";
        httpSession->setExpirationDuration(duration);
    }

    void longpoll(const HttpSessionPtr& httpSession, const RequestPtr& requestPtr, const finalmq::BufferRef& value)
    {
        long long duration = std::atoll(std::string(value.first, value.first + value.second).c_str());
        httpSession->setLongPoll(requestPtr, duration);
    }

    void handleRequest(const RequestPtr& requestPtr)
    {
        // GET http://localhost/root.fmq/MyService/helloworld.getGreetings%7B%22hello%22:%22world%22%7D?call=hello.world{%22hey%22:5}
        // REQUEST_URI: /root.fmq/MyService/helloworld.getGreetings%7B%22hello%22:%22world%22%7D?call=hello.world{%22hey%22:5}
        // PATH_INFO: /MyService/helloworld.getGreetings{"hello":"world"}
        // QUERY_STRING: call=hello.world%7B%22hey%22:5%7D

        Request& request = *requestPtr;

        const char* httpHeaderCookies = FCGX_GetParam("HTTP_COOKIE", request->envp);
        const char* httpHeaderQuerystring = FCGX_GetParam("QUERY_STRING", request->envp);
        const char* httpHeaderPathinfo = FCGX_GetParam("PATH_INFO", request->envp);
        const char* httpHeaderUriEscaped = FCGX_GetParam("REQUEST_URI", request->envp);
        const char* httpHeaderOrigin = FCGX_GetParam("HTTP_ORIGIN", request->envp);

        const char* httpHeaderCreateSession = FCGX_GetParam("HTTP_FMQ_CREATESESSION", request->envp);
        const char* httpHeaderSessionId = FCGX_GetParam("HTTP_FMQ_SESSIONID", request->envp);

        //for(int i=0; request->envp[i] != NULL; i+=1)
        //{
        //    streamInfo << request->envp[i];
        //}

        streamInfo << "----- REQUEST -----";

        if (httpHeaderUriEscaped)
        {
            streamInfo << "REQUEST_URI: " << httpHeaderUriEscaped;
        }
        if (httpHeaderPathinfo)
        {
            streamInfo << "PATH_INFO: " << httpHeaderPathinfo;
        }
        if (httpHeaderQuerystring)
        {
            streamInfo << "QUERY_STRING: " << httpHeaderQuerystring;
        }
        if (httpHeaderCookies)
        {
            streamInfo << "HTTP_COOKIE: " << httpHeaderCookies;
        }
        if (httpHeaderOrigin)
        {
            streamInfo << "HTTP_ORIGIN: " << httpHeaderOrigin;
        }
        if (httpHeaderCreateSession)
        {
            streamInfo << "FMQ_CREATESESSION: " << httpHeaderCreateSession;
        }
        if (httpHeaderSessionId)
        {
            streamInfo << "FMQ_SESSIONID: " << httpHeaderSessionId;
        }

        FCGX_FPrintF(request->out, "Content-type: text/plain; charset=utf-8\r\n");
        FCGX_FPrintF(request->out, "Pragma: no-cache\r\n");
        FCGX_FPrintF(request->out, "Cache-Control: no-cache\r\n");
        FCGX_FPrintF(request->out, "Expires: -1\r\n");
        FCGX_FPrintF(request->out, "Access-Control-Allow-Credentials: true\r\n");
        if (httpHeaderOrigin && httpHeaderOrigin[0] != 0)
        {
            std::string strACAO = "Access-Control-Allow-Origin: ";
            strACAO += httpHeaderOrigin;
            strACAO += "\r\n";
            FCGX_FPrintF(request->out, strACAO.c_str());
        }
        else
        {
            FCGX_FPrintF(request->out, "Access-Control-Allow-Origin: null\r\n");
        }

        HttpSessionPtr httpSession = getHttpSession(request, httpHeaderCreateSession, httpHeaderSessionId, httpHeaderCookies);
        assert(httpSession);
        httpSession->setActivity();


        // end of header
        FCGX_FPrintF(request->out, "\r\n");

        std::deque<std::pair<std::string, finalmq::BufferRef>> listQuery;
        if (httpHeaderQuerystring && httpHeaderQuerystring[0])
        {
            getQueries<'&', true>(httpHeaderQuerystring, listQuery);
        }
        std::string data;
        getData(request, data);
        getQueries<'\n', false>(data.c_str(), listQuery);

        for (size_t i = 0; i < listQuery.size(); ++i)
        {
            const std::string& key = listQuery[i].first;
            const finalmq::BufferRef& value = listQuery[i].second;

            if (key == KEY_REQUEST)
            {
                executeRequest(httpSession, requestPtr, value);
            }
            else if (key == KEY_REPLY)
            {
                executeReply(httpSession, requestPtr, value);
            }
            else if (key == KEY_REMOVESESSION)
            {
                removeSession(httpSession->getSessionId());
            }
            else if (key == KEY_SETEXPIRATIONDURATION)
            {
                setExpirationDuration(httpSession, value);
            }
            else if (key == KEY_LONGPOLL)
            {
                longpoll(httpSession, requestPtr, value);
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

