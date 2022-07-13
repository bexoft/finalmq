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


namespace finalmq 
{
    class AddressHelpers
    {
        public static void ParseEndpoint(string endpoint, out string protocol, out string address)
        {
            if (string.IsNullOrEmpty(endpoint))
            {
                throw new ArgumentException("Endpoint is empty");
            }

            int pos = endpoint.IndexOf("://");
            if (pos == -1)
            {
                throw new ArgumentException(":// is missing inside endpoint: " + endpoint);
            }

            protocol = endpoint.Substring(0, pos);
            address = endpoint.Substring(pos + 3);

            if (string.IsNullOrEmpty(protocol))
            {
                throw new ArgumentException("Protocol is empty, endpoint: " + endpoint);
            }
            if (string.IsNullOrEmpty(address))
            {
                throw new ArgumentException("Address is empty, endpoint: " + endpoint);
            }
        }

        public static void ParseTcpAddress(string address, out string hostname, out int port)
        {
            if (string.IsNullOrEmpty(address))
            {
                throw new ArgumentException("Address is empty");
            }

            int pos = address.IndexOf(":");
            if (pos == -1)
            {
                throw new ArgumentException("/ is missing inside address: " + address);
            }

            hostname = address.Substring(0, pos);
            string strPort = address.Substring(pos + 1);
            port = int.Parse(strPort);

            if (string.IsNullOrEmpty(hostname))
            {
                throw new ArgumentException("Hostname is empty");
            }
        }

        public static ConnectionData Endpoint2ConnectionData(string endpoint)
        {
            if (string.IsNullOrEmpty(endpoint))
            {
                throw new ArgumentException("Endpoint is empty");
            }

            ConnectionData connectionData = new ConnectionData();

            string protocol;
            string address;
            ParseEndpoint(endpoint, out protocol, out address);
            if (protocol == "tcp")
            {
                string hostname;
                int port = -1;
                ParseTcpAddress(address, out hostname, out port);
                connectionData.Endpoint = endpoint;
                connectionData.Hostname = hostname;
                connectionData.Port = port;
                connectionData.Af = AddressFamily.InterNetwork;
                connectionData.Type = SocketType.Stream;
                connectionData.Protocol = ProtocolType.Tcp;
            }
            else
            {
                throw new ArgumentException("Unknown protocol for endpoint: " + endpoint);
            }

            return connectionData;
        }
    }

}   // namespace finalmq
