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
#include "finalmq/protocols/ProtocolDelimiterX.h"
#include "finalmq/helpers/ProtothreadRequestReply.h"

// the definition of the messages is in the file hl7dictionary.2.7.1.js.fmq
#include "hl7dictionary.2.6.js.fmq.h"

#include <iostream>
#include <thread>
#include <iomanip>

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
    }
};

class HL7Client
{
private:
    std::thread m_thread;
    RemoteEntityContainer entityContainer;

public:
    HL7Client() 
    {
        // display log traces
        Logger::instance().registerConsumer([](const LogContext& context, const char* text) {
            std::cout << context.filename << "(" << context.line << ") " << text << std::endl;
            });

        // Initialize entity container. Entities can be added with registerEntity().
        // Entities are like remote objects, but they can be at the same time client and server.
        // This means, an entity can send (client) and receive (server) a request command.
        entityContainer.init();

        // run entity container in separate thread
        m_thread = std::thread([this]() {
            entityContainer.run();
            });


        // register lambda for connection events to see when a network node connects or disconnects.
        entityContainer.registerConnectionEvent([](const SessionInfo& session, ConnectionEvent connectionEvent) {
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
        entityClient.registerPeerEvent([](PeerId peerId, const SessionInfo& session, EntityId entityId, PeerEvent peerEvent, bool incoming) {
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

        SessionInfo sessionClient = entityContainer.connect("tcp://localhost:7000:delimiter_x:hl7", { {}, {},
            VariantStruct{ {ProtocolDelimiterX::KEY_DELIMITER, "\x1C\x0D"} },
            VariantStruct{  {RemoteEntityFormatHl7::PROPERTY_NAMESPACE, "hl7"},
                            {RemoteEntityFormatHl7::PROPERTY_ENTITY, "Hl7Entity"},
                            {RemoteEntityFormatHl7::PROPERTY_MESSAGESTART, "\x0B"} } });


        // connect entityClient to remote server entity "MyService" with the created TCP session.
        // The returned peerId identifies the peer entity.
        // The peerId will be used for sending commands to the peer (requestReply(), sendEvent())
        //PeerId peerId = entityClient.connect(sessionClient, "MyService", [](PeerId peerId, Status status) {
        //    streamInfo << "connect reply: " << status.toString();
        //    });

        PeerId peerId = entityClient.createPublishPeer(sessionClient, "MyService");

        // Date Time Now
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ssDateTime;
        ssDateTime << std::put_time(std::localtime(&in_time_t), "%Y%m%d%H%M%S");
        std::string dateTimeStr = ssDateTime.str();
        std::stringstream ssDate;
        ssDate << std::put_time(std::localtime(&in_time_t), "%Y%m%d");
        std::string dateStr = ssDate.str();

        hl7::ORM_O01 msg;

        // MSH
        msg.msh.sendingApplication = hl7::HD("MyNamespace", "MyApplicationId", hl7::UniversalIdType());
        msg.msh.sendingFacility = hl7::HD("MyNamespace", "MyFacilityId", hl7::UniversalIdType());
        msg.msh.receivingApplication = hl7::HD("YourNamespace", "YourApplicationId", hl7::UniversalIdType());
        msg.msh.receivingFacility = hl7::HD("YourNamespace", "YourFacilityId", hl7::UniversalIdType());
        msg.msh.date_timeOfMessage = dateTimeStr;
        msg.msh.messageControlId = "1234567890";
        msg.msh.processingId.processingId = hl7::ProcessingId::Enum::Production;
        msg.msh.versionId.versionId = hl7::VersionId::Enum::Release2_6;
        msg.msh.characterSet = { "8859/1" };

        // Patient Start
        auto patient = std::make_shared<hl7::PATIENT_13>();

        // PID
        patient->pid.patientIdentifierList.resize(1);
        patient->pid.patientIdentifierList[0].idNumber = "TW 000123/24";
        patient->pid.patientName.resize(1);
        patient->pid.patientName[0].familyName.surname = "Testperson";
        patient->pid.patientName[0].givenName = "Testi";
        patient->pid.date_timeOfBirth = "19700101";
        patient->pid.administrativeSex = "W";

        // PV1
        auto patientVisit = std::make_shared<hl7::PATIENT_VISIT_2>();
        patientVisit->pv1.patientClass = "A";
        patientVisit->pv1.assignedPatientLocation.pointOfCare = "DKD";
        patientVisit->pv1.referringDoctor.resize(1);
        patientVisit->pv1.referringDoctor[0].idNumber = "UBLEI";
        patientVisit->pv1.temporaryLocation.pointOfCare = "UBCT01";
        patientVisit->pv1.visitNumber.idNumber = "TW 000123/24";
        patientVisit->pv1.admitDate_time = dateStr;
        patient->patient_visit = patientVisit;

        msg.patient = patient;

        // Order Start
        hl7::ORDER_26 order;

        // ORC
        hl7::ORC orc;
        orc.orderControl = "NW";
        orc.placerOrderNumber.entityIdentifier = "TW00012324000111";
        orc.placerGroupNumber.entityIdentifier = "BCT";
        orc.responseFlag = "0";
        //orc.quantity_timing.resize(1);
        //orc.quantity_timing[0].interval.repeatPattern = hl7::RepeatPattern::Empty;
        orc.date_timeOfTransaction = dateTimeStr;
        orc.enteredBy.resize(1);
        orc.enteredBy[0].idNumber = "UBLEI";
        orc.orderingProvider.resize(1);
        orc.orderingProvider[0].idNumber = "UBLEI";
        orc.enteringDevice.identifier = "BCT";
        order.orc = orc;


        // OBR
        auto obr = std::make_shared<hl7::OBR>();
        obr->placerOrderNumber.entityIdentifier = "TW00012324000111";
        obr->placerOrderNumber.namespaceId = "1"; // notwendig?
        obr->universalServiceIdentifier.identifier = "CT_KNIE_L";
        obr->universalServiceIdentifier.alternateIdentifier = "CT";
        obr->placerField1 = "-";
        //obr->quantity_timing.resize(1);
        //obr->quantity_timing[0].startDate_time = dateTimeStr;
        auto obrEtc = hl7::OBR_RQD_RQ1_RXO_ODS_ODT_1();
        auto orderDetail = std::make_shared<hl7::ORDER_DETAIL_1>();
        obrEtc.obr = obr;
        orderDetail->obr_rqd_rq1_rxo_ods_odt = { obrEtc };

        // OBX
        hl7::OBSERVATION_2 observation1;
        hl7::OBSERVATION_2 observation2;
        observation1.obx.setId_Obx = "1";
        observation1.obx.valueType = "TX";
        observation1.obx.observationIdentifier.identifier = "WEIGHT";
        observation1.obx.observationValue = { "0" };
        observation1.obx.units.identifier = "kg";
        observation2.obx.setId_Obx = "2";
        observation2.obx.valueType = "TX";
        observation2.obx.observationIdentifier.identifier = "HEIGHT";
        observation2.obx.observationValue = { "0" };
        observation2.obx.units.identifier = "cm";
        orderDetail->observation = { observation1, observation2 };

        order.order_detail = orderDetail;
        msg.order = { order };

        for (int i = 0; i < 100; i++)
        {
            msg.msh.messageControlId = std::to_string(i);
            //        entityClient.sendEvent(peerId, msg);
            entityClient.requestReply<hl7::ACK>(peerId,
                msg,
                [](PeerId peerId, Status status, const std::shared_ptr<hl7::ACK>& reply) {
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

        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }

    void SendMessages()
    {

    }

    ~HL7Client()
    {
        // release the thread
        entityContainer.terminatePollerLoop();
        m_thread.join();
    }
};


int main()
{
    HL7Client client;
    client.SendMessages();
    return 0;
}
