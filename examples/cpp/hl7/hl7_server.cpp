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
#include "finalmq/protocols/ProtocolDelimiterX.h"
#include "finalmq/protocols/ProtocolMqtt5Client.h"

// the definition of the messages is in the file hl7dictionary.2.7.1.js.fmq
#include "hl7dictionary.2.7.1.js.fmq.h"

#include <iostream>

// the modulename is needed for the logger streams (streamDebug, streamInfo, streamWarning, streamError, streamCritical, streamFatal)
#define MODULENAME  "hl7_server"


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
            reply.uac->userAuthenticationCredential.typeOfData = hl7::MimeTypes::AudioData;
            reply.sft.resize(2);
            reply.sft[0].softwareBinaryId = "world";
            reply.sft[1].softwareProductInformation = "world";
            reply.specimen_container.resize(2);
            reply.specimen_container[0].sac.positionInTray.value1 = "hey";
            reply.specimen_container[0].sac.specimenSource = "hh";
            reply.specimen_container[0].sac.carrierIdentifier.entityIdentifier = "uu";
            reply.specimen_container[0].sac.carrierIdentifier.universalId = "bbb";
            reply.specimen_container[0].obx.resize(2);
            reply.specimen_container[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.specimen_container[0].obx[1].equipmentInstanceIdentifier.resize(1);
            reply.specimen_container[0].obx[1].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
            reply.specimen_container[0].specimen.resize(2);
            reply.specimen_container[0].specimen[0].spm.accessionId.resize(1);
            reply.specimen_container[0].specimen[0].spm.accessionId[0].identifierCheckDigit = "ggg";
            reply.specimen_container[0].specimen[0].spm.containerCondition.alternateText = "tt";
            reply.specimen_container[0].specimen[0].spm.containerCondition.nameOfAlternateCodingSystem = hl7::CodingSystem::AstmE1238_E1467Universal;
            reply.specimen_container[0].specimen[0].obx.resize(1);
            reply.specimen_container[0].specimen[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.specimen_container[0].specimen[0].obx[0].equipmentInstanceIdentifier.resize(1);
            reply.specimen_container[0].specimen[0].obx[0].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
            reply.specimen_container[0].specimen[1].spm.accessionId.resize(1);
            reply.specimen_container[0].specimen[1].spm.accessionId[0].securityCheck = "ggg";
            reply.specimen_container[0].specimen[1].spm.containerCondition.alternateText = "tt";
            reply.specimen_container[0].specimen[1].spm.containerCondition.nameOfAlternateCodingSystem = hl7::CodingSystem::CdcAnalyteCodes;
            reply.specimen_container[0].specimen[1].obx.resize(1);
            reply.specimen_container[0].specimen[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.specimen_container[0].specimen[1].obx[0].equipmentInstanceIdentifier.resize(1);
            reply.specimen_container[0].specimen[1].obx[0].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
            reply.specimen_container[1].sac.positionInTray.value1 = "hey";
            reply.specimen_container[1].sac.specimenSource = "hh";
            reply.specimen_container[1].sac.carrierIdentifier.entityIdentifier = "uu";
            reply.specimen_container[1].sac.carrierIdentifier.universalId = "bbb";
            reply.specimen_container[1].obx.resize(2);
            reply.specimen_container[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.specimen_container[1].obx[1].equipmentInstanceIdentifier.resize(1);
            reply.specimen_container[1].obx[1].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
            reply.specimen_container[1].specimen.resize(2);
            reply.specimen_container[1].specimen[0].spm.accessionId.resize(1);
            reply.specimen_container[1].specimen[0].spm.accessionId[0].checkDigitScheme = hl7::CheckDigitScheme::Mod10Algorithm;
            reply.specimen_container[1].specimen[0].spm.containerCondition.alternateText = "tt";
            reply.specimen_container[1].specimen[0].spm.containerCondition.nameOfAlternateCodingSystem = hl7::CodingSystem::AstmE1238_E1467Universal;
            reply.specimen_container[1].specimen[0].obx.resize(1);
            reply.specimen_container[1].specimen[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.specimen_container[1].specimen[0].obx[0].equipmentInstanceIdentifier.resize(1);
            reply.specimen_container[1].specimen[0].obx[0].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
            reply.specimen_container[1].specimen[1].spm.accessionId.resize(1);
            reply.specimen_container[1].specimen[0].spm.accessionId[0].checkDigitScheme = hl7::CheckDigitScheme::Mod11Algorithm;
            reply.specimen_container[1].specimen[1].spm.containerCondition.alternateText = "tt";
            reply.specimen_container[1].specimen[1].spm.containerCondition.nameOfAlternateCodingSystem = hl7::CodingSystem::AstmE1238_E1467Universal;
            reply.specimen_container[1].specimen[1].obx.resize(1);
            reply.specimen_container[1].specimen[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
            reply.specimen_container[1].specimen[1].obx[0].equipmentInstanceIdentifier.resize(1);
            reply.specimen_container[1].specimen[1].obx[0].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
            
            // send reply
            requestContext->reply(std::move(reply));
        });

        
        registerCommand<hl7::SSU_U03>("tubes/{id}/POST", [this](const RequestContextPtr& requestContext, const std::shared_ptr<hl7::SSU_U03>& request) {
            assert(request);

            std::string* filter = requestContext->getMetainfo("QUERY_filter");
            std::string f = (filter) ? *filter : "default";

            std::string* id = requestContext->getMetainfo("PATH_id");
            assert(id);

            hl7::SSU_U03 reply = *request;
            reply.equ.alertLevel.alternateText = std::to_string(time(nullptr));
            reply.equ.alertLevel.alternateCodingSystemOid = *id;

            // send reply
            requestContext->reply(std::move(reply));
        });

        m_thread = std::thread([this]() {
            hl7::SSU_U03 msg;
            msg.msh.countryCode = "de";
            msg.equ.alertLevel.alternateIdentifier = "Hello this is a test";
            msg.uac = std::make_shared<hl7::UAC>();
            msg.uac->userAuthenticationCredential.typeOfData = hl7::MimeTypes::AudioData;
            msg.sft.resize(2);
            msg.sft[0].softwareBinaryId = "world";
            msg.sft[1].softwareProductInformation = "world";
            msg.specimen_container.resize(2);
            msg.specimen_container[0].sac.positionInTray.value1 = "hey";
            msg.specimen_container[0].sac.specimenSource = "hh";
            msg.specimen_container[0].sac.carrierIdentifier.entityIdentifier = "uu";
            msg.specimen_container[0].sac.carrierIdentifier.universalId = "bbb";
            msg.specimen_container[0].obx.resize(2);
            msg.specimen_container[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
            msg.specimen_container[0].obx[1].equipmentInstanceIdentifier.resize(1);
            msg.specimen_container[0].obx[1].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
            msg.specimen_container[0].specimen.resize(2);
            msg.specimen_container[0].specimen[0].spm.accessionId.resize(1);
            msg.specimen_container[0].specimen[0].spm.accessionId[0].identifierCheckDigit = "ggg";
            msg.specimen_container[0].specimen[0].spm.containerCondition.alternateText = "tt";
            msg.specimen_container[0].specimen[0].spm.containerCondition.nameOfAlternateCodingSystem = hl7::CodingSystem::AstmE1238_E1467Universal;
            msg.specimen_container[0].specimen[0].obx.resize(1);
            msg.specimen_container[0].specimen[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
            msg.specimen_container[0].specimen[0].obx[0].equipmentInstanceIdentifier.resize(1);
            msg.specimen_container[0].specimen[0].obx[0].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
            msg.specimen_container[0].specimen[1].spm.accessionId.resize(1);
            msg.specimen_container[0].specimen[1].spm.accessionId[0].securityCheck = "ggg";
            msg.specimen_container[0].specimen[1].spm.containerCondition.alternateText = "tt";
            msg.specimen_container[0].specimen[1].spm.containerCondition.nameOfAlternateCodingSystem = hl7::CodingSystem::CdcAnalyteCodes;
            msg.specimen_container[0].specimen[1].obx.resize(1);
            msg.specimen_container[0].specimen[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
            msg.specimen_container[0].specimen[1].obx[0].equipmentInstanceIdentifier.resize(1);
            msg.specimen_container[0].specimen[1].obx[0].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
            msg.specimen_container[1].sac.positionInTray.value1 = "hey";
            msg.specimen_container[1].sac.specimenSource = "hh";
            msg.specimen_container[1].sac.carrierIdentifier.entityIdentifier = "uu";
            msg.specimen_container[1].sac.carrierIdentifier.universalId = "bbb";
            msg.specimen_container[1].obx.resize(2);
            msg.specimen_container[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
            msg.specimen_container[1].obx[1].equipmentInstanceIdentifier.resize(1);
            msg.specimen_container[1].obx[1].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
            msg.specimen_container[1].specimen.resize(2);
            msg.specimen_container[1].specimen[0].spm.accessionId.resize(1);
            msg.specimen_container[1].specimen[0].spm.accessionId[0].checkDigitScheme = hl7::CheckDigitScheme::Mod10Algorithm;
            msg.specimen_container[1].specimen[0].spm.containerCondition.alternateText = "tt";
            msg.specimen_container[1].specimen[0].spm.containerCondition.nameOfAlternateCodingSystem = hl7::CodingSystem::AstmE1238_E1467Universal;
            msg.specimen_container[1].specimen[0].obx.resize(1);
            msg.specimen_container[1].specimen[0].obx[0].effectiveDateOfReferenceRange = "aaaa";
            msg.specimen_container[1].specimen[0].obx[0].equipmentInstanceIdentifier.resize(1);
            msg.specimen_container[1].specimen[0].obx[0].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";
            msg.specimen_container[1].specimen[1].spm.accessionId.resize(1);
            msg.specimen_container[1].specimen[1].spm.accessionId[0].checkDigitScheme = hl7::CheckDigitScheme::Mod11Algorithm;
            msg.specimen_container[1].specimen[1].spm.containerCondition.alternateText = "tt";
            msg.specimen_container[1].specimen[1].spm.containerCondition.nameOfAlternateCodingSystem = hl7::CodingSystem::AstmE1238_E1467Universal;
            msg.specimen_container[1].specimen[1].obx.resize(1);
            msg.specimen_container[1].specimen[1].obx[0].effectiveDateOfReferenceRange = "aaaa";
            msg.specimen_container[1].specimen[1].obx[0].equipmentInstanceIdentifier.resize(1);
            msg.specimen_container[1].specimen[1].obx[0].equipmentInstanceIdentifier[0].namespaceId = "bbbbb";

            while (true)
            {
                // send event every 1 second
                std::this_thread::sleep_for(std::chrono::milliseconds(10000));

                // send timer event to all connected peers. No reply expected.
                sendEventToAllPeers(msg);
            }
        });
    }

    private:
        std::thread m_thread;
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

    EntityServer entityServer;

    // register lambda for connection events to see when a network node connects or disconnects.
    entityContainer.registerConnectionEvent([&entityServer] (const SessionInfo& session, ConnectionEvent connectionEvent) {
        const ConnectionData connectionData = session.getConnectionData();
        streamInfo << "connection event at " << connectionData.endpoint
                  << " remote: " << connectionData.endpointPeer
                  << " event: " << connectionEvent.toString();
    });

    // Create server entity and register it at the entityContainer with the service name "MyService"
    // note: multiple entities can be registered.
    entityContainer.registerEntity(&entityServer, "MyService");

    // register an entity for file download. The name "*" means that if an entity name, given by a client, is not found by name, 
    // then this entity will try to open a file inside the htdocs directory. An entity name can contain slashes ('/')
    EntityFileServer entityFileServer("htdocs");
    entityContainer.registerEntity(&entityFileServer, "*");

    // Open listener port 7777 with simple framing protocol ProtocolHeaderBinarySize (4 byte header with the size of payload).
    // content type in payload: protobuf
    entityContainer.bind("tcp://*:7777:headersize:protobuf");

    entityContainer.bind("tcp://*:7000:delimiter_x:hl7", { {}, 
        VariantStruct{ {ProtocolDelimiterX::KEY_DELIMITER, "\x1C\x0D"} },
        VariantStruct{  {RemoteEntityFormatHl7::PROPERTY_NAMESPACE, "hl7"},
                        {RemoteEntityFormatHl7::PROPERTY_ENTITY, "MyService"},
                        {RemoteEntityFormatHl7::PROPERTY_MESSAGESTART, "\x0B"} } });

    entityContainer.bind("tcp://*:7001:delimiter_x:hl7", { {},
        VariantStruct{ {ProtocolDelimiterX::KEY_DELIMITER, "\r\n\r\n"} },
        VariantStruct{ {RemoteEntityFormatHl7::PROPERTY_NAMESPACE, "hl7"},
                       {RemoteEntityFormatHl7::PROPERTY_ENTITY, "MyService"},
                       {RemoteEntityFormatHl7::PROPERTY_LINEEND, "\r\n"}
        }
    });

#ifndef WIN32
    entityContainer.bind("ipc://udp_hl7:delimiter_lf:hl7", { {}, {},
        VariantStruct{  {"hl7namespace", std::string{"hl7"}},
                        {"hl7entity", std::string{"MyService"}} } });
#endif

    entityContainer.bind("tcp://*:7002:delimiter_lf:json", { {}, {},
        VariantStruct{ {RemoteEntityFormatJson::PROPERTY_SERIALIZE_ENUM_AS_STRING, true},
                       {RemoteEntityFormatJson::PROPERTY_SERIALIZE_SKIP_DEFAULT_VALUES, true} }
        });

    entityContainer.bind("tcp://*:7003:headersize:protobuf"),

    entityContainer.bind("tcp://*:8081:httpserver:hl7");

#ifndef WIN32
    entityContainer.bind("ipc://my_uds:headersize:protobuf");
#endif

    // Open listener port 8888 with delimiter framing protocol ProtocolDelimiterLinefeed ('\n' is end of frame).
    // content type in payload: JSON
    entityContainer.bind("tcp://*:8888:delimiter_lf:json");

    // Open listener port 8080 with http.
    // content type in payload: JSON
    entityContainer.bind("tcp://*:8080:httpserver:json", { {}, {},
        VariantStruct{ {RemoteEntityFormatJson::PROPERTY_SERIALIZE_ENUM_AS_STRING, true},
                       {RemoteEntityFormatJson::PROPERTY_SERIALIZE_SKIP_DEFAULT_VALUES, true} }
        });

    entityContainer.bind("tcp://*:8082:httpserver:json", { {}, {},
        VariantStruct{ {RemoteEntityFormatJson::PROPERTY_SERIALIZE_ENUM_AS_STRING, true},
                       {RemoteEntityFormatJson::PROPERTY_SERIALIZE_SKIP_DEFAULT_VALUES, false} }
        });

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
