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

            static void Resize<T>(IList<T> list, int num)
            {
                while (list.Count < 2)
                {
                    list.Add(Activator.CreateInstance<T>());
                }
            }

            RegisterCommand<hl7.SSU_U03>((RequestContext requestContext, hl7.SSU_U03 request) => {
                // prepare the reply
                hl7.SSU_U03 reply = request;
                reply.msh.countryCode = "de";
                reply.equ.alertLevel.alternateIdentifier = "Hello this is a test";
                reply.uac = new hl7.UAC();
                reply.uac.userAuthenticationCredential.typeOfData = hl7.MimeTypes.AudioData;
                Resize(reply.sft, 2);
                reply.sft[0].softwareBinaryId = "world";
                reply.sft[1].softwareProductInformation = "world";
                Resize(reply.specimen_container, 2);
                reply.specimen_container[0].sac.positionInTray.value1 = "hey";
                reply.specimen_container[0].sac.specimenSource = "hh";
                reply.specimen_container[0].sac.carrierIdentifier.entityIdentifier = "uu";
                reply.specimen_container[0].sac.carrierIdentifier.universalId = "bbb";
                Resize(reply.specimen_container[0].obx, 2);
                reply.specimen_container[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
                Resize(reply.specimen_container[0].obx[1].equipmentInstanceIdentifier, 1);
                reply.specimen_container[0].obx[1].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
                Resize(reply.specimen_container[0].specimen, 2);
                Resize(reply.specimen_container[0].specimen[0].spm.accessionId, 1);
                reply.specimen_container[0].specimen[0].spm.accessionId[0].identifierCheckDigit = "ggg";
                reply.specimen_container[0].specimen[0].spm.containerCondition.alternateText = "tt";
                reply.specimen_container[0].specimen[0].spm.containerCondition.nameOfAlternateCodingSystem = hl7.CodingSystem.AstmE1238_E1467Universal;
                Resize(reply.specimen_container[0].specimen[0].obx, 1);
                reply.specimen_container[0].specimen[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
                Resize(reply.specimen_container[0].specimen[0].obx[0].equipmentInstanceIdentifier, 1);
                reply.specimen_container[0].specimen[0].obx[0].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
                Resize(reply.specimen_container[0].specimen[1].spm.accessionId, 1);
                reply.specimen_container[0].specimen[1].spm.accessionId[0].securityCheck = "ggg";
                reply.specimen_container[0].specimen[1].spm.containerCondition.alternateText = "tt";
                reply.specimen_container[0].specimen[1].spm.containerCondition.nameOfAlternateCodingSystem = hl7.CodingSystem.CdcAnalyteCodes;
                Resize(reply.specimen_container[0].specimen[1].obx, 1);
                reply.specimen_container[0].specimen[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
                Resize(reply.specimen_container[0].specimen[1].obx[0].equipmentInstanceIdentifier, 1);
                reply.specimen_container[0].specimen[1].obx[0].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
                reply.specimen_container[1].sac.positionInTray.value1 = "hey";
                reply.specimen_container[1].sac.specimenSource = "hh";
                reply.specimen_container[1].sac.carrierIdentifier.entityIdentifier = "uu";
                reply.specimen_container[1].sac.carrierIdentifier.universalId = "bbb";
                Resize(reply.specimen_container[1].obx, 2);
                reply.specimen_container[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
                Resize(reply.specimen_container[1].obx[1].equipmentInstanceIdentifier, 1);
                reply.specimen_container[1].obx[1].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
                Resize(reply.specimen_container[1].specimen, 2);
                Resize(reply.specimen_container[1].specimen[0].spm.accessionId, 1);
                reply.specimen_container[1].specimen[0].spm.accessionId[0].checkDigitScheme = hl7.CheckDigitScheme.Mod10Algorithm;
                reply.specimen_container[1].specimen[0].spm.containerCondition.alternateText = "tt";
                reply.specimen_container[1].specimen[0].spm.containerCondition.nameOfAlternateCodingSystem = hl7.CodingSystem.AstmE1238_E1467Universal;
                Resize(reply.specimen_container[1].specimen[0].obx, 1);
                reply.specimen_container[1].specimen[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
                Resize(reply.specimen_container[1].specimen[0].obx[0].equipmentInstanceIdentifier, 1);
                reply.specimen_container[1].specimen[0].obx[0].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
                Resize(reply.specimen_container[1].specimen[1].spm.accessionId, 1);
                reply.specimen_container[1].specimen[1].spm.accessionId[0].checkDigitScheme = hl7.CheckDigitScheme.Mod11Algorithm;
                reply.specimen_container[1].specimen[1].spm.containerCondition.alternateText = "tt";
                reply.specimen_container[1].specimen[1].spm.containerCondition.nameOfAlternateCodingSystem = hl7.CodingSystem.AstmE1238_E1467Universal;
                Resize(reply.specimen_container[1].specimen[1].obx, 1);
                reply.specimen_container[1].specimen[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
                Resize(reply.specimen_container[1].specimen[1].obx[0].equipmentInstanceIdentifier, 1);
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

            entityContainer.Bind("tcp://*:7000:delimiter_lf:hl7", new BindProperties(
                null, null,
                Variant.Create(new VariantStruct {
                    new NameValue( RemoteEntityFormatHl7.PROPERTY_NAMESPACE, Variant.Create("hl7") ),
                    new NameValue( RemoteEntityFormatHl7.PROPERTY_ENTITY, Variant.Create("MyService") )
                })
            ));

            entityContainer.Bind("tcp://*:7001:delimiter_x:hl7", new BindProperties(
                null,
                Variant.Create(new VariantStruct {
                    new NameValue( ProtocolDelimiterX.KEY_DELIMITER, Variant.Create("\r\n\r\n") ),
                }),
                Variant.Create(new VariantStruct {
                    new NameValue( RemoteEntityFormatHl7.PROPERTY_NAMESPACE, Variant.Create("hl7") ),
                    new NameValue( RemoteEntityFormatHl7.PROPERTY_ENTITY, Variant.Create("MyService") ),
                    new NameValue( RemoteEntityFormatHl7.PROPERTY_LINEEND, Variant.Create("\r\n") ),
                    new NameValue( RemoteEntityFormatHl7.PROPERTY_MESSAGEEND, Variant.Create("") )
                })
            ));

            // Open listener port 8080 with http.
            // content type in payload: JSON
            entityContainer.Bind("tcp://*:8080:httpserver:json", new BindProperties(
                null, null,
                Variant.Create(new VariantStruct {
                    new NameValue( RemoteEntityFormatJson.PROPERTY_SERIALIZE_ENUM_AS_STRING, Variant.Create(true) ),
                    new NameValue( RemoteEntityFormatJson.PROPERTY_SERIALIZE_SKIP_DEFAULT_VALUES, Variant.Create(true) ),
                })
            ));
            entityContainer.Bind("tcp://*:8082:httpserver:json", new BindProperties(
                null, null,
                Variant.Create(new VariantStruct {
                    new NameValue( RemoteEntityFormatJson.PROPERTY_SERIALIZE_ENUM_AS_STRING, Variant.Create(true) ),
                    new NameValue( RemoteEntityFormatJson.PROPERTY_SERIALIZE_SKIP_DEFAULT_VALUES, Variant.Create(false) ),
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
