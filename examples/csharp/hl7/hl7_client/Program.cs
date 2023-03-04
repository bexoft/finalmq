using PeerId = System.Int64;
using EntityId = System.UInt64;

using finalmq;

namespace hl7example
{

    class MainClient
    {
        static readonly int LOOP_PARALLEL = 10000;
        static readonly int LOOP_SEQUENTIAL = 10000;

        public static /*async Task*/void Main(string[] args)
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
/*
            // asynchronous request/reply
            // A peer entity is been identified by its peerId.
            // each request has its own lambda. The lambda is been called when the corresponding reply is received.
            entityClient.RequestReply<HelloReply>(peerId,
                    new HelloRequest(new List<Person>
                            {
                                new Person("Bonnie","Parker",Gender.FEMALE,1910, new Address("somestreet",   12,76875,"Rowena","USA") ),
                                new Person("Clyde", "Barrow",Gender.MALE,  1909, new Address("anotherstreet",32,37385,"Telico","USA") )
                            }),
                (PeerId peerId, Status status, HelloReply? reply) => {
                    if (reply != null)
                    {
                        System.Console.Write("REPLY: ");
                        foreach (var greeting in reply.greetings)
                        {
                            System.Console.Write(greeting + ". ");
                        }
                        System.Console.WriteLine("");
                    }
                    else
                    {
                        System.Console.WriteLine("REPLY error: " + status.ToString());
                    }
            });

            Thread.Sleep(1000);

            for (int n = 0; n < 10; ++n)
            {
                // performance measurement of throughput
                EventWaitHandle wait = new EventWaitHandle(false, EventResetMode.AutoReset);
                long starttime = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond;
                int counter = 0;
                for (int i = 0; i < LOOP_PARALLEL; ++i)
                {
                    // asynchronous request/reply
                    // A peer entity is been identified by its peerId.
                    // each request has its own lambda. The lambda is been called when the corresponding reply is received.
                    entityClient.RequestReply<HelloReply>(peerId,
                            new HelloRequest(new List<Person>
                                    {
                                new Person("Bonnie","Parker",Gender.FEMALE,1910, new Address("somestreet",   12,76875,"Rowena","USA") ),
                                new Person("Clyde", "Barrow",Gender.MALE,  1909, new Address("anotherstreet",32,37385,"Telico","USA") )
                                    }),
                        (PeerId peerId, Status status, HelloReply? reply) => {
                            if (reply != null)
                            {
                                ++counter;
                                if (counter == LOOP_PARALLEL)
                                {
                                    long now = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond;
                                    long dur = now - starttime;
                                    System.Console.WriteLine("time for " + LOOP_PARALLEL + " parallel requests: " + dur + "ms");
                                    wait.Set();
                                }
                            }
                            else
                            {
                                System.Console.WriteLine("REPLY error: ", status.ToString());
                            }
                        });
                }
                wait.WaitOne();
            }

            for (int n = 0; n < 10; ++n)
            {
                long starttimeAsync = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond;
                for (int i = 0; i < LOOP_SEQUENTIAL; ++i)
                {
                    ReplyResult<HelloReply> replyResult = await entityClient.RequestReply<HelloReply>(peerId,
                            new HelloRequest(new List<Person> {
                                    new Person("Bonnie","Parker",Gender.FEMALE,1910, new Address("somestreet",   12,76875,"Rowena","USA") ),
                                    new Person("Clyde", "Barrow",Gender.MALE,  1909, new Address("anotherstreet",32,37385,"Telico","USA") )
                                }));
                }
                long now = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond;
                long dur = now - starttimeAsync;
                System.Console.WriteLine("time for " + LOOP_SEQUENTIAL + " sequential requests: " + dur + "ms");
            }
*/
        }
    }
}
