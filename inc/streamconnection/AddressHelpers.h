#pragma once


#include "ConnectionData.h"

#include <string>


class AddressHelpers
{
public:
    static int parseEndpoint(const std::string& endpoint, std::string& protocol, std::string& address);
    static int parseTcpAddress(const std::string& address, std::string& hostname, int& port);
    static ConnectionData endpoint2ConnectionData(const std::string& endpoint);
    static void addr2peer(struct sockaddr* addr, ConnectionData& connectionData);
    static std::string makeSocketAddress(const std::string& hostname, int port, int af);
};

