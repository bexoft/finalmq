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


#include "finalmq/protocols/ProtocolMqtt5.h"
#include "finalmq/streamconnection/Socket.h"
#include "finalmq/protocolsession/ProtocolMessage.h"
#include "finalmq/protocolsession/ProtocolRegistry.h"
#include "finalmq/serializestruct/StructBase.h"

namespace finalmq {

static const ssize_t HEADERSIZE = 1;

const int ProtocolMqtt5::PROTOCOL_ID = 5;
const std::string ProtocolMqtt5::PROTOCOL_NAME = "mqtt5";


//////////////////////////////////////////////////////////
// Properties

class MqttPropertyId
{
public:
    enum Enum : std::int32_t {
        Invalid = 0,
        PayloadFormatIndicator = 1,    // Byte, PUBLISH, Will Properties
        MessageExpiryInterval = 2,    // Four Byte Integer, PUBLISH, Will Properties
        ContentType = 3,    // UTF-8 Encoded String, PUBLISH, Will Properties
        ResponseTopic = 8,    // UTF-8 Encoded String, PUBLISH, Will Properties
        CorrelationData = 9,    // Binary Data, PUBLISH, Will Properties
        SubscriptionIdentifier = 11,   // Variable Byte Integer, PUBLISH, SUBSCRIBE
        SessionExpiryInterval = 17,   // Four Byte Integer, CONNECT, CONNACK, DISCONNECT
        AssignedClientIdentifier = 18,   // UTF-8 Encoded String, CONNACK
        ServerKeepAlive = 19,   // Two Byte Integer, CONNACK
        AuthenticationMethod = 21,   // UTF-8 Encoded String, CONNECT, CONNACK, AUTH
        AuthenticationData = 22,   // Binary Data, CONNECT, CONNACK, AUTH
        RequestProblemInformation = 23,   // Byte, CONNECT
        WillDelayInterval = 24,   // Four Byte Integer, Will Properties
        RequestResponseInformation = 25,   // Byte, CONNECT
        ResponseInformation = 26,   // UTF-8 Encoded String, CONNACK
        ServerReference = 28,   // UTF-8 Encoded String, CONNACK, DISCONNECT
        ReasonString = 31,   // UTF-8 Encoded String, CONNACK, PUBACK, PUBREC, PUBREL, PUBCOMP, SUBACK, UNSUBACK, DISCONNECT, AUTH
        ReceiveMaximum = 33,   // Two Byte Integer, CONNECT, CONNACK
        TopicAliasMaximum = 34,   // Two Byte Integer, CONNECT, CONNACK
        TopicAlias = 35,   // Two Byte Integer, PUBLISH
        MaximumQoS = 36,   // Byte, CONNACK
        RetainAvailable = 37,   // Byte, CONNACK
        UserProperty = 38,   // UTF-8 String Pair, CONNECT, CONNACK, PUBLISH, Will Properties, PUBACK, PUBREC, PUBREL, PUBCOMP, SUBSCRIBE, SUBACK, UNSUBSCRIBE, UNSUBACK, DISCONNECT, AUTH
        MaximumPacketSize = 39,   // Four Byte Integer, CONNECT, CONNACK
        WildcardSubscriptionAvailable = 40,   // Byte, CONNACK
        SubscriptionIdentifierAvailable = 41,   // Byte, CONNACK
        SharedSubscriptionAvailable = 42,   // Byte, CONNACK
    };

    MqttPropertyId();
    MqttPropertyId(Enum en);
    operator const Enum& () const;
    operator Enum& ();
    const MqttPropertyId& operator =(Enum en);
    const std::string& toString() const;
    void fromString(const std::string& name);

private:
    Enum m_value = Invalid;
    static const finalmq::EnumInfo _enumInfo;
};


MqttPropertyId::MqttPropertyId()
{
}
MqttPropertyId::MqttPropertyId(Enum en)
    : m_value(en)
{
}
MqttPropertyId::operator const Enum& () const
{
    return m_value;
}
MqttPropertyId::operator Enum& ()
{
    return m_value;
}
const MqttPropertyId& MqttPropertyId::operator =(Enum en)
{
    m_value = en;
    return *this;
}
const std::string& MqttPropertyId::toString() const
{
    return _enumInfo.getMetaEnum().getNameByValue(m_value);
}
void MqttPropertyId::fromString(const std::string& name)
{
    m_value = static_cast<Enum>(_enumInfo.getMetaEnum().getValueByName(name));
}
const finalmq::EnumInfo MqttPropertyId::_enumInfo = {
    "private.MqttPropertyId", "", {
        {"Invalid", 0, ""},
        {"PayloadFormatIndicator", 1, ""},
        {"ContentType", 3, ""},
        {"ResponseTopic", 8, ""},
        {"CorrelationData", 9, ""},
        {"SubscriptionIdentifier", 11, ""},
        {"AssignedClientIdentifier", 18, ""},
        {"ServerKeepAlive", 19, ""},
        {"AuthenticationMethod", 21, ""},
        {"AuthenticationData", 22, ""},
        {"RequestProblemInformation", 23, ""},
        {"WillDelayInterval", 24, ""},
        {"RequestResponseInformation", 25, ""},
        {"ResponseInformation", 26, ""},
        {"ServerReference", 28, ""},
        {"ReasonString", 31, ""},
        {"ReceiveMaximum", 33, ""},
        {"TopicAliasMaximum", 34, ""},
        {"TopicAlias", 35, ""},
        {"MaximumQoS", 36, ""},
        {"RetainAvailable", 37, ""},
        {"UserProperty", 38, ""},
        {"MaximumPacketSize", 39, ""},
        {"WildcardSubscriptionAvailable", 40, ""},
        {"SubscriptionIdentifierAvailable", 41, ""},
        {"SharedSubscriptionAvailable", 42, ""},
     }
};


#define TypeNone                         0
#define TypeByte                         1
#define TypeTwoByteInteger               2
#define TypeFourByteInteger              3
#define TypeVariableByteInteger          4
#define TypeUTF8String                   5
#define TypeUTF8StringPair               6
#define TypeBinaryData                   7

static const int PropertyType[] = {
    TypeNone,                   //  0
    TypeByte,                   //  1 PayloadFormatIndicator
    TypeFourByteInteger,        //  2 MessageExpiryInterval
    TypeUTF8String,             //  3 ContentType
    TypeNone,                   //  4 
    TypeNone,                   //  5
    TypeNone,                   //  6
    TypeNone,                   //  7
    TypeUTF8String,             //  8 ResponseTopic
    TypeBinaryData,             //  9 CorrelationData
    TypeNone,                   // 10
    TypeVariableByteInteger,    // 11 SubscriptionIdentifier
    TypeNone,                   // 12
    TypeNone,                   // 13
    TypeNone,                   // 14
    TypeNone,                   // 15
    TypeNone,                   // 16
    TypeFourByteInteger,        // 17 SessionExpiryInterval
    TypeUTF8String,             // 18 AssignedClientIdentifier
    TypeTwoByteInteger,         // 19 ServerKeepAlive
    TypeNone,                   // 20
    TypeUTF8String,             // 21 AuthenticationMethod
    TypeBinaryData,             // 22 AuthenticationData
    TypeByte,                   // 23 RequestProblemInformation
    TypeFourByteInteger,        // 24 WillDelayInterval
    TypeByte,                   // 25 RequestResponseInformation
    TypeUTF8String,             // 26 ResponseInformation
    TypeNone,                   // 27
    TypeUTF8String,             // 28 ServerReference
    TypeNone,                   // 29
    TypeNone,                   // 30
    TypeUTF8String,             // 31 ReasonString
    TypeNone,                   // 32
    TypeTwoByteInteger,         // 33 ReceiveMaximum
    TypeTwoByteInteger,         // 34 TopicAliasMaximum
    TypeTwoByteInteger,         // 35 TopicAlias
    TypeByte,                   // 36 MaximumQoS
    TypeByte,                   // 37 RetainAvailable
    TypeUTF8StringPair,         // 38 UserProperty
    TypeFourByteInteger,        // 39 MaximumPacketSize
    TypeByte,                   // 40 WildcardSubscriptionAvailable
    TypeByte,                   // 41 SubscriptionIdentifierAvailable
    TypeByte,                   // 42 SharedSubscriptionAvailable
};

static int getPropertyType(int id)
{
    if (id > MqttPropertyId::Invalid && id <= MqttPropertyId::SharedSubscriptionAvailable)
    {
        return PropertyType[id];
    }
    return TypeNone;
}
///////////////////////////////////////////////////////////////////////////




#define HEADER_Command(header)      (((header) & 0xf0) >> 4)
#define HEADER_Dup(header)          (((header) & 0x08) >> 3)
#define HEADER_QoS(header)          (((header) & 0x06) >> 1)
#define HEADER_Retain(header)       (((header) & 0x01) >> 0)


#define COMMAND_CONNECT         1       // Server <-  Client, Connection request
#define COMMAND_CONNACK         2       // Server  -> Client, Connect acknowledgment
#define COMMAND_PUBLISH         3       // Server <-> Client, Publish message
#define COMMAND_PUBACK          4       // Server <-> Client, Publish acknowledgment(QoS 1)
#define COMMAND_PUBREC          5       // Server <-> Client, Publish received(QoS 2 delivery part 1)
#define COMMAND_PUBREL          6       // Server <-> Client, Publish release(QoS 2 delivery part 2)
#define COMMAND_PUBCOMP         7       // Server <-> Client, Publish complete(QoS 2 delivery part 3)
#define COMMAND_SUBSCRIBE       8       // Server <-  Client, Subscribe request
#define COMMAND_SUBACK          9       // Server  -> Client, Subscribe acknowledgment
#define COMMAND_UNSUBSCRIBE     10      // Server <-  Client, Unsubscribe request
#define COMMAND_UNSUBACK        11      // Server  -> Client, Unsubscribe acknowledgment
#define COMMAND_PING            12      // Server <-  Client, PING request
#define COMMAND_PINGRESP        13      // Server  -> Client, PING response
#define COMMAND_DISCONNECT      14      // Server <-> Client, Disconnect notification
#define COMMAND_AUTH            15      // Server <-> Client, Authentication exchange


ProtocolMqtt5::ProtocolMqtt5()
{
}


bool ProtocolMqtt5::receive(const SocketPtr& socket, int bytesToRead, std::deque<IMessagePtr>& messages)
{
    m_messages = &messages;
    bool ok = true;
    while ((bytesToRead > 0) && ok)
    {
        switch (m_state)
        {
        case State::WAITFORHEADER:
            ok = receiveHeader(socket, bytesToRead);
            break;
        case State::WAITFORLENGTH:
            ok = receiveRemainingSize(socket, bytesToRead);
            break;
        case State::WAITFORPAYLOAD:
            ok = receivePayload(socket, bytesToRead);
            break;
        default:
            assert(false);
            break;
        }
    }
    m_messages = nullptr;
    return ok;
}


bool ProtocolMqtt5::receiveHeader(const SocketPtr& socket, int& bytesToRead)
{
    assert(bytesToRead >= 1);
    assert(m_state == State::WAITFORHEADER);
    bool ret = false;

    int res = socket->receive(reinterpret_cast<char*>(&m_header), HEADERSIZE);
    if (res == HEADERSIZE)
    {
        bytesToRead -= HEADERSIZE;
        ret = true;
        m_remainingSize = 0;
        m_remainingSizeShift = 0;
        m_state = State::WAITFORLENGTH;
    }
    return ret;
}

bool ProtocolMqtt5::receiveRemainingSize(const SocketPtr& socket, int& bytesToRead)
{
    assert(bytesToRead >= 1);
    assert(m_state == State::WAITFORHEADER);
    bool ok = true;
    bool done = false;
    while (bytesToRead > 1 && !done && ok)
    {
        ok = false;
        if (m_remainingSizeShift <= 21)
        {
            char data;
            int res = socket->receive(&data, 1);
            if (res == 1)
            {
                ok = true;
                bytesToRead -= 1;
                m_remainingSize |= (data & 0x7f) << m_remainingSizeShift;
                if ((data & 0x80) == 0)
                {
                    done = true;
                    setPayloadSize();
                }
                m_remainingSizeShift += 7;
            }
        }
    }

    return ok;
}


void ProtocolMqtt5::setPayloadSize()
{
    assert(m_state == State::WAITFORLENGTH);
    assert(m_remainingSize >= 0);
    m_message = std::make_shared<ProtocolMessage>(0, HEADERSIZE);
    m_buffer = m_message->resizeReceiveBuffer(HEADERSIZE + m_remainingSize);
    m_buffer[0] = m_header;
    if (m_remainingSize != 0)
    {
        m_state = State::WAITFORPAYLOAD;
    }
    else
    {
        handlePayloadReceived();
    }
    m_sizeCurrent = 0;
}


bool ProtocolMqtt5::receivePayload(const SocketPtr& socket, int& bytesToRead)
{
    assert(m_state == State::WAITFORPAYLOAD);
    assert(m_sizeCurrent < m_remainingSize);
    bool ok = false;

    ssize_t sizeRead = m_remainingSize - m_sizeCurrent;
    if (bytesToRead < sizeRead)
    {
        sizeRead = bytesToRead;
    }
    int res = socket->receive(m_buffer + HEADERSIZE + m_sizeCurrent, static_cast<int>(sizeRead));
    if (res > 0)
    {
        int bytesReceived = res;
        assert(bytesReceived <= sizeRead);
        bytesToRead -= bytesReceived;
        assert(bytesToRead >= 0);
        m_sizeCurrent += bytesReceived;
        assert(m_sizeCurrent <= m_remainingSize);
        if (bytesReceived == sizeRead)
        {
            ok = true;
        }
        if (m_sizeCurrent == m_remainingSize)
        {
            m_sizeCurrent = 0;
            if (ok)
            {
                ok = processPayload();
            }
            if (ok)
            {
                handlePayloadReceived();
            }
        }
    }
    return ok;
}

void ProtocolMqtt5::handlePayloadReceived()
{
    m_messages->push_back(m_message);
    clearState();
}





#define CF_UserName(flags)       (((flags) & 0x80) >> 7) 
#define CF_Password(flags)       (((flags) & 0x40) >> 6) 
#define CF_WillRetain(flags)     (((flags) & 0x20) >> 5) 
#define CF_WillQoS(flags)        (((flags) & 0x18) >> 3) 
#define CF_WillFlag(flags)       (((flags) & 0x04) >> 2) 
#define CF_CleanStart(flags)     (((flags) & 0x02) >> 1) 


struct ProtocolMqtt5::MqttWillMessage
{
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
    std::string topic;
    Bytes payload;
};
struct ProtocolMqtt5::MqttConnectData
{
    std::string protocol;
    unsigned int version = 0;
    unsigned int connectFlags = 0;
    unsigned int keepAlive = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
    std::string clientId;
    std::unique_ptr<MqttWillMessage> willMessage;
    std::string username;
    std::string password;
};


bool ProtocolMqtt5::deserializeConnect(MqttConnectData& data)
{
    // protocol
    bool ok = readString(data.protocol);
    if (!ok || data.protocol != "MQTT")
    {
        return false;
    }

    // version
    ok = read1ByteNumber(data.version);
    if (!ok || data.version != 5)
    {
        return false;
    }

    // connect flags
    ok = read1ByteNumber(data.connectFlags);
    if (!ok)
    {
        return false;
    }

    // keep alive
    ok = read2ByteNumber(data.keepAlive);
    if (!ok)
    {
        return false;
    }

    // properties
    ok = readProperties(data.properties, data.metainfo);
    if (!ok)
    {
        return false;
    }

    // client ID
    std::string clientId;
    ok = readString(data.clientId);
    if (!ok)
    {
        return false;
    }

    // will message
    if (CF_WillFlag(data.connectFlags))
    {
        data.willMessage = std::make_unique<MqttWillMessage>();
        // will properties
        ok = readProperties(data.willMessage->properties, data.willMessage->metainfo);
        if (!ok)
        {
            return false;
        }

        // will topic
        ok = readString(data.willMessage->topic);
        if (!ok)
        {
            return false;
        }

        // will payload
        ok = readBinary(data.willMessage->payload);
        if (!ok)
        {
            return false;
        }
    }

    // username
    if (CF_UserName(data.connectFlags))
    {
        ok = readString(data.username);
        if (!ok)
        {
            return false;
        }
    }

    // password
    if (CF_Password(data.connectFlags))
    {
        ok = readString(data.password);
        if (!ok)
        {
            return false;
        }
    }

    return ok;
}



struct ProtocolMqtt5::MqttConnAckData
{
    bool sessionPresent = false;
    unsigned int reasoncode = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
};

#define CAF_SessionPresent(flags)   (((flags) & 0x01) >> 0)

bool ProtocolMqtt5::deserializeConnAck(MqttConnAckData& data)
{
    // connect ack flags
    unsigned int connackflags = 0;
    bool ok = read1ByteNumber(connackflags);
    if (!ok)
    {
        return false;
    }
    data.sessionPresent = CAF_SessionPresent(connackflags);

    // reason code
    ok = read1ByteNumber(data.reasoncode);
    if (!ok)
    {
        return false;
    }

    // properties
    ok = readProperties(data.properties, data.metainfo);
    if (!ok)
    {
        return false;
    }

    return ok;
}


struct ProtocolMqtt5::MqttPublishData
{
    std::string topicName;
    unsigned int packetId = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
};

bool ProtocolMqtt5::deserializePublish(MqttPublishData& data)
{
    // topic name
    bool ok = readString(data.topicName);
    if (!ok)
    {
        return false;
    }

    // packet identifier
    if (HEADER_QoS(m_header) > 0)
    {
        ok = read2ByteNumber(data.packetId);
        if (!ok)
        {
            return false;
        }
    }

    // properties
    ok = readProperties(data.properties, data.metainfo);
    if (!ok)
    {
        return false;
    }

    return ok;
}


struct ProtocolMqtt5::MqttPubAckData
{
    unsigned int packetId = 0;
    unsigned int reasoncode = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
};

bool ProtocolMqtt5::deserializePubAck(MqttPubAckData& data)
{
    // packet identifier
    bool ok = read2ByteNumber(data.packetId);
    if (!ok)
    {
        return false;
    }

    if (doesFit(1))
    {
        // reason code
        ok = read1ByteNumber(data.reasoncode);
        if (!ok)
        {
            return false;
        }

        // properties
        ok = readProperties(data.properties, data.metainfo);
        if (!ok)
        {
            return false;
        }
    }

    return ok;
}


struct ProtocolMqtt5::MqttSubscribeEntry
{
    std::string topic;
    unsigned int retainHandling = 0;
    bool retainAsPublished = false;
    bool noLocal = false;
    unsigned int qos = 0;
};

struct ProtocolMqtt5::MqttSubscribeData
{
    unsigned int packetId = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
    std::vector<MqttSubscribeEntry> subscriptions;
};

#define SO_RetainHandling(flags)        (((flags) & 0x30) >> 4)
#define SO_RetainAsPublished(flags)     (((flags) & 0x08) >> 3)
#define SO_NoLocal(flags)               (((flags) & 0x04) >> 2)
#define SO_QoS(flags)                   (((flags) & 0x03) >> 0)


bool ProtocolMqtt5::deserializeSubscribe(MqttSubscribeData& data)
{
    // packet identifier
    bool ok = read2ByteNumber(data.packetId);
    if (!ok)
    {
        return false;
    }

    // properties
    ok = readProperties(data.properties, data.metainfo);
    if (!ok)
    {
        return false;
    }

    // at least one subscription must be available
    if (!doesFit(1))
    {
        return false;
    }

    while (doesFit(1))
    {
        MqttSubscribeEntry entry;
        ok = readString(entry.topic);
        if (!ok)
        {
            return false;
        }
        unsigned int option = 0;
        ok = read1ByteNumber(option);
        if (!ok)
        {
            return false;
        }
        entry.retainHandling = SO_RetainHandling(option);
        entry.retainAsPublished = SO_RetainAsPublished(option);
        entry.noLocal = SO_NoLocal(option);
        entry.qos = SO_QoS(option);

        data.subscriptions.emplace_back(std::move(entry));
    }

    return ok;
}

struct ProtocolMqtt5::MqttSubAckData
{
    unsigned int packetId = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
    std::vector<unsigned int> reasoncodes;
};

bool ProtocolMqtt5::deserializeSubAck(MqttSubAckData& data)
{
    // packet identifier
    bool ok = read2ByteNumber(data.packetId);
    if (!ok)
    {
        return false;
    }

    // properties
    ok = readProperties(data.properties, data.metainfo);
    if (!ok)
    {
        return false;
    }

    // at least one ack must be available
    if (!doesFit(1))
    {
        return false;
    }

    while (doesFit(1))
    {
        data.reasoncodes.push_back(-1);
        ok = read1ByteNumber(data.reasoncodes.back());
        if (!ok)
        {
            return false;
        }
    }

    return ok;
}


struct ProtocolMqtt5::MqttUnsubscribeData
{
    unsigned int packetId = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
    std::vector<std::string> topics;
};



bool ProtocolMqtt5::deserializeUnsubscribe(MqttUnsubscribeData& data)
{
    // packet identifier
    bool ok = read2ByteNumber(data.packetId);
    if (!ok)
    {
        return false;
    }

    // properties
    ok = readProperties(data.properties, data.metainfo);
    if (!ok)
    {
        return false;
    }

    // at least one unsubscription must be available
    if (!doesFit(1))
    {
        return false;
    }

    while (doesFit(1))
    {
        std::string topic;
        ok = readString(topic);
        if (!ok)
        {
            return false;
        }
        data.topics.emplace_back(std::move(topic));
    }

    return ok;
}




struct ProtocolMqtt5::MqttDisconnectData
{
    unsigned int reasoncode = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
};



bool ProtocolMqtt5::deserializeDisconnect(MqttDisconnectData& data)
{
    bool ok = true;

    if (doesFit(1))
    {
        // reason code
        ok = read2ByteNumber(data.reasoncode);
        if (!ok)
        {
            return false;
        }

        // properties
        ok = readProperties(data.properties, data.metainfo);
        if (!ok)
        {
            return false;
        }
    }

    return ok;
}



struct ProtocolMqtt5::MqttAuthData
{
    unsigned int reasoncode = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
};



bool ProtocolMqtt5::deserializeAuth(MqttAuthData& data)
{
    bool ok = true;

    if (doesFit(1))
    {
        // reason code
        ok = read2ByteNumber(data.reasoncode);
        if (!ok)
        {
            return false;
        }

        // properties
        ok = readProperties(data.properties, data.metainfo);
        if (!ok)
        {
            return false;
        }
    }

    return ok;
}






bool ProtocolMqtt5::processPayload()
{
    m_indexRead = 1;

    bool ok = false;
    int command = HEADER_Command(m_header);
    switch (command)
    {
    case COMMAND_CONNECT:
        {
            MqttConnectData data;
            ok = deserializeConnect(data);
        }
        break;
    case COMMAND_CONNACK:
        {
            MqttConnAckData data;
            ok = deserializeConnAck(data);
        }
        break;
    case COMMAND_PUBLISH:
        {
            MqttPublishData data;
            ok = deserializePublish(data);
        }
        break;
    case COMMAND_PUBACK:
        {
            MqttPubAckData data;
            ok = deserializePubAck(data);
        }
        break;
    case COMMAND_PUBREC:
        {
            MqttPubAckData data;
            ok = deserializePubAck(data);
        }
        break;
    case COMMAND_PUBREL:
        {
            MqttPubAckData data;
            ok = deserializePubAck(data);
        }
        break;
    case COMMAND_PUBCOMP:
        {
            MqttPubAckData data;
            ok = deserializePubAck(data);
        }
        break;
    case COMMAND_SUBSCRIBE:
        {
            MqttSubscribeData data;
            ok = deserializeSubscribe(data);
        }
        break;
    case COMMAND_SUBACK:
        {
            MqttSubAckData data;
            ok = deserializeSubAck(data);
        }
        break;
    case COMMAND_UNSUBSCRIBE:
        {
            MqttUnsubscribeData data;
            ok = deserializeUnsubscribe(data);
        }
        break;
    case COMMAND_UNSUBACK:
        {
            MqttSubAckData data;
            ok = deserializeSubAck(data);
        }
        break;
    case COMMAND_PING:
        break;
    case COMMAND_PINGRESP:
        break;
    case COMMAND_DISCONNECT:
        {
            MqttDisconnectData data;
            ok = deserializeDisconnect(data);
        }
        break;
    case COMMAND_AUTH:
        {
            MqttAuthData data;
            ok = deserializeAuth(data);
        }
        break;
    default:
        break;
    }
    return ok;
}








bool ProtocolMqtt5::doesFit(int size) const
{
    bool ok = (m_indexRead + size <= m_remainingSize);
    return ok;
}



bool ProtocolMqtt5::read1ByteNumber(unsigned int& number)
{
    if (doesFit(1))
    {
        unsigned int byte1 = m_buffer[m_indexRead];
        ++m_indexRead;
        number = byte1;
        return true;
    }
    number = 0;
    return false;
}

bool ProtocolMqtt5::read2ByteNumber(unsigned int& number)
{
    if (doesFit(2))
    {
        unsigned int byte1 = m_buffer[m_indexRead];
        ++m_indexRead;
        unsigned int byte2 = m_buffer[m_indexRead];
        ++m_indexRead;
        number = (byte1 << 8) | byte2;
        return true;
    }
    number = 0;
    return false;
}

bool ProtocolMqtt5::read4ByteNumber(unsigned int& number)
{
    if (doesFit(4))
    {
        unsigned int byte1 = m_buffer[m_indexRead];
        ++m_indexRead;
        unsigned int byte2 = m_buffer[m_indexRead];
        ++m_indexRead;
        unsigned int byte3 = m_buffer[m_indexRead];
        ++m_indexRead;
        unsigned int byte4 = m_buffer[m_indexRead];
        ++m_indexRead;
        number = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
        return true;
    }
    number = 0;
    return false;
}

bool ProtocolMqtt5::readVarByteNumber(unsigned int& number)
{
    bool ok = true;
    bool done = false;
    int shift = 0;
    while (!done && ok)
    {
        ok = false;
        if (m_indexRead < m_remainingSize && shift <= 21)
        {
            ok = true;
            char data = m_buffer[m_indexRead];
            ++m_indexRead;
            number |= (data & 0x7f) << shift;
            if ((data & 0x80) == 0)
            {
                done = true;
            }
            shift += 7;
        }
    }
    return ok;
}

bool ProtocolMqtt5::readString(std::string& str)
{
    unsigned int size = 0;
    bool ok = read2ByteNumber(size);
    if (ok && doesFit(size))
    {
        str.insert(0, &m_buffer[m_indexRead], size);
        m_indexRead += size;
    }
    else
    {
        str.clear();
    }
    return ok;
}

bool ProtocolMqtt5::readStringPair(std::string& key, std::string& value)
{
    bool ok = readString(key);
    if (!ok)
    {
        return false;
    }
    ok = readString(value);
    return ok;
}


bool ProtocolMqtt5::readBinary(Bytes& value)
{
    unsigned int size = 0;
    bool ok = read2ByteNumber(size);
    if (ok && doesFit(size))
    {
        value.insert(value.end(), &m_buffer[m_indexRead], &m_buffer[m_indexRead + size]);
        m_indexRead += size;
    }
    else
    {
        value.clear();
    }
    return ok;
}





bool ProtocolMqtt5::readProperties(std::unordered_map<unsigned int, Variant>& properties, std::unordered_map<std::string, std::string>& metainfo)
{
    unsigned int size = 0;
    bool ok = readVarByteNumber(size);
    if (!ok || !doesFit(size))
    {
        return false;
    }

    int indexEndProperties = m_indexRead + size;
    bool done = (size == 0);
    while (!done && ok)
    {
        ok = false;
        if (m_indexRead < indexEndProperties)
        {
            ok = true;
            unsigned int id = 0;
            ok = readVarByteNumber(id);
            if (!ok)
            {
                return false;
            }
            MqttPropertyId propertyId(static_cast<MqttPropertyId::Enum>(id));
            if (propertyId == MqttPropertyId::Invalid)
            {
                return false;
            }
            int type = getPropertyType(propertyId);
            switch (type)
            {
            case TypeNone:
                ok = false;
                break;
            case TypeByte:
                {
                    unsigned int value;
                    ok = read1ByteNumber(value);
                    if (ok)
                    {
                        properties[propertyId] = value;
                    }
                }
                break;
            case TypeTwoByteInteger:
                {
                    unsigned int value;
                    ok = read2ByteNumber(value);
                    if (ok)
                    {
                        properties[propertyId] = value;
                    }
                }
                break;
            case TypeFourByteInteger:
                {
                    unsigned int value;
                    ok = read4ByteNumber(value);
                    if (ok)
                    {
                        properties[propertyId] = value;
                    }
                }
                break;
            case TypeVariableByteInteger:
                {
                    unsigned int value;
                    ok = readVarByteNumber(value);
                    if (ok)
                    {
                        properties[propertyId] = value;
                    }
                }
                break;
            case TypeUTF8String:
                {
                    std::string value;
                    ok = readString(value);
                    if (ok)
                    {
                        properties[propertyId] = value;
                    }
                }
                break;
            case TypeUTF8StringPair:
                {
                    std::string key;
                    std::string value;
                    ok = readStringPair(key, value);
                    if (ok)
                    {
                        if (propertyId == MqttPropertyId::UserProperty)
                        {
                            metainfo[std::move(key)] = std::move(value);
                        }
                    }
                }
                break;
            case TypeBinaryData:
                {
                    Bytes value;
                    ok = readBinary(value);
                    if (ok)
                    {
                        properties[propertyId] = value;
                    }
                }
                break;
            default:
                assert(false);
                ok = false;
                break;
            }
            if (m_indexRead == indexEndProperties)
            {
                done = true;
            }
        }
    }

    return ok;
}


void ProtocolMqtt5::clearState()
{
    m_sizeCurrent = 0;
    m_sizePayload = 0;
    m_message = nullptr;
    m_buffer = nullptr;
    m_state = State::WAITFORHEADER;
}


// IProtocol
void ProtocolMqtt5::setCallback(const std::weak_ptr<IProtocolCallback>& callback)
{
    m_callback = callback;
}

void ProtocolMqtt5::setConnection(const IStreamConnectionPtr& connection)
{
    m_connection = connection;
}

std::uint32_t ProtocolMqtt5::getProtocolId() const 
{
    return PROTOCOL_ID;
}

bool ProtocolMqtt5::areMessagesResendable() const 
{
    return false;
}

bool ProtocolMqtt5::doesSupportMetainfo() const 
{
    return true;
}

bool ProtocolMqtt5::doesSupportSession() const 
{
    return true;
}

bool ProtocolMqtt5::needsReply() const 
{
    return false;
}

bool ProtocolMqtt5::isMultiConnectionSession() const 
{
    return false;
}

bool ProtocolMqtt5::isSendRequestByPoll() const 
{
    return false;
}

bool ProtocolMqtt5::doesSupportFileTransfer() const 
{
    return false;
}

IProtocol::FuncCreateMessage ProtocolMqtt5::getMessageFactory() const
{
    return []() {
        return std::make_shared<ProtocolMessage>(PROTOCOL_ID);
    };
}

bool ProtocolMqtt5::sendMessage(IMessagePtr /*message*/) 
{
    return false;
}

void ProtocolMqtt5::moveOldProtocolState(IProtocol& /*protocolOld*/) 
{
}

bool ProtocolMqtt5::received(const IStreamConnectionPtr& /*connection*/, const SocketPtr& socket, int bytesToRead) 
{
    std::deque<IMessagePtr> messages;
    bool ok = receive(socket, bytesToRead, messages);
    return ok;
}

hybrid_ptr<IStreamConnectionCallback> ProtocolMqtt5::connected(const IStreamConnectionPtr& /*connection*/) 
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->connected();
    }
    return nullptr;
}

void ProtocolMqtt5::disconnected(const IStreamConnectionPtr& /*connection*/) 
{
    auto callback = m_callback.lock();
    if (callback)
    {
        callback->disconnected();
    }
}

IMessagePtr ProtocolMqtt5::pollReply(std::deque<IMessagePtr>&& /*messages*/) 
{
    return {};
}


//---------------------------------------
// ProtocolMqtt5Factory
//---------------------------------------

struct RegisterProtocolMqtt5Factory
{
    RegisterProtocolMqtt5Factory()
    {
        ProtocolRegistry::instance().registerProtocolFactory(ProtocolMqtt5::PROTOCOL_NAME, ProtocolMqtt5::PROTOCOL_ID, std::make_shared<ProtocolMqtt5Factory>());
    }
} g_registerProtocolMqtt5Factory;




// IProtocolFactory
IProtocolPtr ProtocolMqtt5Factory::createProtocol()
{
    return std::make_shared<ProtocolMqtt5>();
}

}   // namespace finalmq
