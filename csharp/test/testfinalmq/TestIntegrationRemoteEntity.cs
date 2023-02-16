
using System;
using System.Threading;
using System.Security.Cryptography.X509Certificates;
using System.Net.Sockets;
using System.Diagnostics;
using System.Text;

using Xunit;
using Moq;

using finalmq;
using test;
using System.Runtime.CompilerServices;
using System.Net.Security;

namespace testfinalmq
{

    public interface ITestEvents
    {
        void TestRequest(RequestContext requestContext, TestRequest request);
        void TestReply(long peerId, Status status, TestReply reply);
        void PeerEvent(long peerId, SessionInfo session, ulong entityId, PeerEvent peerEvent, bool incoming);
        void ConnEvent(IProtocolSession session, ConnectionEvent connectionEvent);
        void ConnectReply(long peerId, Status status);
    }

    class Constants
    {
        public static readonly string DATA_REQUEST = "Hello";
        public static readonly string DATA_REPLY = "World";
    }


    class EntityServer : RemoteEntity
    {
        public EntityServer(ITestEvents mockEvents)
        {
            m_mockEvents = mockEvents;
            RegisterCommand<TestRequest>((RequestContext requestContext, TestRequest request) => {
                m_mockEvents.TestRequest(requestContext, request);
                Debug.Assert(request.datarequest == Constants.DATA_REQUEST);
                requestContext.Reply(new TestReply(Constants.DATA_REPLY));
            });

            RegisterPeerEvent((long peerId, SessionInfo session, ulong entityId, PeerEvent peerEvent, bool incoming) => {
                m_mockEvents.PeerEvent(peerId, session, entityId, peerEvent, incoming);
            });
        }

        ITestEvents m_mockEvents;
    };


    class EntityServerProto : RemoteEntity
    {
        public EntityServerProto(ITestEvents mockEvents)
        {
            m_mockEvents = mockEvents;
            RegisterCommand<TestRequest>((RequestContext requestContext, TestRequest request) => {
                m_mockEvents.TestRequest(requestContext, request);
                Debug.Assert(request.datarequest == Constants.DATA_REQUEST);
                requestContext.Reply(new TestReply(Constants.DATA_REPLY));
            });

            RegisterPeerEvent((long peerId, SessionInfo session, ulong entityId, PeerEvent peerEvent, bool incoming) => {
                m_mockEvents.PeerEvent(peerId, session, entityId, peerEvent, incoming);
            });
        }

        ITestEvents m_mockEvents;
    };




    [Collection("TestCollectionRemoteEntity")]
    public class TestIntegrationRemoteEntity : IDisposable
    {
        readonly IRemoteEntityContainer m_remoteEntityContainerServer = new RemoteEntityContainer();
        readonly IRemoteEntityContainer m_remoteEntityContainerClient = new RemoteEntityContainer();

        public void Dispose()
        {
            m_remoteEntityContainerServer.Dispose();
            m_remoteEntityContainerClient.Dispose();
        }

        [Fact]
        public void TestProto()
        {
            Mock<ITestEvents> mockEventsServer = new Mock<ITestEvents>();
            Mock<ITestEvents> mockEventsClient = new Mock<ITestEvents>();
            EntityServerProto entityServer = new EntityServerProto(mockEventsServer.Object);
            RemoteEntity entityClient = new RemoteEntity();

            entityClient.RegisterPeerEvent((long peerId, SessionInfo session, ulong entityId, PeerEvent peerEvent, bool incoming) => {
                mockEventsClient.Object.PeerEvent(peerId, session, entityId, peerEvent, incoming);
            });

            m_remoteEntityContainerServer.Bind("tcp://*:3200:headersize:protobuf");
            m_remoteEntityContainerServer.RegisterEntity(entityServer, "MyServer");

            SessionInfo sessionClient = m_remoteEntityContainerClient.Connect("tcp://localhost:3200:headersize:protobuf");

            long peerId = entityClient.Connect(sessionClient, "MyServer", (long peerId, Status status) => {
                mockEventsClient.Object.ConnectReply(peerId, status);
            });

            int callCounter = 0;
            EventWaitHandle wait = new EventWaitHandle(false, EventResetMode.AutoReset);
            int LOOP = 1000;
            for (int i = 0; i < LOOP; ++i)
            {
                entityClient.RequestReply<TestReply>(peerId, new TestRequest(Constants.DATA_REQUEST), (long peerId, Status status, TestReply? reply) =>
                {
                    Debug.Assert(status == Status.STATUS_OK);
                    Debug.Assert(reply != null);
                    Debug.Assert(reply.datareply == Constants.DATA_REPLY);
                    mockEventsClient.Object.TestReply(peerId, status, reply);
                    ++callCounter;
                    if (callCounter == LOOP)
                    {
                        wait.Set();
                    }
                });
            }

            wait.WaitOne(10000);

            m_remoteEntityContainerServer.Dispose();
            m_remoteEntityContainerClient.Dispose();

            // long peerId, SessionInfo session, ulong entityId, PeerEvent peerEvent, bool incoming
            mockEventsServer.Verify(x => x.PeerEvent(It.IsAny<long>(), It.IsAny<SessionInfo>(), entityClient.EntityId, PeerEvent.PEER_CONNECTED, true), Times.Once);
            mockEventsClient.Verify(x => x.PeerEvent(It.IsAny<long>(), sessionClient, entityServer.EntityId, PeerEvent.PEER_CONNECTED, false), Times.Once);
            mockEventsServer.Verify(x => x.PeerEvent(It.IsAny<long>(), It.IsAny<SessionInfo>(), entityClient.EntityId, PeerEvent.PEER_DISCONNECTED, true), Times.Once);
            mockEventsClient.Verify(x => x.PeerEvent(It.IsAny<long>(), sessionClient, entityServer.EntityId, PeerEvent.PEER_DISCONNECTED, false), Times.Once);
            mockEventsClient.Verify(x => x.ConnectReply(It.IsAny<long>(), Status.STATUS_OK), Times.Once);

            mockEventsServer.Verify(x => x.TestRequest(It.IsAny<RequestContext>(), It.IsAny<TestRequest>()), Times.Exactly(LOOP));
            mockEventsClient.Verify(x => x.TestReply(It.IsAny<long>(), Status.STATUS_OK, It.IsAny<TestReply>()), Times.Exactly(LOOP));
        }

        [Fact]
        public void TestJson()
        {
            Mock<ITestEvents> mockEventsServer = new Mock<ITestEvents>();
            Mock<ITestEvents> mockEventsClient = new Mock<ITestEvents>();
            EntityServer entityServer = new EntityServer(mockEventsServer.Object);
            RemoteEntity entityClient = new RemoteEntity();

            entityClient.RegisterPeerEvent((long peerId, SessionInfo session, ulong entityId, PeerEvent peerEvent, bool incoming) => {
                mockEventsClient.Object.PeerEvent(peerId, session, entityId, peerEvent, incoming);
            });

            m_remoteEntityContainerServer.Bind("tcp://*:3201:delimiter_lf:json");
            m_remoteEntityContainerServer.RegisterEntity(entityServer, "MyServer");

            SessionInfo sessionClient = m_remoteEntityContainerClient.Connect("tcp://localhost:3201:delimiter_lf:json");

            long peerId = entityClient.Connect(sessionClient, "MyServer", (long peerId, Status status) => {
                mockEventsClient.Object.ConnectReply(peerId, status);
            });

            int callCounter = 0;
            EventWaitHandle wait = new EventWaitHandle(false, EventResetMode.AutoReset);
            int LOOP = 1000;
            for (int i = 0; i < LOOP; ++i)
            {
                entityClient.RequestReply<TestReply>(peerId, new TestRequest(Constants.DATA_REQUEST), (long peerId, Status status, TestReply? reply) =>
                {
                    Debug.Assert(status == Status.STATUS_OK);
                    Debug.Assert(reply != null);
                    Debug.Assert(reply.datareply == Constants.DATA_REPLY);
                    mockEventsClient.Object.TestReply(peerId, status, reply);
                    ++callCounter;
                    if (callCounter == LOOP)
                    {
                        wait.Set();
                    }
                });
            }

            wait.WaitOne(15000);

            m_remoteEntityContainerServer.Dispose();
            m_remoteEntityContainerClient.Dispose();

            // long peerId, SessionInfo session, ulong entityId, PeerEvent peerEvent, bool incoming
            mockEventsServer.Verify(x => x.PeerEvent(It.IsAny<long>(), It.IsAny<SessionInfo>(), entityClient.EntityId, PeerEvent.PEER_CONNECTED, true), Times.Once);
            mockEventsClient.Verify(x => x.PeerEvent(It.IsAny<long>(), sessionClient, entityServer.EntityId, PeerEvent.PEER_CONNECTED, false), Times.Once);
            mockEventsServer.Verify(x => x.PeerEvent(It.IsAny<long>(), It.IsAny<SessionInfo>(), entityClient.EntityId, PeerEvent.PEER_DISCONNECTED, true), Times.Once);
            mockEventsClient.Verify(x => x.PeerEvent(It.IsAny<long>(), sessionClient, entityServer.EntityId, PeerEvent.PEER_DISCONNECTED, false), Times.Once);
            mockEventsClient.Verify(x => x.ConnectReply(It.IsAny<long>(), Status.STATUS_OK), Times.Once);

            mockEventsServer.Verify(x => x.TestRequest(It.IsAny<RequestContext>(), It.IsAny<TestRequest>()), Times.Exactly(LOOP));
            mockEventsClient.Verify(x => x.TestReply(It.IsAny<long>(), Status.STATUS_OK, It.IsAny<TestReply>()), Times.Exactly(LOOP));
        }

        public static bool ValidateServerCertificate(
                     object sender,
                     X509Certificate? certificate,
                     X509Chain? chain,
                     SslPolicyErrors sslPolicyErrors)
        {
            sslPolicyErrors &= ~SslPolicyErrors.RemoteCertificateChainErrors;
            sslPolicyErrors &= ~SslPolicyErrors.RemoteCertificateNameMismatch;
            if (sslPolicyErrors == SslPolicyErrors.None)
            {
                return true;
            }

            Console.WriteLine("Certificate error: {0}", sslPolicyErrors);

            // Do not allow this client to communicate with unauthenticated servers.
            return false;
        }

        [Fact]
        public void TestSslProto()
        {
            Mock<ITestEvents> mockEventsServer = new Mock<ITestEvents>();
            Mock<ITestEvents> mockEventsClient = new Mock<ITestEvents>();
            EntityServerProto entityServer = new EntityServerProto(mockEventsServer.Object);
            RemoteEntity entityClient = new RemoteEntity();

            entityClient.RegisterPeerEvent((long peerId, SessionInfo session, ulong entityId, PeerEvent peerEvent, bool incoming) => {
                mockEventsClient.Object.PeerEvent(peerId, session, entityId, peerEvent, incoming);
            });

            m_remoteEntityContainerServer.Bind("tcp://*:3202:headersize:protobuf", 
                new BindProperties(new SslServerOptions(new X509Certificate("ssl-certificate.pfx"))));
            m_remoteEntityContainerServer.RegisterEntity(entityServer, "MyServer");

            SessionInfo sessionClient = m_remoteEntityContainerClient.Connect("tcp://localhost:3202:headersize:protobuf", 
                new ConnectProperties(new SslClientOptions("", new RemoteCertificateValidationCallback(ValidateServerCertificate))));

            long peerId = entityClient.Connect(sessionClient, "MyServer", (long peerId, Status status) => {
                mockEventsClient.Object.ConnectReply(peerId, status);
            });

            int callCounter = 0;
            EventWaitHandle wait = new EventWaitHandle(false, EventResetMode.AutoReset);
            int LOOP = 1000;
            for (int i = 0; i < LOOP; ++i)
            {
                entityClient.RequestReply<TestReply>(peerId, new TestRequest(Constants.DATA_REQUEST), (long peerId, Status status, TestReply? reply) =>
                {
                    Debug.Assert(status == Status.STATUS_OK);
                    Debug.Assert(reply != null);
                    Debug.Assert(reply.datareply == Constants.DATA_REPLY);
                    mockEventsClient.Object.TestReply(peerId, status, reply);
                    ++callCounter;
                    if (callCounter == LOOP)
                    {
                        wait.Set();
                    }
                });
            }

            wait.WaitOne(10000);

            m_remoteEntityContainerServer.Dispose();
            m_remoteEntityContainerClient.Dispose();

            // long peerId, SessionInfo session, ulong entityId, PeerEvent peerEvent, bool incoming
            mockEventsServer.Verify(x => x.PeerEvent(It.IsAny<long>(), It.IsAny<SessionInfo>(), entityClient.EntityId, PeerEvent.PEER_CONNECTED, true), Times.Once);
            mockEventsClient.Verify(x => x.PeerEvent(It.IsAny<long>(), sessionClient, entityServer.EntityId, PeerEvent.PEER_CONNECTED, false), Times.Once);
            mockEventsServer.Verify(x => x.PeerEvent(It.IsAny<long>(), It.IsAny<SessionInfo>(), entityClient.EntityId, PeerEvent.PEER_DISCONNECTED, true), Times.Once);
            mockEventsClient.Verify(x => x.PeerEvent(It.IsAny<long>(), sessionClient, entityServer.EntityId, PeerEvent.PEER_DISCONNECTED, false), Times.Once);
            mockEventsClient.Verify(x => x.ConnectReply(It.IsAny<long>(), Status.STATUS_OK), Times.Once);

            mockEventsServer.Verify(x => x.TestRequest(It.IsAny<RequestContext>(), It.IsAny<TestRequest>()), Times.Exactly(LOOP));
            mockEventsClient.Verify(x => x.TestReply(It.IsAny<long>(), Status.STATUS_OK, It.IsAny<TestReply>()), Times.Exactly(LOOP));
        }

        [Fact]
        public void TestProtoLateConnect()
        {
            Mock<ITestEvents> mockEventsServer = new Mock<ITestEvents>();
            Mock<ITestEvents> mockEventsClient = new Mock<ITestEvents>();
            EntityServerProto entityServer = new EntityServerProto(mockEventsServer.Object);
            RemoteEntity entityClient = new RemoteEntity();

            entityClient.RegisterPeerEvent((long peerId, SessionInfo session, ulong entityId, PeerEvent peerEvent, bool incoming) => {
                mockEventsClient.Object.PeerEvent(peerId, session, entityId, peerEvent, incoming);
            });

            m_remoteEntityContainerServer.Bind("tcp://*:3200:headersize:protobuf");
            m_remoteEntityContainerServer.RegisterEntity(entityServer, "MyServer");

            long peerId = entityClient.CreatePeer(m_remoteEntityContainerClient, (long peerId, Status status) => {
                mockEventsClient.Object.ConnectReply(peerId, status);
            });

            int callCounter = 0;
            EventWaitHandle wait = new EventWaitHandle(false, EventResetMode.AutoReset);
            int LOOP = 1000;
            for (int i = 0; i < LOOP; ++i)
            {
                entityClient.RequestReply<TestReply>(peerId, new TestRequest(Constants.DATA_REQUEST), (long peerId, Status status, TestReply? reply) =>
                {
                    Debug.Assert(status == Status.STATUS_OK);
                    Debug.Assert(reply != null);
                    Debug.Assert(reply.datareply == Constants.DATA_REPLY);
                    mockEventsClient.Object.TestReply(peerId, status, reply);
                    ++callCounter;
                    if (callCounter == LOOP)
                    {
                        wait.Set();
                    }
                });
            }

            SessionInfo sessionClient = m_remoteEntityContainerClient.Connect("tcp://localhost:3200:headersize:protobuf");
            entityClient.Connect(peerId, sessionClient, "MyServer");

            wait.WaitOne(10000);

            m_remoteEntityContainerServer.Dispose();
            m_remoteEntityContainerClient.Dispose();

            // long peerId, SessionInfo session, ulong entityId, PeerEvent peerEvent, bool incoming
            mockEventsServer.Verify(x => x.PeerEvent(It.IsAny<long>(), It.IsAny<SessionInfo>(), entityClient.EntityId, PeerEvent.PEER_CONNECTED, true), Times.Once);
            mockEventsClient.Verify(x => x.PeerEvent(It.IsAny<long>(), sessionClient, entityServer.EntityId, PeerEvent.PEER_CONNECTED, false), Times.Once);
            mockEventsServer.Verify(x => x.PeerEvent(It.IsAny<long>(), It.IsAny<SessionInfo>(), entityClient.EntityId, PeerEvent.PEER_DISCONNECTED, true), Times.Once);
            mockEventsClient.Verify(x => x.PeerEvent(It.IsAny<long>(), sessionClient, entityServer.EntityId, PeerEvent.PEER_DISCONNECTED, false), Times.Once);
            mockEventsClient.Verify(x => x.ConnectReply(It.IsAny<long>(), Status.STATUS_OK), Times.Once);

            mockEventsServer.Verify(x => x.TestRequest(It.IsAny<RequestContext>(), It.IsAny<TestRequest>()), Times.Exactly(LOOP));
            mockEventsClient.Verify(x => x.TestReply(It.IsAny<long>(), Status.STATUS_OK, It.IsAny<TestReply>()), Times.Exactly(LOOP));
        }
    }

}


