using PeerId = System.Int64;
using EntityId = System.UInt64;

using finalmq;

namespace timer
{

    class MainClient
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

            // Create client entity and register it at the entityContainer
            // note: multiple entities can be registered.
            IRemoteEntity entityClient = new RemoteEntity();

            // It is possible to register a command in the constructor of a derived RemoteEntity.
            // But here, it is shown that also a register is possible from outside.
            entityClient.RegisterCommand<TimerEvent>((RequestContext requestContext, TimerEvent request) => {
                System.Console.WriteLine("time: " + request.time);
            });

            // register peer events to see when a remote entity connects or disconnects.
            entityClient.RegisterPeerEvent((PeerId peerId, SessionInfo session, EntityId entityId, PeerEvent peerEvent, bool incoming) => {
                System.Console.WriteLine("peer event " + peerEvent.ToString());
            });

            // connect to port 7777 with simple framing protocol ProtocolHeaderBinarySize (4 byte header with the size of payload).
            // content type in payload: protobuf
            // note: Also multiple connects are possible.
            // And by the way, also bind()s are possible. An EntityContainer can be client and server at the same time.
            // A client can be started before the server is started. The connect is been retried in the background till the server
            // becomes available. Use the ConnectProperties to change the reconnect properties
            // (default is: try to connect every 5s forever till the server becomes available).
            SessionInfo sessionClient = entityContainer.Connect("tcp://localhost:7777:headersize:protobuf");

            // connect entityClient to remote server entity "MyService" with the created TCP session.
            // The returned peerId identifies the peer entity.
            // The peerId will be used for sending commands to the peer (requestReply(), sendEvent())
            PeerId peerId = entityClient.Connect(sessionClient, "MyService", (PeerId peerId, Status status) => {
                System.Console.WriteLine("connect reply: " + status.ToString());
            });


            Thread.Sleep(60000);
        }
    }
}
