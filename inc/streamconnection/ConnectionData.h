#pragma once



#include "helpers/BexDefines.h"
#include <string>
#include <chrono>



enum ConnectionState
{
    CONNECTIONSTATE_CREATED = 0,
    CONNECTIONSTATE_CONNECTING = 1,
    CONNECTIONSTATE_CONNECTING_FAILED = 2,
    CONNECTIONSTATE_CONNECTED = 3,
    CONNECTIONSTATE_DISCONNECTED = 4,
};



struct ConnectionData
{
    std::int64_t    connectionId = 0;
    std::string     endpoint;
    std::string     hostname;
    int             port = 0;
    std::string     endpointPeer;
    std::string     addressPeer;
    int             af = 0;
    int             type = 0;
    int             protocol = 0;
    int             portPeer = 0;
    std::string     sockaddr;
    bool            incomingConnection = false;
    SOCKET          sd;
    int             reconnectInterval = 5000;
    int             totalReconnectDuration = -1;
    std::chrono::time_point<std::chrono::system_clock> startTime;
    bool            ssl = false;
    ConnectionState connectionState = CONNECTIONSTATE_CREATED;
};

