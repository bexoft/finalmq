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



class HttpSession
{
public:
    HttpSession()
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
            return true;
        }

        peerId = m_entity->createPeer();
        assert(peerId != PEERID_INVALID);

        m_objectName2PeerId[objectName] = peerId;

        return false;
    }


private:
    IRemoteEntityPtr                        m_entity;
    std::unordered_map<std::string, PeerId> m_objectName2PeerId;
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
        m_entityContainer->init();
        m_thread = std::thread([this] () {
            m_entityContainer->run();
        });
    }

    static void decode(std::string& str)
    {
        const char* src = str.c_str();
        char* dest = const_cast<char*>(str.c_str());
        char code[3] = {0};
        unsigned long c = 0;

        while (*src)
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
        str.resize(dest - str.c_str());
    }

    void getQueries(const char* querystring, std::vector<std::string>& listQuery)
    {
        const char* str = nullptr;
        do
        {
            str = strchr(querystring, '&');
            std::string query;
            if (str)
            {
                query = {querystring, str};
                querystring = str + 1;
            }
            else
            {
                query = querystring;
            }
            decode(query);
            if (!query.empty())
            {
                listQuery.push_back(std::move(query));
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

    HttpSessionPtr findSession(const char* cookies)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
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
                        auto it = m_httpSessions.find(httpSessionId);
                        if (it != m_httpSessions.end())
                        {
                            session = it->second;
                        }
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

    HttpSessionPtr createHttpSession(const std::string& httpSessionId)
    {
        HttpSessionPtr httpSession = std::make_shared<HttpSession>();
        std::unique_lock<std::mutex> lock(m_mutex);
        m_httpSessions[httpSessionId] = httpSession;
        lock.unlock();

        return httpSession;
    }

    static ssize_t getRequestData(const std::string& query, ssize_t posValue, std::string& objectName, std::string& type)
    {
        // 01234567890123456789012345678901234
        // request=MyServer/test.TestRequest{}
        ssize_t ixEndHeader = query.find_first_of('{');   //33
        if (ixEndHeader == -1)
        {
            ixEndHeader = query.size();
        }
//                endHeader = &buffer[ixEndHeader];
        ssize_t ixStartCommand = query.find_last_of('/', ixEndHeader);
        assert(ixStartCommand >= 0);
        if (ixStartCommand != -1)
        {
            objectName = {&query[posValue], &query[ixStartCommand]};
        }
        type = {&query[ixStartCommand+1], &query[ixEndHeader]};

        return ixEndHeader;
    }

    void connectEntity(const std::string& objectName, PeerId peerId, const IRemoteEntityPtr& entity)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto it = m_objectName2sessionAndEntity.find(objectName);
        bool found = (it != m_objectName2sessionAndEntity.end());
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

    void sendReplyHeader(Request& request, Status status)
    {
        FCGX_FPrintF(request->out, "{\"mode\":\"MSG_REPLY\",\"status\":\"%s\"}\t", status.toString().c_str());
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

        std::vector<std::string> listQuery;
        if (querystring && querystring[0])
        {
            getQueries(querystring, listQuery);
        }

        std::string data;
        getData(request, data);
        getQueries(data.c_str(), listQuery);

        HttpSessionPtr httpSession = findSession(cookies);
        if (httpSession)
        {
            streamInfo << "session found";
        }
        else
        {
            streamInfo << "create session";
            std::string httpSessionId = createHttpSessionId();
            FCGX_FPrintF(request->out, "Set-Cookie: sessionid-fmqfcgi=%s\r\n", httpSessionId.c_str());
            httpSession = createHttpSession(httpSessionId);

            IRemoteEntityPtr entity = std::make_shared<RemoteEntity>();
            httpSession->setEntity(entity);
            m_entityContainer->registerEntity(entity);
        }

        assert(httpSession);

        // end of header
        FCGX_FPrintF(request->out, "\r\n");

        static const std::string KEY_REQUEST = "request=";
        for (size_t i = 0; i < listQuery.size(); ++i)
        {
            const std::string& query = listQuery[i];
            if (query.find_first_of(KEY_REQUEST) == 0)
            {
                std::string objectName;
                std::string typeName;

                ssize_t posParameter = getRequestData(query, KEY_REQUEST.size(), objectName, typeName);
                PeerId peerId;
                bool found = httpSession->getPeerId(objectName, peerId);
                if (!found)
                {
                    connectEntity(objectName, peerId, httpSession->getEntity());
                }

                finalmq::IMessagePtr message = std::make_shared<finalmq::ProtocolMessage>(0);
                ssize_t sizePayload = query.size() - posParameter;
                char* payload = message->addSendPayload(sizePayload);
                memcpy(payload, &query[posParameter], sizePayload);
                httpSession->getEntity()->sendRequest(peerId, typeName, message, [this, requestPtr] (PeerId peerId, Status status, const finalmq::BufferRef& payload) {
                    assert(requestPtr);
                    Request& request = *requestPtr;
                    sendReplyHeader(request, status);
                    if (payload.first)
                    {
                        char* start = strchr(payload.first, '\t');
                        if (start != nullptr)
                        {
                            ++start;
                            ssize_t offset = start - payload.first;
                            request.putstr(start, payload.second - offset);
                        }
                    }
                });
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

