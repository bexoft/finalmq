using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;


namespace finalmq
{
    interface IPlatform
    {
        ISocket CreateSocket(SocketType socketType, ProtocolType protocolType);
        void MakeSocketPair(out ISocket socket1, out ISocket socket2);
    };

    class PlatformImpl : IPlatform
    {
        public PlatformImpl()
        {
        }

        // IPlatform
        public ISocket CreateSocket(SocketType socketType, ProtocolType protocolType)
        {
            return new SocketWrapper(socketType, protocolType);
        }
        public void MakeSocketPair(out ISocket socket1, out ISocket socket2)
        {
            socket1 = null;
            socket2 = null;
            ISocket socketAccept = null;
            try
            {
                socketAccept = CreateSocket(SocketType.Stream, ProtocolType.Tcp);
                socket2 = CreateSocket(SocketType.Stream, ProtocolType.Tcp);

                socket2.LingerTime = 0;
                socket2.NoDelay = true;

                socketAccept.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, 1);
                socketAccept.Bind(new IPEndPoint(IPAddress.Loopback, 0));
                socketAccept.Listen(1);

                EndPoint endpoint = socketAccept.LocalEndPoint;
                socket2.Connect(endpoint);
                socket2.Blocking = false;

                socket1 = socketAccept.Accept();
                socket1.Blocking = false;
                socket1.LingerTime = 0;
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
    };


    class Platform
    {
        public static IPlatform instance()
        {
            if (m_instance == null)
            {
                m_instance = new PlatformImpl();
            }
            return m_instance;
        }
        public static void setInstance(IPlatform instance)
        {
            m_instance = instance;
        }
        private Platform()
        {
        }

        static IPlatform m_instance = null;
    };
}
