using System;
using System.Threading;
using System.Security.Cryptography.X509Certificates;
using System.Net.Sockets;

using Xunit;
using Moq;

using finalmq;

namespace testfinalmq
{

    class StreamConnectionCallback : IStreamConnectionCallback
    {
        public IStreamConnectionCallback? Connected(IStreamConnection connection)
        {
            return null;
        }
        public void Disconnected(IStreamConnection connection)
        {

        }
        public bool Received(IStreamConnection connection, Socket socket, int bytesToRead)
        {
            return false;
        }
    }



    [Collection("Our Test Collection #1")]
    public class TestIntegrationStreamConnectionContainer : IDisposable
    {
        StreamConnectionContainer m_streamConnectionContainer = new StreamConnectionContainer();

        public TestIntegrationStreamConnectionContainer()
        {
        }

        public void Dispose()
        {
        }

        [Fact]
        public void TestBind()
        {
            m_streamConnectionContainer.Connect("tcp://localhost:1234", new StreamConnectionCallback());
            //m_streamConnectionContainer.Bind("tcp://*:8080", new StreamConnectionCallback(), new BindProperties(
            //                                    new SslServerOptions(X509Certificate2.CreateFromPemFile("ssltest.cert.pem", "ssltest.key.pem"))));
            //for (int i = 0; i < 100; i++)
            //{
            //    int port = 2000 + i;
            //    m_streamConnectionContainer.Bind("tcp://*:" + port.ToString(), new StreamConnectionCallback());
            //}
            Thread.Sleep(1000000);
        }

    }
}
