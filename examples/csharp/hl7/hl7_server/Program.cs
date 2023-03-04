using PeerId = System.Int64;
using EntityId = System.UInt64;

using System.Diagnostics;
using finalmq;
using hl7;

namespace hl7example
{
    using VariantStruct = List<NameValue>;

    class EntityServer : RemoteEntity
    {
        public EntityServer()
        {
            // register peer events to see when a remote entity connects or disconnects.
            RegisterPeerEvent((PeerId peerId, SessionInfo session, EntityId entityId, PeerEvent peerEvent, bool incoming) => {
                System.Console.WriteLine("peer event " + peerEvent.ToString());
            });


            RegisterCommand<hl7.SSU_U03>((RequestContext requestContext, hl7.SSU_U03 request) => {
                // prepare the reply
                hl7.SSU_U03 reply = request;
                reply.msh.countryCode = "de";
                reply.equ.alertLevel.alternateIdentifier = "Hello this is a test";
                reply.uac = new hl7.UAC();
                reply.uac.userAuthenticationCredential.typeOfData = hl7.MimeTypes.AudioData;
                reply.sft.Add(new hl7.SFT());
                reply.sft.Add(new hl7.SFT());
                reply.sft[0].softwareBinaryId = "world";
                reply.sft[1].softwareProductInformation = "world";
                reply.specimen_container.Add(new hl7.SPECIMEN_CONTAINER_5());
                reply.specimen_container.Add(new hl7.SPECIMEN_CONTAINER_5());
                reply.specimen_container[0].sac.positionInTray.value1 = "hey";
                reply.specimen_container[0].sac.specimenSource = "hh";
                reply.specimen_container[0].sac.carrierIdentifier.entityIdentifier = "uu";
                reply.specimen_container[0].sac.carrierIdentifier.universalId = "bbb";
                reply.specimen_container[0].obx.Add(new hl7.OBX());
                reply.specimen_container[0].obx.Add(new hl7.OBX());
                reply.specimen_container[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
                reply.specimen_container[0].obx[1].equipmentInstanceIdentifier.Add(new EI());
                reply.specimen_container[0].obx[1].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
                reply.specimen_container[0].specimen.Add(new hl7.SPECIMEN_17());
                reply.specimen_container[0].specimen.Add(new hl7.SPECIMEN_17());
                reply.specimen_container[0].specimen[0].spm.accessionId.Add(new CX());
                reply.specimen_container[0].specimen[0].spm.accessionId[0].identifierCheckDigit = "ggg";
                reply.specimen_container[0].specimen[0].spm.containerCondition.alternateText = "tt";
                reply.specimen_container[0].specimen[0].spm.containerCondition.nameOfAlternateCodingSystem = hl7.CodingSystem.AstmE1238_E1467Universal;
                reply.specimen_container[0].specimen[0].obx.Add(new hl7.OBX());
                reply.specimen_container[0].specimen[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
                reply.specimen_container[0].specimen[0].obx[0].equipmentInstanceIdentifier.Add(new EI());
                reply.specimen_container[0].specimen[0].obx[0].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
                reply.specimen_container[0].specimen[1].spm.accessionId.Add(new CX());
                reply.specimen_container[0].specimen[1].spm.accessionId[0].securityCheck = "ggg";
                reply.specimen_container[0].specimen[1].spm.containerCondition.alternateText = "tt";
                reply.specimen_container[0].specimen[1].spm.containerCondition.nameOfAlternateCodingSystem = hl7.CodingSystem.CdcAnalyteCodes;
                reply.specimen_container[0].specimen[1].obx.Add(new OBX());
                reply.specimen_container[0].specimen[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
                reply.specimen_container[0].specimen[1].obx[0].equipmentInstanceIdentifier.Add(new EI());
                reply.specimen_container[0].specimen[1].obx[0].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
                reply.specimen_container[1].sac.positionInTray.value1 = "hey";
                reply.specimen_container[1].sac.specimenSource = "hh";
                reply.specimen_container[1].sac.carrierIdentifier.entityIdentifier = "uu";
                reply.specimen_container[1].sac.carrierIdentifier.universalId = "bbb";
                reply.specimen_container[1].obx.Add(new OBX());
                reply.specimen_container[1].obx.Add(new OBX());
                reply.specimen_container[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
                reply.specimen_container[1].obx[1].equipmentInstanceIdentifier.Add(new EI());
                reply.specimen_container[1].obx[1].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
                reply.specimen_container[1].specimen.Add(new SPECIMEN_17());
                reply.specimen_container[1].specimen.Add(new SPECIMEN_17());
                reply.specimen_container[1].specimen[0].spm.accessionId.Add(new CX());
                reply.specimen_container[1].specimen[0].spm.accessionId[0].checkDigitScheme = hl7.CheckDigitScheme.Mod10Algorithm;
                reply.specimen_container[1].specimen[0].spm.containerCondition.alternateText = "tt";
                reply.specimen_container[1].specimen[0].spm.containerCondition.nameOfAlternateCodingSystem = hl7.CodingSystem.AstmE1238_E1467Universal;
                reply.specimen_container[1].specimen[0].obx.Add(new OBX());
                reply.specimen_container[1].specimen[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
                reply.specimen_container[1].specimen[0].obx[0].equipmentInstanceIdentifier.Add(new EI());
                reply.specimen_container[1].specimen[0].obx[0].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
                reply.specimen_container[1].specimen[1].spm.accessionId.Add(new CX());
                reply.specimen_container[1].specimen[1].spm.accessionId[0].checkDigitScheme = hl7.CheckDigitScheme.Mod11Algorithm;
                reply.specimen_container[1].specimen[1].spm.containerCondition.alternateText = "tt";
                reply.specimen_container[1].specimen[1].spm.containerCondition.nameOfAlternateCodingSystem = hl7.CodingSystem.AstmE1238_E1467Universal;
                reply.specimen_container[1].specimen[1].obx.Add(new OBX());
                reply.specimen_container[1].specimen[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
                reply.specimen_container[1].specimen[1].obx[0].equipmentInstanceIdentifier.Add(new EI());
                reply.specimen_container[1].specimen[1].obx[0].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";

                // send reply
                requestContext.Reply(reply);
            });


            RegisterCommand<hl7.SSU_U03>("tubes/{id}/POST", (RequestContext requestContext, hl7.SSU_U03 request) => {
                string? filter = requestContext.GetMetainfo("QUERY_filter");
                string f = (filter != null) ? filter : "default";

                string? id = requestContext.GetMetainfo("PATH_id");
                Debug.Assert(id != null);

                string strTime = DateTime.Now.ToString("o");

                hl7.SSU_U03 reply = request;
                reply.equ.alertLevel.alternateText = strTime;
                reply.equ.alertLevel.alternateCodingSystemOid = id;

                // send reply
                requestContext.Reply(reply);
            });

            /*
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
            */
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

            entityContainer.Bind("tcp://*:7000:delimiter_lf:hl7", new BindProperties(
                null, null,
                Variant.Create(new VariantStruct {
                    new NameValue( RemoteEntityFormatHl7.PROPERTY_NAMESPACE, Variant.Create("hl7") ),
                    new NameValue( RemoteEntityFormatHl7.PROPERTY_ENTITY, Variant.Create("MyService") )
                })
            ));

            entityContainer.Bind("tcp://*:8081:httpserver:hl7");

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
