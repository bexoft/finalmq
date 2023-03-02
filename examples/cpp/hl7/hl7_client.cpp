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
#include "finalmq/remoteentity/RemoteEntityFormatHl7.h"
#include "finalmq/remoteentity/RemoteEntityFormatJson.h"
#include "finalmq/logger/Logger.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/protocols/ProtocolMqtt5Client.h"
#include "finalmq/helpers/Executor.h"
#include "finalmq/helpers/ProtothreadRequestReply.h"

// the definition of the messages is in the file hl7dictionary.2.7.1.js.fmq
#include "hl7dictionary.2.7.1.js.fmq.h"

#include <iostream>
#include <thread>
#include <algorithm>

// the modulename is needed for the logger streams (streamDebug, streamInfo, streamWarning, streamError, streamCritical, streamFatal)
#define MODULENAME  "hl7_server"


using namespace finalmq;


class EntityClient : public RemoteEntity
{
public:
    EntityClient()
    {
        // register peer events to see when a remote entity connects or disconnects.
        registerPeerEvent([](PeerId peerId, const SessionInfo& session, EntityId entityId, PeerEvent peerEvent, bool incoming) {
            streamInfo << "peer event " << peerEvent.toString();
        });



        registerCommand<hl7::SSU_U03>([this](const RequestContextPtr& requestContext, const std::shared_ptr<hl7::SSU_U03>& request) {
            assert(request);

//            std::cout << "Result: " << request->equ.alertLevel.alternateIdentifier << std::endl;

            std::cout << ".";
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
    // run entity container in separate thread
    std::thread thread([&entityContainer] () {
        entityContainer.run();
    });


    // register lambda for connection events to see when a network node connects or disconnects.
    entityContainer.registerConnectionEvent([] (const SessionInfo& session, ConnectionEvent connectionEvent) {
        const ConnectionData connectionData = session.getConnectionData();
        streamInfo << "connection event at " << connectionData.endpoint
                  << " remote: " << connectionData.endpointPeer
                  << " event: " << connectionEvent.toString();
    });

    // Create client entity and register it at the entityContainer
    // note: multiple entities can be registered.
    IRemoteEntityPtr entityClientPtr = std::make_shared<EntityClient>();
    IRemoteEntity& entityClient = *entityClientPtr;

    entityContainer.registerEntity(entityClientPtr, "Hl7Entity");

    // register peer events to see when a remote entity connects or disconnects.
    entityClient.registerPeerEvent([] (PeerId peerId, const SessionInfo& session, EntityId entityId, PeerEvent peerEvent, bool incoming) {
        streamInfo << "peer event " << peerEvent.toString();
    });

    // connect to port 7777 with simple framing protocol ProtocolHeaderBinarySize (4 byte header with the size of payload).
    // content type in payload: protobuf
    // note: Also multiple connects are possible.
    // And by the way, also bind()s are possible. An EntityContainer can be client and server at the same time.
    // A client can be started before the server is started. The connect is been retried in the background till the server
    // becomes available. Use the ConnectProperties to change the reconnect properties
    // (default is: try to connect every 5s forever till the server becomes available).
    //SessionInfo sessionClient = entityContainer.connect("tcp://localhost:7777:headersize:protobuf");
    //SessionInfo sessionClient = entityContainer.connect("ipc://my_uds:headersize:protobuf");

    SessionInfo sessionClient = entityContainer.connect("tcp://localhost:7000:delimiter_lf:hl7", { {}, {}, {},
        VariantStruct{  {RemoteEntityFormatHl7::PROPERTY_NAMESPACE, std::string{"hl7"}},
                        {RemoteEntityFormatHl7::PROPERTY_ENTITY, std::string{"Hl7Entity"}} } });


    //SessionInfo sessionClient = entityContainer.connect("tcp://localhost:7001:delimiter_lf:json", { {}, {}, {},
    //    VariantStruct{  {RemoteEntityFormatJson::PROPERTY_SERIALIZE_ENUM_AS_STRING, true},
    //                    {RemoteEntityFormatJson::PROPERTY_SERIALIZE_SKIP_DEFAULT_VALUES, true} } });


    //// if you want to use mqtt5 -> connect to broker
    //SessionInfo sessionClient = entityContainer.connect("tcp://localhost:1883:mqtt5client:json", { {},{},
    //    VariantStruct{  //{ProtocolMqtt5Client::KEY_USERNAME, std::string("")},
    //                    //{ProtocolMqtt5Client::KEY_PASSWORD, std::string("")},
    //                    {ProtocolMqtt5Client::KEY_SESSIONEXPIRYINTERVAL, 300},
    //                    {ProtocolMqtt5Client::KEY_KEEPALIVE, 20},
    //    } });

    // connect entityClient to remote server entity "MyService" with the created TCP session.
    // The returned peerId identifies the peer entity.
    // The peerId will be used for sending commands to the peer (requestReply(), sendEvent())
    //PeerId peerId = entityClient.connect(sessionClient, "MyService", [](PeerId peerId, Status status) {
    //    streamInfo << "connect reply: " << status.toString();
    //    });

    PeerId peerId = entityClient.createPublishPeer(sessionClient, "MyService");


    hl7::SSU_U03 msg;
    msg.msh.countryCode = "de";
    msg.uac = std::make_shared<hl7::UAC>();
    msg.uac->userAuthenticationCredential.typeOfData = hl7::MimeTypes::AudioData;
    msg.sft.resize(3);
    msg.sft[0].softwareBinaryId = "world";
    msg.specimen_container.resize(4);
    msg.specimen_container[0].sac.positionInTray.value1 = "hey";
    msg.specimen_container[0].sac.specimenSource = "hh";
    msg.specimen_container[0].sac.carrierIdentifier.entityIdentifier = "uu";
    msg.specimen_container[0].sac.carrierIdentifier.universalId = "bbb";
    msg.specimen_container[0].obx.resize(2);
    msg.specimen_container[0].specimen.resize(3);
    msg.specimen_container[0].specimen[0].spm.accessionId.resize(1);
    msg.specimen_container[0].specimen[0].spm.accessionId[0].idNumber = "ggg";
    msg.specimen_container[0].specimen[0].spm.containerCondition.alternateText = "tt";
    msg.specimen_container[0].specimen[0].spm.containerCondition.nameOfAlternateCodingSystem = hl7::CodingSystem::AstmE1238_E1467Universal;
    msg.specimen_container[0].specimen[0].obx.resize(5);

    for (int i = 0; i < 10000; i++)
    {
//        entityClient.sendEvent(peerId, msg);

        entityClient.requestReply<hl7::SSU_U03>(peerId,
            msg,
            [](PeerId peerId, Status status, const std::shared_ptr<hl7::SSU_U03>& reply) {
                if (reply)
                {
                    std::cout << ".";
                }
                else
                {
                    std::cout << "REPLY error: " << status.toString() << std::endl;
                }
            });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10000000));

    // release the thread
    entityContainer.terminatePollerLoop();
    thread.join();
    return 0;
}
