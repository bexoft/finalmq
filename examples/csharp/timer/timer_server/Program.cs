using PeerId = System.Int64;
using EntityId = System.UInt64;

using finalmq;

namespace timer
{

    class EntityServer : RemoteEntity
    {
        private bool m_timerActive = true;
        private Thread m_thread;

        public EntityServer()
        {
            // register peer events to see when a remote entity connects or disconnects.
            RegisterPeerEvent((PeerId peerId, SessionInfo session, EntityId entityId, PeerEvent peerEvent, bool incoming) => {
                System.Console.WriteLine("peer event " + peerEvent.ToString());
            });


            RegisterCommand<StartRequest>((RequestContext requestContext, StartRequest request) => {
                m_timerActive = true;
            });

            m_thread = new Thread(() => {
                while (true)
                {
                    // send event every 1 second
                    Thread.Sleep(1000);

                    // send, only if active
                    if (m_timerActive)
                    {
                        // get current time string
                        string strTime = DateTime.Now.ToString("o");

                        // send timer event to all connected peers. No reply expected.
                        SendEventToAllPeers(new TimerEvent( strTime ));
                    }
                }
            });

            m_thread.Start();
        }

    }

    class MainServer
    {
        public static void Main(string[] args)
        {
            // Create and initialize entity container. Entities can be added with registerEntity().
            // Entities are like remote objects, but they can be at the same time client and server.
            // This means, an entity can send (client) and receive (server) a request command.
            RemoteEntityContainer entityContainer = new RemoteEntityContainer();

            // register lambda for connection events to see when a network node connects or disconnects.
            entityContainer.RegisterConnectionEvent((SessionInfo session, ConnectionEvent connectionEvent) => {
                ConnectionData connectionData = session.ConnectionData;
                System.Console.WriteLine("connection event at " + connectionData.Endpoint
                          + " remote: " + connectionData.EndpointPeer
                          + " event: " + connectionEvent.ToString());
            });

            // Create server entity and register it at the entityContainer with the service name "MyService"
            // note: multiple entities can be registered.
            EntityServer entityServer = new EntityServer();
            entityContainer.RegisterEntity(entityServer, "MyService");

            // register an entity for file download. The name "*" means that if an entity name, given by a client, is not found by name, 
            // then this entity will try to open a file inside the htdocs directory. An entity name can contain slashes ('/')
            EntityFileServer entityFileServer = new EntityFileServer("../../../../../../../htdocs");
            entityContainer.RegisterEntity(entityFileServer, "*");

            // Open listener port 7777 with simple framing protocol ProtocolHeaderBinarySize (4 byte header with the size of payload).
            // content type in payload: protobuf
            entityContainer.Bind("tcp://*:7777:headersize:protobuf");

            // Open listener port 8888 with delimiter framing protocol ProtocolDelimiterLinefeed ('\n' is end of frame).
            // content type in payload: JSON
            entityContainer.Bind("tcp://*:8888:delimiter_lf:json");

            // Open listener port 8080 with http.
            // content type in payload: JSON
            entityContainer.Bind("tcp://*:8080:httpserver:json");


            // note:
            // multiple access points (listening ports) can be activated by calling bind() several times.
            // For Unix Domain Sockets use: "ipc://socketname"
            // For SSL/TLS encryption use BindProperties e.g.:
            // entityContainer->bind("tcp://*:7777:headersize", 
            //                       new BindProperties(new SslServerOptions(new X509Certificate("ssl-certificate.pfx"))));
            // And by the way, also connect()s are possible for an EntityContainer. An EntityContainer can be client and server at the same time.

            Thread.Sleep(100000000);
        }
    }

}
