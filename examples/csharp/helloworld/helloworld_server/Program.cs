using PeerId = System.Int64;
using EntityId = System.UInt64;

using finalmq;

namespace helloworld
{

    class EntityServer : RemoteEntity
    {
        public EntityServer()
        {
            // register peer events to see when a remote entity connects or disconnects.
            RegisterPeerEvent((PeerId peerId, SessionInfo session, EntityId entityId, PeerEvent peerEvent, bool incoming) => {
                System.Console.WriteLine("peer event " + peerEvent.ToString());
            });

            // handle the HelloRequest
            // this is fun - try to access the server with the json interface at port 8888:
            // telnet localhost 8888  (or: netcat localhost 8888) and type:
            //   /MyService/helloworld.HelloRequest!4711{"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}
            // or open a browser and type:
            //   localhost:8080/MyService/helloworld.HelloRequest{"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}
            // or use a HTTP client and do an HTTP request (the method GET, POST, ... does not matter) to localhost:8080 with:
            //   /MyService/helloworld.HelloRequest
            //   and payload: {"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}
            RegisterCommand<HelloRequest>((RequestContext requestContext, HelloRequest request) => {
                // prepare the reply
                string prefix = "Hello ";
                HelloReply reply = new HelloReply();
                // go through all persons and make a greeting
                foreach (var person in request.persons)
                {
                    reply.greetings.Add(prefix + person.name);
                }

                //PeerId peerid = requestContext.PeerId;

                // send reply
                requestContext.Reply(reply);

                // note:
                // The reply does not have to be sent immediately:
                // If you want to reply later, then call requestContext.DelayReply(), you can now store the requestContext and reply later from another thread.

                // note:
                // The requestContext has the method requestContext->peerId()
                // The returned peerId can be used for calling requestReply() or sendEvent().
                // So, also a server entity can act as a client and can send requestReply()
                // to the peer entity that is calling this request.
                // An entity can act as a client and as a server. It is bidirectional (symmetric), like a socket.
            });


            // just to demonstrate REST API. You can access the service with either an HTTP PUT with path: "mypath/1234".
            // Or with "maypath/1234/PUT" and the HTTP method (like: GET, POST, PUT, ...) does not matter.
            // try to access the server with the json interface at port 8888:
            // telnet localhost 8888  (or: netcat localhost 8888) and type:
            //   /MyService/mypath/1234/PUT!4711{"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}
            // or open a browser and type:
            //   localhost:8080/MyService/mypath/1234/PUT{"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}
            // or use a HTTP client and do an HTTP PUT request to localhost:8080 with:
            //   /MyService/mypath/1234
            //   and payload: {"persons":[{"name":"Bonnie"},{"name":"Clyde"}]}
            RegisterCommand<HelloRequest>("mypath/{id}/PUT", (RequestContext requestContext, HelloRequest request) => {
                string? id = requestContext.GetMetainfo("PATH_id"); // when a field is defined as {keyname}, then you can get the value with the key prefix "PATH_" ("PATH_keyname"}
                if (id != null)
                {
                }

                // prepare the reply
                string prefix = "Hello ";
                HelloReply reply = new HelloReply();
                // go through all persons and make a greeting
                foreach (var person in request.persons)
                {
                    reply.greetings.Add(prefix + person.name);
                }

                // send reply
                requestContext.Reply(reply);
            });

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
