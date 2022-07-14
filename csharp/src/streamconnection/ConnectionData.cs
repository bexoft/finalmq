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

using System.Net.Sockets;



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
            connectionData.m_connectionId = m_connectionId;
            connectionData.m_endpoint = m_endpoint;
            connectionData.m_hostname = m_hostname;
            connectionData.m_port = m_port;
            connectionData.m_endpointPeer = m_endpointPeer;
            connectionData.m_af = m_af;
            connectionData.m_type = m_type;
            connectionData.m_protocol = m_protocol;
            connectionData.m_portPeer = m_portPeer;
            connectionData.m_incomingConnection = m_incomingConnection;
            connectionData.m_stream = m_stream;
            connectionData.m_reconnectInterval = m_reconnectInterval;
            connectionData.m_totalReconnectDuration = m_totalReconnectDuration;
            connectionData.m_startTime = m_startTime;
            connectionData.m_ssl = m_ssl;
            connectionData.m_connectionState = m_connectionState;
            connectionData.m_connectProperties = m_connectProperties;
            return connectionData;
        }

        public long ConnectionId
        {
            get => m_connectionId;
            set => m_connectionId = value;
        }

        public string Endpoint
        {
            get => m_endpoint;
            set => m_endpoint = value;
        }

        public string Hostname
        {
            get => m_hostname;
            set => m_hostname = value;
        }

        public int Port
        {
            get => m_port;
            set => m_port = value;
        }

        public string EndpointPeer
        {
            get => m_endpointPeer;
            set => m_endpointPeer = value;
        }

        public AddressFamily Af
        {
            get => m_af;
            set => m_af = value;
        }

        public SocketType Type
        {
            get => m_type;
            set => m_type = value;
        }

        public ProtocolType Protocol
        {
            get => m_protocol;
            set => m_protocol = value;
        }

        public int PortPeer
        {
            get => m_portPeer;
            set => m_portPeer = value;
        }

        public bool IncomingConnection
        {
            get => m_incomingConnection;
            set => m_incomingConnection = value;
        }

        public Stream Stream
        {
            get => m_stream;
            set => m_stream = value;
        }

        public int ReconnectInterval
        {
            get => m_reconnectInterval;
            set => m_reconnectInterval = value;
        }

        public int TotalReconnectDuration
        {
            get => m_totalReconnectDuration;
            set => m_totalReconnectDuration = value;
        }

        public DateTime StartTime
        {
            get => m_startTime;
            set => m_startTime = value;
        }

        public bool Ssl
        {
            get => m_ssl;
            set => m_ssl = value;
        }

        public ConnectionState ConnectionState
        {
            get => m_connectionState;
            set => m_connectionState = value;
        }

        public ConnectProperties? ConnectProperties
        {
            get => m_connectProperties;
            set => m_connectProperties = value;
        }

        private long m_connectionId = 0;
        private string m_endpoint = "";
        private string m_hostname = "";
        private int m_port = 0;
        private string m_endpointPeer = "";
        private AddressFamily m_af = AddressFamily.Unknown;
        private SocketType m_type = SocketType.Unknown;
        private ProtocolType m_protocol = ProtocolType.Unknown;
        private int m_portPeer = 0;
        private bool m_incomingConnection = false;
        private Stream? m_stream = null;
        private int m_reconnectInterval;
        private int m_totalReconnectDuration;
        private DateTime m_startTime = DateTime.Now;
        private bool m_ssl;
        private ConnectionState m_connectionState;
        private ConnectProperties? m_connectProperties = null;
    }

}   // namespace finalmq
