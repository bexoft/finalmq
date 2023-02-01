//MIT License

//Copyright (c) 2020 bexoft GmbH (mail@bexoft.de)

//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.


#include "finalmq/remoteentity/RemoteEntityContainer.h"
#include "finalmq/remoteentity/EntityFileService.h"
#include "finalmq/remoteentity/RemoteEntityFormatHl7.h"
#include "finalmq/remoteentity/RemoteEntityFormatJson.h"
#include "finalmq/logger/Logger.h"
#include "finalmq/helpers/Executor.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/protocols/ProtocolMqtt5Client.h"

// the definition of the messages are in the file helloworld.fmq
#include "hl7.fmq.h"

#include <iostream>

// the modulename is needed for the logger streams (streamDebug, streamInfo, streamWarning, streamError, streamCritical, streamFatal)
#define MODULENAME  "helloworld_server"


using namespace finalmq;


class EntityServer : public RemoteEntity
{
public:
    EntityServer()
    {
        // register peer events to see when a remote entity connects or disconnects.
        registerPeerEvent([] (PeerId peerId, const SessionInfo& session, EntityId entityId, PeerEvent peerEvent, bool incoming) {
            streamInfo << "peer event " << peerEvent.toString();
        });

        registerCommand<hl7::SSU_U03>([this] (const RequestContextPtr& requestContext, const std::shared_ptr<hl7::SSU_U03>& request) {
            assert(request);

            // prepare the reply
            hl7::SSU_U03 reply = *request;
            reply.msh.countryCode = "de";
            reply.equ.alertLevel.alternateIdentifier = "Hello this is a test";
            reply.uac = std::make_shared<hl7::UAC>();
            reply.uac->userAuthenticationCredential.typeOfData = "hello";
            reply.sft.resize(2);
            reply.sft[0].softwareBinaryID = "world";
            reply.sft[1].softwareProductInformation = "world";
            reply.sac.resize(2);
            reply.sac[0].sac.positionInTray = "hey";
            reply.sac[0].sac.specimenSource = "hh";
            reply.sac[0].sac.carrierId.entityIdentifier = "uu";
            reply.sac[0].sac.carrierId.universalId = "bbb";
            reply.sac[0].obx.resize(2);
            reply.sac[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.sac[0].obx[1].equipmentInstanceIdentifier.namespaceId = "bbbbb";
            reply.sac[0].nte.resize(1);
            reply.sac[0].nte[0].commentType.alternateText = "This is text";
            reply.sac[0].spm.resize(2);
            reply.sac[0].spm[0].spm.accessionId = "ggg";
            reply.sac[0].spm[0].spm.containerCondition.alternateText = "tt";
            reply.sac[0].spm[0].spm.containerCondition.nameOfAlternateCodingSystem = "cc";
            reply.sac[0].spm[0].obx.resize(1);
            reply.sac[0].spm[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.sac[0].spm[0].obx[0].equipmentInstanceIdentifier.namespaceId = "bbbbb";
            reply.sac[0].spm[1].spm.accessionId = "ggg";
            reply.sac[0].spm[1].spm.containerCondition.alternateText = "tt";
            reply.sac[0].spm[1].spm.containerCondition.nameOfAlternateCodingSystem = "cc";
            reply.sac[0].spm[1].obx.resize(1);
            reply.sac[0].spm[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.sac[0].spm[1].obx[0].equipmentInstanceIdentifier.namespaceId = "bbbbb";
            reply.sac[1].sac.positionInTray = "hey";
            reply.sac[1].sac.specimenSource = "hh";
            reply.sac[1].sac.carrierId.entityIdentifier = "uu";
            reply.sac[1].sac.carrierId.universalId = "bbb";
            reply.sac[1].obx.resize(2);
            reply.sac[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.sac[1].obx[1].equipmentInstanceIdentifier.namespaceId = "bbbbb";
            reply.sac[1].nte.resize(1);
            reply.sac[1].nte[0].commentType.alternateText = "This is text";
            reply.sac[1].spm.resize(2);
            reply.sac[1].spm[0].spm.accessionId = "ggg";
            reply.sac[1].spm[0].spm.containerCondition.alternateText = "tt";
            reply.sac[1].spm[0].spm.containerCondition.nameOfAlternateCodingSystem = "cc";
            reply.sac[1].spm[0].obx.resize(1);
            reply.sac[1].spm[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.sac[1].spm[0].obx[0].equipmentInstanceIdentifier.namespaceId = "bbbbb";
            reply.sac[1].spm[1].spm.accessionId = "ggg";
            reply.sac[1].spm[1].spm.containerCondition.alternateText = "tt";
            reply.sac[1].spm[1].spm.containerCondition.nameOfAlternateCodingSystem = "cc";
            reply.sac[1].spm[1].obx.resize(1);
            reply.sac[1].spm[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.sac[1].spm[1].obx[0].equipmentInstanceIdentifier.namespaceId = "bbbbb";

            // send reply
            requestContext->reply(std::move(reply));
        });

        
        registerCommand<NoData>("tubes/{id}/GET", [this](const RequestContextPtr& requestContext, const std::shared_ptr<NoData>& request) {
            assert(request);

            std::string* filter = requestContext->getMetainfo("QUERY_filter");
            std::string f = (filter) ? *filter : "default";

            std::string* id = requestContext->getMetainfo("PATH_id");
            assert(id);

            hl7::SSU_U03 reply;
            reply.msh.countryCode = "de";
            reply.equ.alertLevel.alternateIdentifier = "Hello this is a test";
            reply.equ.equipmentInstanceIdentifier.entityIdentifier = *id;
            reply.uac = std::make_shared<hl7::UAC>();
            reply.uac->userAuthenticationCredentialTypeCode.alternateIdentifier = f;
            reply.uac->userAuthenticationCredential.typeOfData = "hello";
            reply.sft.resize(2);
            reply.sft[0].softwareBinaryID = "world";
            reply.sft[1].softwareProductInformation = "world";
            reply.sac.resize(2);
            reply.sac[0].sac.positionInTray = "hey";
            reply.sac[0].sac.specimenSource = "hh";
            reply.sac[0].sac.carrierId.entityIdentifier = "uu";
            reply.sac[0].sac.carrierId.universalId = "bbb";
            reply.sac[0].obx.resize(2);
            reply.sac[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.sac[0].obx[1].equipmentInstanceIdentifier.namespaceId = "bbbbb";
            reply.sac[0].nte.resize(1);
            reply.sac[0].nte[0].commentType.alternateText = "This is text";
            reply.sac[0].spm.resize(2);
            reply.sac[0].spm[0].spm.accessionId = "ggg";
            reply.sac[0].spm[0].spm.containerCondition.alternateText = "tt";
            reply.sac[0].spm[0].spm.containerCondition.nameOfAlternateCodingSystem = "cc";
            reply.sac[0].spm[0].obx.resize(1);
            reply.sac[0].spm[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.sac[0].spm[0].obx[0].equipmentInstanceIdentifier.namespaceId = "bbbbb";
            reply.sac[0].spm[1].spm.accessionId = "ggg";
            reply.sac[0].spm[1].spm.containerCondition.alternateText = "tt";
            reply.sac[0].spm[1].spm.containerCondition.nameOfAlternateCodingSystem = "cc";
            reply.sac[0].spm[1].obx.resize(1);
            reply.sac[0].spm[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.sac[0].spm[1].obx[0].equipmentInstanceIdentifier.namespaceId = "bbbbb";
            reply.sac[1].sac.positionInTray = "hey";
            reply.sac[1].sac.specimenSource = "hh";
            reply.sac[1].sac.carrierId.entityIdentifier = "uu";
            reply.sac[1].sac.carrierId.universalId = "bbb";
            reply.sac[1].obx.resize(2);
            reply.sac[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.sac[1].obx[1].equipmentInstanceIdentifier.namespaceId = "bbbbb";
            reply.sac[1].nte.resize(1);
            reply.sac[1].nte[0].commentType.alternateText = "This is text";
            reply.sac[1].spm.resize(2);
            reply.sac[1].spm[0].spm.accessionId = "ggg";
            reply.sac[1].spm[0].spm.containerCondition.alternateText = "tt";
            reply.sac[1].spm[0].spm.containerCondition.nameOfAlternateCodingSystem = "cc";
            reply.sac[1].spm[0].obx.resize(1);
            reply.sac[1].spm[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.sac[1].spm[0].obx[0].equipmentInstanceIdentifier.namespaceId = "bbbbb";
            reply.sac[1].spm[1].spm.accessionId = "ggg";
            reply.sac[1].spm[1].spm.containerCondition.alternateText = "tt";
            reply.sac[1].spm[1].spm.containerCondition.nameOfAlternateCodingSystem = "cc";
            reply.sac[1].spm[1].obx.resize(1);
            reply.sac[1].spm[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.sac[1].spm[1].obx[0].equipmentInstanceIdentifier.namespaceId = "bbbbb";

            // send reply
            requestContext->reply(std::move(reply));
        });
    }
};



int main()
{
    // display log traces
    Logger::instance().registerConsumer([] (const LogContext& context, const char* text) {
        std::cout << context.filename << "(" << context.line << ") " << text << std::endl;
    });

    // Create and initialize entity container. Entities can be added with registerEntity().
    // Entities are like remote objects, but they can be at the same time client and server.
    // This means, an entity can send (client) and receive (server) a request command.
    RemoteEntityContainer entityContainer;

    entityContainer.init();

    // register lambda for connection events to see when a network node connects or disconnects.
    entityContainer.registerConnectionEvent([] (const SessionInfo& session, ConnectionEvent connectionEvent) {
        const ConnectionData connectionData = session.getConnectionData();
        streamInfo << "connection event at " << connectionData.endpoint
                  << " remote: " << connectionData.endpointPeer
                  << " event: " << connectionEvent.toString();
    });

    // Create server entity and register it at the entityContainer with the service name "MyService"
    // note: multiple entities can be registered.
    EntityServer entityServer;
    entityContainer.registerEntity(&entityServer, "MyService");

    // register an entity for file download. The name "*" means that if an entity name, given by a client, is not found by name, 
    // then this entity will try to open a file inside the htdocs directory. An entity name can contain slashes ('/')
    EntityFileServer entityFileServer("htdocs");
    entityContainer.registerEntity(&entityFileServer, "*");

    // Open listener port 7777 with simple framing protocol ProtocolHeaderBinarySize (4 byte header with the size of payload).
    // content type in payload: protobuf
    entityContainer.bind("tcp://*:7777:headersize:protobuf");

    entityContainer.bind("tcp://*:7000:delimiter_lf:hl7", { {},
        VariantStruct{  {RemoteEntityFormatHl7::PROPERTY_NAMESPACE, std::string{"hl7"}},
                        {RemoteEntityFormatHl7::PROPERTY_ENTITY, std::string{"MyService"}} } });

#ifndef WIN32
    entityContainer.bind("ipc://udp_hl7:delimiter_lf:hl7", { {},
        VariantStruct{  {"hl7namespace", std::string{"hl7"}},
                        {"hl7entity", std::string{"MyService"}} } });
#endif

    entityContainer.bind("tcp://*:7001:delimiter_lf:json", { {},
        VariantStruct{ {RemoteEntityFormatJson::PROPERTY_SERIALIZE_ENUM_AS_STRING, true},
                       {RemoteEntityFormatJson::PROPERTY_SERIALIZE_SKIP_DEFAULT_VALUES, true} }
    });

    entityContainer.bind("tcp://*:7002:headersize:protobuf"),

    entityContainer.bind("tcp://*:8081:httpserver:hl7");

    entityContainer.bind("tcp://*:8082:httpserver:json");

#ifndef WIN32
    entityContainer.bind("ipc://my_uds:headersize:protobuf");
#endif

    // Open listener port 8888 with delimiter framing protocol ProtocolDelimiterLinefeed ('\n' is end of frame).
    // content type in payload: JSON
    entityContainer.bind("tcp://*:8888:delimiter_lf:json");

    // Open listener port 8080 with http.
    // content type in payload: JSON
    entityContainer.bind("tcp://*:8080:httpserver:json");

    //// if you want to use mqtt5 -> connect to broker
    //entityContainer.connect("tcp://localhost:1883:mqtt5client:json", { {},{},
    //    VariantStruct{  //{ProtocolMqtt5Client::KEY_USERNAME, std::string("")},
    //                    //{ProtocolMqtt5Client::KEY_PASSWORD, std::string("")},
    //                    {ProtocolMqtt5Client::KEY_SESSIONEXPIRYINTERVAL, 300},
    //                    {ProtocolMqtt5Client::KEY_KEEPALIVE, 20},
    //} });

    // note:
    // multiple access points (listening ports) can be activated by calling bind() several times.
    // For Unix Domain Sockets use: "ipc://socketname"
    // For SSL/TLS encryption use BindProperties e.g.:
    // entityContainer->bind("tcp://*:7777:headersize", 
    //                       {{true, SSL_VERIFY_NONE, "myservercertificate.cert", "myservercertificate.key"}});
    // And by the way, also connect()s are possible for an EntityContainer. An EntityContainer can be client and server at the same time.


    // run the entity container. this call blocks the execution. 
    // If you do not want to block, then execute run() in another thread
    entityContainer.run();

    return 0;
}
