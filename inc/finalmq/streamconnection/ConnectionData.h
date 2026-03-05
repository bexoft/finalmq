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

#pragma once

#include <chrono>
#include <string>

#include "OpenSsl.h"
#include "finalmq/helpers/FmqDefines.h"
#include "finalmq/variant/Variant.h"

namespace finalmq
{
enum class ConnectionState
{
    CONNECTIONSTATE_CREATED = 0,
    CONNECTIONSTATE_RECONNECT = 1,
    CONNECTIONSTATE_CONNECTING = 2,
    CONNECTIONSTATE_CONNECTING_FAILED = 3,
    CONNECTIONSTATE_CONNECTED = 4,
    CONNECTIONSTATE_DISCONNECTED = 5,
};

struct BindProperties
{
    CertificateData certificateData{};
    Variant protocolData{};
    Variant formatData{}; ///< data for the serialization format
};

struct ConnectConfig
{
    ConnectConfig(int r = 1000, int t = -1)
        : reconnectInterval(r)
        , totalReconnectDuration(t)
    {
    }
    int reconnectInterval{1000};    ///< if the server is not available, you can pass a reconnection intervall in [ms]
    int totalReconnectDuration{-1}; ///< if the server is not available, you can pass a duration in [ms] how long the reconnect shall happen. -1 means: try for ever.
};

struct ConnectProperties
{
    CertificateData certificateData{};
    ConnectConfig config{};
    Variant protocolData{};
    Variant formatData{}; ///< data for the serialization format
};


struct ConnectionData
{
    std::int64_t connectionId = 0;
    std::string endpoint{};
    std::string hostname{};
    int port = 0;
    std::string endpointPeer{};
    std::string addressPeer{};
    int af = 0;
    int type = 0;
    int protocol = 0;
    int portPeer = 0;
    std::string sockaddr{};
    bool incomingConnection = false;
    SOCKET sd = INVALID_SOCKET;
    int reconnectInterval = 1000;
    int totalReconnectDuration = -1;
    std::chrono::time_point<std::chrono::steady_clock> startTime{};
    bool ssl = false;
    ConnectionState connectionState = ConnectionState::CONNECTIONSTATE_CREATED;
    ConnectProperties connectionProperties{};
};

} // namespace finalmq
