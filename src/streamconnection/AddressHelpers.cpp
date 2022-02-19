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

#include "finalmq/streamconnection/AddressHelpers.h"
#include "finalmq/logger/LogStream.h"
#include "finalmq/helpers/ModulenameFinalmq.h"
#include <assert.h>

#if defined(WIN32) || defined(__MINGW32__)
#pragma warning(disable: 4996)
#else
#include <sys/un.h>
#include <netdb.h>
#endif

#ifdef __QNX__
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <iostream>


namespace finalmq {

int AddressHelpers::parseEndpoint(const std::string& endpoint, std::string& protocol, std::string& address)
{
    if (endpoint.empty())
    {
        errno = EINVAL;
        return -1;
    }

    std::string::size_type pos = endpoint.find("://");
    if (pos == std::string::npos)
    {
        errno = EINVAL;
        return -1;
    }

    protocol = endpoint.substr(0, pos);
    address = endpoint.substr(pos + 3);

    if (protocol.empty() || address.empty())
    {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

int AddressHelpers::parseTcpAddress(const std::string& address, std::string& hostname, int& port)
{
    if (address.empty())
    {
        errno = EINVAL;
        return -1;
    }

    std::string::size_type pos = address.find(":");
    if (pos == std::string::npos)
    {
        errno = EINVAL;
        return -1;
    }

    hostname = address.substr(0, pos);
    std::string strPort = address.substr(pos + 1);
    port = atoi(strPort.c_str());

    if (hostname.empty() || strPort.empty())
    {
        errno = EINVAL;
        return -1;
    }

    return 0;
}


ConnectionData AddressHelpers::endpoint2ConnectionData(const std::string& endpoint)
{
    ConnectionData connectionData;

    std::string protocol;
    std::string address;
    int ret = parseEndpoint(endpoint, protocol, address);
    if (ret == 0)
    {
        if (protocol == "tcp")
        {
            std::string hostname;
            int port = -1;
            ret = parseTcpAddress(address, hostname, port);
            connectionData.endpoint = endpoint;
            connectionData.hostname = hostname;
            connectionData.port = port;
            connectionData.af = AF_INET;
            connectionData.type = SOCK_STREAM;
            connectionData.protocol = IPPROTO_TCP;
        }
#ifndef WIN32
        else if (protocol == "ipc")
        {
            connectionData.endpoint = endpoint;
            connectionData.hostname = address;
            connectionData.af = AF_UNIX;
            connectionData.type = SOCK_STREAM;
            connectionData.protocol = 0;
        }
#endif
        else
        {
            streamError << "unknown protocol " << protocol;
            ret = -1;
        }
    }

    if (ret != 0)
    {
        streamError << "invalid endpoint " << endpoint;
    }

    return connectionData;
}




std::string AddressHelpers::makeSocketAddress(const std::string& hostname, int port, int af, bool asyncGetHostByName, bool& doAsyncGetHostByName)
{
    const sockaddr* addr = nullptr;
    int addrlen = 0;
    doAsyncGetHostByName = false;

    struct sockaddr_in addrTcp;
#ifndef WIN32
    struct sockaddr_un addrUnix;
#endif
    switch (af)
    {
    case AF_INET:
        {
            assert(!hostname.empty());
            addrlen = 0;
            std::string hname = hostname;
            if (hname == "*")
            {
                hname = "0.0.0.0";
            }
            if (asyncGetHostByName)
            {
                doAsyncGetHostByName = true;
            }
            else
            {
                addrlen = sizeof(addrTcp);
                struct hostent* hp = gethostbyname(hname.c_str());
                if (hp)
                {
                    memset(&addrTcp, 0, addrlen);
                    addrTcp.sin_family = af;
                    addrTcp.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
                    addrTcp.sin_port = htons(port);
                    addr = (sockaddr*)&addrTcp;
                }
            }
        }
        break;
#ifndef WIN32
    case AF_UNIX:
        addrlen = sizeof(addrUnix);
        memset(&addrUnix, 0, sizeof(addrUnix));
        addrUnix.sun_family = af;
        strcpy(addrUnix.sun_path, hostname.c_str());
        addr = (sockaddr*)&addrUnix;
        break;
#endif
    default:
        streamError << "protocol not supported";
        break;
    }
    if (addr)
    {
        return std::string((char*)addr, addrlen);
    }
    else
    {
        return "";
    }
}



void AddressHelpers::addr2peer(sockaddr* addr, ConnectionData& connectionData)
{
    switch (addr->sa_family)
    {
#ifndef WIN32
    case AF_UNIX:
        {
            struct sockaddr_un* a = (struct sockaddr_un*)addr;
            std::string endpoint = "icp://";
            endpoint += a->sun_path;
            connectionData.endpointPeer = std::move(endpoint);
            connectionData.addressPeer = a->sun_path;
            connectionData.portPeer = 0;
        }
        break;
#endif
    case AF_INET:
        {
            struct sockaddr_in* a = (struct sockaddr_in*)addr;
            int port = ntohs(a->sin_port);
            unsigned char* ipaddr = (unsigned char*)&a->sin_addr.s_addr;
            std::string address;
            address += std::to_string(ipaddr[0]);
            address += '.';
            address += std::to_string(ipaddr[1]);
            address += '.';
            address += std::to_string(ipaddr[2]);
            address += '.';
            address += std::to_string(ipaddr[3]);
            std::string endpoint = "tcp://";
            endpoint += address;
            endpoint += ':';
            endpoint += std::to_string(port);
            connectionData.endpointPeer = std::move(endpoint);
            connectionData.addressPeer = std::move(address);
            connectionData.portPeer = port;
        }
        break;
    default:
        connectionData.endpointPeer.clear();
        connectionData.addressPeer.clear();
        connectionData.portPeer = 0;
        break;
    }
}

}   // namespace finalmq
