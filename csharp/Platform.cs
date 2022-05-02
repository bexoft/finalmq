using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;


namespace finalmq
{
    public interface IPlatform
    {
        Socket CreateSocket(SocketType socketType, ProtocolType protocolType);
        EndPoint GetLocalEndPoint(Socket socket);
        bool GetNoDelay(Socket socket);
        void SetNoDelay(Socket socket, bool value);
        int GetLingerTime(Socket socket);
        void SetLingerTime(Socket socket, int value);
        bool GetBlocking(Socket socket);
        void SetBlocking(Socket socket, bool value);
        int GetAvailable(Socket socket);
        void SetSocketOption(Socket socket, SocketOptionLevel optionLevel, SocketOptionName optionName, int optionValue);
        void Bind(Socket socket, EndPoint localEP);
        void Listen(Socket socket, int backlog);
        void Connect(Socket socket, EndPoint remoteEP);
        Socket Accept(Socket socket);
        int Send(Socket socket, byte[] buffer, int offset, int size, SocketFlags socketFlags = SocketFlags.None);
        int Receive(Socket socket, byte[] buffer, int offset, int size, SocketFlags socketFlags = SocketFlags.None);
        void MakeSocketPair(out Socket socket1, out Socket socket2);
        void Select(IList<Socket> checkRead, IList<Socket> checkWrite, IList<Socket> checkError, int microSeconds);
    };

    class PlatformImpl : IPlatform
    {
        public PlatformImpl()
        {
        }

        // IPlatform
        public Socket CreateSocket(SocketType socketType, ProtocolType protocolType)
        {
            return new Socket(socketType, protocolType);
        }
        public EndPoint GetLocalEndPoint(Socket socket)
        {
            return socket.LocalEndPoint;
        }
        public bool GetNoDelay(Socket socket)
        {
            return socket.NoDelay;
        }
        public void SetNoDelay(Socket socket, bool value)
        {
            socket.NoDelay = value;
        }
        public int GetLingerTime(Socket socket)
        {
            LingerOption value = socket.LingerState;
            if (value.Enabled)
            {
                return value.LingerTime;
            }
            else
            {
                return -1;
            }
        }
        public void SetLingerTime(Socket socket, int value)
        {
            if (value >= 0)
            {
                socket.LingerState = new LingerOption(true, value);
            }
            else
            {
                socket.LingerState = new LingerOption(false, 0);
            }
        }
        public bool GetBlocking(Socket socket)
        {
            return socket.Blocking;
        }
        public void SetBlocking(Socket socket, bool value)
        {
            socket.Blocking = value;
        }
        public int GetAvailable(Socket socket)
        {
            return socket.Available;
        }
        public void SetSocketOption(Socket socket, SocketOptionLevel optionLevel, SocketOptionName optionName, int optionValue)
        {
            socket.SetSocketOption(optionLevel, optionName, optionValue);
        }
        public void Bind(Socket socket, EndPoint localEP)
        {
            socket.Bind(localEP);
        }
        public void Listen(Socket socket, int backlog)
        {
            socket.Listen(backlog);
        }
        public void Connect(Socket socket, EndPoint remoteEP)
        {
            socket.Connect(remoteEP);
        }
        public Socket Accept(Socket socket)
        {
            return socket.Accept();
        }
        public int Send(Socket socket, byte[] buffer, int offset, int size, SocketFlags socketFlags = SocketFlags.None)
        {
            return socket.Send(buffer, offset, size, socketFlags);
        }
        public int Receive(Socket socket, byte[] buffer, int offset, int size, SocketFlags socketFlags = SocketFlags.None)
        {
            return socket.Receive(buffer, offset, size, socketFlags);
        }
        public void MakeSocketPair(out Socket socket1, out Socket socket2)
        {
            socket1 = null;
            socket2 = null;
            Socket socketAccept = null;
            try
            {
                socketAccept = CreateSocket(SocketType.Stream, ProtocolType.Tcp);
                socket2 = CreateSocket(SocketType.Stream, ProtocolType.Tcp);

                socket2.LingerState = new LingerOption(true, 0);
                socket2.NoDelay = true;

                socketAccept.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, 1);
                socketAccept.Bind(new IPEndPoint(IPAddress.Loopback, 0));
                socketAccept.Listen(1);

                EndPoint endpoint = socketAccept.LocalEndPoint;
                socket2.Connect(endpoint);
                socket2.Blocking = false;

                socket1 = socketAccept.Accept();
                socket1.Blocking = false;
                socket1.LingerState = new LingerOption(true, 0);
                socket1.NoDelay = true;
            }
            catch (System.Exception)
            {
                socket1?.Dispose();
                socket2?.Dispose();
                throw;
            }
            finally
            {
                socketAccept?.Dispose();
            }
        }
        public void Select(IList<Socket> checkRead, IList<Socket> checkWrite, IList<Socket> checkError, int microSeconds)
        {
            Socket.Select((IList)checkRead, (IList)checkWrite, (IList)checkError, microSeconds);
        }
    };


    public class Platform
    {
        public static IPlatform Instance
        {
            get => m_instance;
            set
            {
                m_instance = value;
            }
        }
        private Platform()
        {
        }

        private static IPlatform m_instance = new PlatformImpl();
    };
}
