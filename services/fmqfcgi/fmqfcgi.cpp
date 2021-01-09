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
#include "protocols/ProtocolHeaderBinarySize.h"
#include "protocols/ProtocolDelimiter.h"
#include "logger/Logger.h"

#include <thread>
#include <random>
#include <iostream>

#include <fcgiapp.h>

#define MODULENAME "fmqfcgi"

//using finalmq::RemoteEntity;
using finalmq::RemoteEntityContainer;
using finalmq::IRemoteEntityPtr;
using finalmq::RemoteEntity;
//using finalmq::PeerId;
//using finalmq::PeerEvent;
//using finalmq::ReplyContextUPtr;
//using finalmq::ProtocolHeaderBinarySizeFactory;
//using finalmq::ProtocolDelimiterFactory;
//using finalmq::RemoteEntityContentType;
//using finalmq::IProtocolSessionPtr;
//using finalmq::ConnectionData;
//using finalmq::ConnectionEvent;
using finalmq::Logger;
using finalmq::LogContext;
//using finalmq::fmqreg::RegisterService;
//using finalmq::fmqreg::GetService;
//using finalmq::fmqreg::GetServiceReply;
//using finalmq::fmqreg::Service;



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

private:
    IRemoteEntityPtr m_entity;
};

typedef std::shared_ptr<HttpSession>    HttpSessionPtr;



class RequestManager
{
public:
    RequestManager()
        : m_randomDevice()
        , m_randomGenerator(m_randomDevice())
    {

    }

    ~RequestManager()
    {
        m_thread.join();
    }

    void init()
    {
        m_entityContainer.init();
        m_thread = std::thread([this] () {
            m_entityContainer.run();
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
                        auto it = m_sessions.find(httpSessionId);
                        if (it != m_sessions.end())
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

    HttpSessionPtr createSession(const std::string& httpSessionId)
    {
        HttpSessionPtr session = std::make_shared<HttpSession>();
        std::unique_lock<std::mutex> lock(m_mutex);
        m_sessions[httpSessionId] = session;
        lock.unlock();

        return session;
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

    void handleRequest(Request& request)
    {
        // GET http://localhost/root.fmq/MyService/helloworld.getGreetings%7B%22hello%22:%22world%22%7D?call=hello.world{%22hey%22:5}
        // REQUEST_URI: /root.fmq/MyService/helloworld.getGreetings%7B%22hello%22:%22world%22%7D?call=hello.world{%22hey%22:5}
        // PATH_INFO: /MyService/helloworld.getGreetings{"hello":"world"}
        // QUERY_STRING: call=hello.world%7B%22hey%22:5%7D

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
            httpSession = createSession(httpSessionId);

            IRemoteEntityPtr entity = std::make_shared<RemoteEntity>();
            httpSession->setEntity(entity);
            m_entityContainer.registerEntity(entity);
        }

        static const std::string KEY_REQUEST = "request=";
        for (size_t i = 0; i < listQuery.size(); ++i)
        {
            const std::string& query = listQuery[i];
            if (query.find_first_of(KEY_REQUEST) == 0)
            {
                std::string objectName;
                std::string type;

//                ssize_t posParameter = getRequestData(query, KEY_REQUEST.size(), objectName, type);

            }
        }


        std::string outstr("Content-type: text/html\r\n"
            "\r\n"
            "<html>\n"
            "  <head>\n"
            "    <title>Hello, World</title>\n"
            "  </head>\n"
            "  <body>\n"
            "    <h1>Hello, World</h1>\n"
            "  </body>\n"
            "</html>\n");
        request.putstr(outstr);
    }

private:
    std::random_device                              m_randomDevice;
    std::mt19937                                    m_randomGenerator;
    std::uniform_int_distribution<std::uint64_t>    m_randomVariable;
    std::unordered_map<std::string, HttpSessionPtr> m_sessions;
    std::uint64_t                                   m_nextSessionCounter = 1;

    RemoteEntityContainer                           m_entityContainer;
    std::thread                                     m_thread;

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

    Request request;
    while (request.accept() == 0)
    {
        requestManager.handleRequest(request);
    }

    return 0;
}

