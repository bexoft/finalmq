using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using System.Net;

namespace finalmq
{
    interface ISocket : IDisposable
    {
        Socket getSocket();
        EndPoint LocalEndPoint { get; }
        bool NoDelay { get; set; }
        int LingerTime { get; set; }
        bool Blocking { get; set; }

        void SetSocketOption(SocketOptionLevel optionLevel, SocketOptionName optionName, int optionValue);
        void Bind(EndPoint localEP);
        void Listen(int backlog);
        void Connect(EndPoint remoteEP);
        ISocket Accept();
    }

    class SocketWrapper : ISocket
    {
        private Socket m_socket;
        private bool m_disposed = false;

        public SocketWrapper(SocketType socketType, ProtocolType protocolType)
        {
            m_socket = new Socket(socketType, protocolType);
        }
        private SocketWrapper(Socket socket)
        {
            m_socket = socket;
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected void Dispose(bool disposing)
        {
            if (!m_disposed)
            {
                if (disposing)
                {
                    m_socket.Dispose();
                }
                m_disposed = true;
            }
        }

        public Socket getSocket()
        {
            return m_socket;
        }

        public EndPoint LocalEndPoint 
        { 
            get => m_socket.LocalEndPoint; 
        }

        public bool NoDelay
        {
            get => m_socket.NoDelay;
            set
            {
                m_socket.NoDelay = value;
            }
        }
        public int LingerTime
        {
            get
            {
                LingerOption value = m_socket.LingerState;
                if (value.Enabled)
                {
                    return value.LingerTime;
                }
                else
                {
                    return -1;
                }
            }
            set
            {
                if (value >= 0)
                {
                    m_socket.LingerState = new LingerOption(true, value);
                }
                else
                {
                    m_socket.LingerState = new LingerOption(false, 0);
                }
            }
        }

        public bool Blocking
        {
            get => m_socket.Blocking;
            set
            {
                m_socket.Blocking = value;
            }
        }

        public void SetSocketOption(SocketOptionLevel optionLevel, SocketOptionName optionName, int optionValue)
        {
            m_socket.SetSocketOption(optionLevel, optionName, optionValue);
        }

        public void Bind(EndPoint localEP)
        {
            m_socket.Bind(localEP);
        }

        public void Listen(int backlog)
        {
            m_socket.Listen(backlog);
        }

        public void Connect(EndPoint remoteEP)
        {
            m_socket.Connect(remoteEP);
        }

        public ISocket Accept()
        {
            Socket socket = m_socket.Accept();
            return new SocketWrapper(socket);
        }
    }
}
