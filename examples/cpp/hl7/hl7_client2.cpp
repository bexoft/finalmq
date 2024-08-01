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
#include "finalmq/logger/Logger.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/protocols/ProtocolDelimiterX.h"

// the definition of the messages is in the file hl7dictionary.2.6.js.fmq
#include "hl7dictionary.2.6.js.fmq.h"

#include <iostream>
#include <thread>
#include <iomanip>
#include <map>

// the modulename is needed for the logger streams (streamDebug, streamInfo, streamWarning, streamError, streamCritical, streamFatal)
#define MODULENAME  "hl7_server"


using namespace finalmq;


class EntityClient : public RemoteEntity
{
public:
    EntityClient()
    {
        // RECEIVE ack
        registerCommand<hl7::ACK>([this](const RequestContextPtr& requestContext, const std::shared_ptr<hl7::ACK>& ack) {
            assert(ack);
            std::cout << "Received ACK: " << ack->msh.messageControlId << std::endl;

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                auto it = m_requests.find(ack->msh.messageControlId);
                if (it != m_requests.end())
                {
                    m_requests.erase(it);   // remove acknowledged message
                }
            }
        });
    }

    // SEND
    void sendHl7Message(finalmq::StructBasePtr& msg)
    {
        hl7::MSH& msh = reinterpret_cast<hl7::ACK*>(msg.get())->msh;    // finalmq does not have a base class for HL7 Messages, yet, so I use the ACK to get MSH
        
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_id++;
            std::string idAsString = std::to_string(m_id);
            msh.messageControlId = idAsString;
            m_requests[idAsString] = msg;   // keep not acknowledged messages
            std::cout << "Sending MSG: " << msh.messageControlId << std::endl;
        }

        sendEvent(m_peerId, *msg);
    }    

    void connected(PeerId peerId)
    {
        m_peerId = peerId;

        // collect all unacknowledged messages
        std::vector<finalmq::StructBasePtr> messagesToResend;

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto element : m_requests)
            {
                messagesToResend.push_back(element.second);
            }
        }

        // resend all unacknowledged messages
        for (auto msg : messagesToResend)
        {
            sendEvent(m_peerId, *msg);
        }
    }

    std::map<std::string, finalmq::StructBasePtr> m_requests{};
    int m_id{ 0 };
    PeerId m_peerId{};
    std::mutex  m_mutex{};
};

class HL7Client
{
public:
    HL7Client(std::shared_ptr<EntityClient> entityClientPtr)
    {
        m_entityClient = entityClientPtr;

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
        entityContainer.registerConnectionEvent([this](const SessionInfo& session, ConnectionEvent connectionEvent) {
            const ConnectionData connectionData = session.getConnectionData();
            streamInfo << "connection event at " << connectionData.endpoint
                << " remote: " << connectionData.endpointPeer
                << " event: " << connectionEvent.toString();

            switch (connectionEvent)
            {
            case ConnectionEvent::CONNECTIONEVENT_DISCONNECTED:
                doConnect();    // do an automatic reconnect, in case the connection gets lost
                break;
            }
        });

        entityContainer.registerEntity(entityClientPtr, "Hl7Entity");

        doConnect(); // connect to the server
    }

    void doConnect()
    {
        SessionInfo sessionClient = entityContainer.connect("tcp://localhost:7000:delimiter_x:hl7", { {}, {},
            VariantStruct{ {ProtocolDelimiterX::KEY_DELIMITER, "\x1C\x0D"} },
            VariantStruct{  {RemoteEntityFormatHl7::PROPERTY_NAMESPACE, "hl7"},
                            {RemoteEntityFormatHl7::PROPERTY_ENTITY, "Hl7Entity"},
                            {RemoteEntityFormatHl7::PROPERTY_MESSAGESTART, "\x0B"} } });


        PeerId peerId = m_entityClient->createPublishPeer(sessionClient, "MyService");
        m_entityClient->connected(peerId);
    }

    ~HL7Client()
    {
        // release the thread
        entityContainer.terminatePollerLoop();
        m_thread.join();
    }

private:
    std::thread m_thread;
    RemoteEntityContainer entityContainer;
    std::shared_ptr<EntityClient> m_entityClient{};
};




static finalmq::StructBasePtr create_ORM_O01()
{
    std::shared_ptr<hl7::ORM_O01> msgPtr = std::make_shared< hl7::ORM_O01>();
    hl7::ORM_O01& msg = *msgPtr;

    // Date Time Now
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ssDateTime;
    ssDateTime << std::put_time(std::localtime(&in_time_t), "%Y%m%d%H%M%S");
    std::string dateTimeStr = ssDateTime.str();
    std::stringstream ssDate;
    ssDate << std::put_time(std::localtime(&in_time_t), "%Y%m%d");
    std::string dateStr = ssDate.str();

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

    return msgPtr;
}



int main()
{
    std::shared_ptr<EntityClient> entityClientPtr = std::make_shared<EntityClient>();
    HL7Client client(entityClientPtr);

    for (int i = 0; i < 10; i++)
	{
        finalmq::StructBasePtr msg = create_ORM_O01();

        // send from outside
        entityClientPtr->sendHl7Message(msg);
	}

    std::this_thread::sleep_for(std::chrono::milliseconds(1000000));
    return 0;
}
