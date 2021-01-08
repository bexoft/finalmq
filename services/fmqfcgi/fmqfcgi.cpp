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
#include <iostream>

#include <fcgiapp.h>


//using finalmq::RemoteEntity;
//using finalmq::RemoteEntityContainer;
//using finalmq::IRemoteEntityContainer;
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




class RequestManager
{
public:

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
        while ((str = strchr(querystring, '&')) != nullptr)
        {
            std::string query(querystring, str);
            decode(query);
            if (!query.empty())
            {
                listQuery.push_back(std::move(query));
            }
            querystring = str + 1;
        }
        std::string query(querystring);
        decode(query);
        if (!query.empty())
        {
            listQuery.push_back(std::move(query));
        }
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
            std::cerr << "REQUEST_URI: " << uriEscaped << std::endl;
        }
        if (pathinfo)
        {
            std::cerr << "PATH_INFO: " << pathinfo << std::endl;
        }
        if (querystring)
        {
            std::cerr << "QUERY_STRING: " << querystring << std::endl;
        }
        if (cookies)
        {
            std::cerr << "HTTP_COOKIE: " << cookies << std::endl;
        }
        if (origin)
        {
            std::cerr << "HTTP_ORIGIN: " << origin << std::endl;
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

        for (size_t i = 0; i < listQuery.size(); ++i)
        {
            std::cerr << listQuery[i] << std::endl;
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
};


int main(void)
{
    FCGX_Init();
    RequestManager requestManager;

    Request request;
    while (request.accept() == 0)
    {
        requestManager.handleRequest(request);
    }

    return 0;
}

