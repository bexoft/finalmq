// MIT License

// Copyright (c) 2020 bexoft GmbH (mail@bexoft.de)

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

using System;
using System.Net.Sockets;
using System.IO;



namespace finalmq 
{

    public enum ConnectionState
    {
        CONNECTIONSTATE_CREATED = 0,
        CONNECTIONSTATE_CONNECTING = 1,
        CONNECTIONSTATE_CONNECTING_FAILED = 2,
        CONNECTIONSTATE_CONNECTED = 3,
        CONNECTIONSTATE_DISCONNECTED = 4,
    }


    public class ConnectionData
    {
        public ConnectionData Clone()
        {
            ConnectionData connectionData = new ConnectionData();
            connectionData.connectionId = connectionId;
            connectionData.endpoint = endpoint;
            connectionData.hostname = hostname;
            connectionData.port = port;
            connectionData.endpointPeer = endpointPeer;
            connectionData.af = af;
            connectionData.type = type;
            connectionData.protocol = protocol;
            connectionData.portPeer = portPeer;
            connectionData.incomingConnection = incomingConnection;
            connectionData.stream = stream;
            connectionData.reconnectInterval = reconnectInterval;
            connectionData.totalReconnectDuration = totalReconnectDuration;
            connectionData.startTime = startTime;
            connectionData.ssl = ssl;
            connectionData.connectionState = connectionState;
            return connectionData;
        }

        public long connectionId = 0;
        public string endpoint = "";
        public string hostname = "";
        public int port = 0;
        public string endpointPeer = "";
        public AddressFamily af = AddressFamily.Unknown;
        public SocketType type = SocketType.Unknown;
        public ProtocolType protocol = ProtocolType.Unknown;
        public int portPeer = 0;
        public bool incomingConnection = false;
        public Stream stream;
        public int reconnectInterval;
        public int totalReconnectDuration;
        public DateTime startTime = DateTime.Now;
        public bool ssl;
        public ConnectionState connectionState;
    }

}   // namespace finalmq
