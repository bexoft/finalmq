using System;
using System.Threading;
using System.Security.Cryptography.X509Certificates;

using Xunit;
using Moq;

using finalmq;

namespace testfinalmq
{
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
            m_streamConnectionContainer.Bind("tcp://*:8080", null, new BindProperties(new CertificateData(true, 
                                                            X509Certificate2.CreateFromPemFile("ssltest.cert.pem", "ssltest.key.pem"))));
            for (int i = 0; i < 100; i++)
            {
                int port = 2000 + i;
                m_streamConnectionContainer.Bind("tcp://*:" + port.ToString(), null);
            }
            Thread.Sleep(1000000);
        }

    }
}
