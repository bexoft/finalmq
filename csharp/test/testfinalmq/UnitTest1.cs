using System;
using System.Runtime;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;

using Xunit;
using Moq;

using finalmq;

namespace testfinalmq
{
    public class UnitTestSelect : IDisposable
    {
        static int MILLITOMICRO = 1000;
        static int TIMEOUT = 10;

        IPoller m_poller = new PollerImplSelect();
        Socket m_controlSocketRead = new Socket(SocketType.Stream, ProtocolType.Tcp);
        Socket m_controlSocketWrite = new Socket(SocketType.Stream, ProtocolType.Tcp);

        public UnitTestSelect()
        {
            Mock<IPlatform> mockPlatform = new Mock<IPlatform>();
            Platform.Instance = mockPlatform.Object;

            
            //mockPlatform.Setup(x => x.makeSocketPair());

            //mockPlatform.Setup(x => x.makeSocketPair(_, _)).Times(1)
            //            .WillRepeatedly(DoAll(testing::SetArgReferee < 0 > (sd1), testing::SetArgReferee < 1 > (sd2), Return(0)));
            //EXPECT_CALL(*m_mockMockOperatingSystem, send(CONTROLSOCKET_WRITE, _, 1, 0)).Times(1)
            //                                                    .WillRepeatedly(Return(1));

            m_poller.Init();
        }

        public void Dispose()
        {
            m_controlSocketRead.Dispose();
            m_controlSocketWrite.Dispose();
        }

        [Fact]
        public void timeout()
        {
            int timeout = TIMEOUT * MILLITOMICRO;


            //mockPlatform.Setup(x => x.Select(new List<Socket>(), new List<Socket>(), new List<Socket>(), timeout));

            PollerResult result = m_poller.Wait(TIMEOUT);
            Assert.True(result.Timeout);
            Assert.Equal(0, result.DescriptorInfos.Count);
        }
    }
}
