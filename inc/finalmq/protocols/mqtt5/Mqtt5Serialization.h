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

#pragma once

//#include "finalmq/streamconnection/IMessage.h"
#include "finalmq/metadata/MetaType.h"
#include "finalmq/variant/Variant.h"

#include <unordered_map>

namespace finalmq {


enum class Mqtt5Command : unsigned int
{
    COMMAND_CONNECT         = 1,       // Server <-  Client, Connection request
    COMMAND_CONNACK         = 2,       // Server  -> Client, Connect acknowledgment
    COMMAND_PUBLISH         = 3,       // Server <-> Client, Publish message
    COMMAND_PUBACK          = 4,       // Server <-> Client, Publish acknowledgment(QoS 1)
    COMMAND_PUBREC          = 5,       // Server <-> Client, Publish received(QoS 2 delivery part 1)
    COMMAND_PUBREL          = 6,       // Server <-> Client, Publish release(QoS 2 delivery part 2)
    COMMAND_PUBCOMP         = 7,       // Server <-> Client, Publish complete(QoS 2 delivery part 3)
    COMMAND_SUBSCRIBE       = 8,       // Server <-  Client, Subscribe request
    COMMAND_SUBACK          = 9,       // Server  -> Client, Subscribe acknowledgment
    COMMAND_UNSUBSCRIBE     = 10,      // Server <-  Client, Unsubscribe request
    COMMAND_UNSUBACK        = 11,      // Server  -> Client, Unsubscribe acknowledgment
    COMMAND_PINGREQ         = 12,      // Server <-  Client, PING request
    COMMAND_PINGRESP        = 13,      // Server  -> Client, PING response
    COMMAND_DISCONNECT      = 14,      // Server <-> Client, Disconnect notification
    COMMAND_AUTH            = 15,      // Server <-> Client, Authentication exchange
};


struct Mqtt5WillMessage
{
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
    std::string topic;
    Bytes payload;
    bool retain = false;
    unsigned int qos = 0;
};

struct Mqtt5ConnectData
{
    std::string protocol;
    unsigned int version = 0;
    unsigned int keepAlive = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
    std::string clientId;
    std::unique_ptr<Mqtt5WillMessage> willMessage;
    std::string username;
    std::string password;
    bool cleanStart = false;
};

struct Mqtt5ConnAckData
{
    bool sessionPresent = false;
    unsigned int reasoncode = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
};

struct Mqtt5PublishData
{
    unsigned int qos = 0;
    bool dup = false;
    bool retain = false;
    std::string topicName;
    unsigned int packetId = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
};

struct Mqtt5PubAckData
{
    unsigned int packetId = 0;
    unsigned int reasoncode = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
};

struct Mqtt5SubscribeEntry
{
    std::string topic;
    unsigned int retainHandling = 0;
    bool retainAsPublished = false;
    bool noLocal = false;
    unsigned int qos = 0;
};

struct Mqtt5SubscribeData
{
    unsigned int packetId = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
    std::vector<Mqtt5SubscribeEntry> subscriptions;
};

struct Mqtt5SubAckData
{
    unsigned int packetId = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
    std::vector<unsigned int> reasoncodes;
};

struct Mqtt5UnsubscribeData
{
    unsigned int packetId = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
    std::vector<std::string> topics;
};

struct Mqtt5DisconnectData
{
    unsigned int reasoncode = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
};

struct Mqtt5AuthData
{
    unsigned int reasoncode = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
};



class Mqtt5Serialization
{
public:
    Mqtt5Serialization(char* buffer, unsigned int sizeBuffer, unsigned int indexBuffer);
    int getReadIndex() const;

    bool deserializeConnect(Mqtt5ConnectData& data);
    bool deserializeConnAck(Mqtt5ConnAckData& data);
    bool deserializePublish(Mqtt5PublishData& data);
    bool deserializePubAck(Mqtt5PubAckData& data, Mqtt5Command command);
    bool deserializeSubscribe(Mqtt5SubscribeData& data);
    bool deserializeSubAck(Mqtt5SubAckData& data);
    bool deserializeUnsubscribe(Mqtt5UnsubscribeData& data);
    bool deserializeDisconnect(Mqtt5DisconnectData& data);
    bool deserializeAuth(Mqtt5AuthData& data);

    bool doesFit(int size) const;
    bool read1ByteNumber(unsigned int& number);
    bool read2ByteNumber(unsigned int& number);
    bool read4ByteNumber(unsigned int& number);
    bool readVarByteNumber(unsigned int& number);
    bool readString(std::string& str);
    bool readStringPair(std::string& key, std::string& value);
    bool readBinary(Bytes& value);
    bool readProperties(std::unordered_map<unsigned int, Variant>& properties, std::unordered_map<std::string, std::string>& metainfo);

    void write1ByteNumber(unsigned int number);
    void write2ByteNumber(unsigned int number);
    void write4ByteNumber(unsigned int number);
    void writeVarByteNumber(unsigned int number);
    void writeString(const std::string& str);
    void writeStringPair(const std::string& key, const std::string& value);
    void writeBinary(const Bytes& value);
    void writeProperties(const std::unordered_map<unsigned int, Variant>& properties, const std::unordered_map<std::string, std::string>& metainfo, unsigned int sizePropertyPayload);

    static unsigned int sizeVarByteNumber(unsigned int number);
    static unsigned int sizeString(const std::string& str);
    static unsigned int sizeStringPair(const std::string& key, const std::string& value);
    static unsigned int sizeBinary(const Bytes& value);
    static unsigned int sizeProperties(const std::unordered_map<unsigned int, Variant>& properties, const std::unordered_map<std::string, std::string>& metainfo, unsigned int& sizePropertyPayload);

    unsigned int sizeConnect(const Mqtt5ConnectData& data, unsigned int& sizePropPayload, unsigned int& sizePropWillMessage) const;
    unsigned int sizeConnAck(const Mqtt5ConnAckData& data, unsigned int& sizePropPayload) const;
    unsigned int sizePublish(const Mqtt5PublishData& data, unsigned int& sizePropPayload) const;
    unsigned int sizePubAck(const Mqtt5PubAckData& data, unsigned int& sizePropPayload) const;
    unsigned int sizeSubscribe(const Mqtt5SubscribeData& data, unsigned int& sizePropPayload) const;
    unsigned int sizeSubAck(const Mqtt5SubAckData& data, unsigned int& sizePropPayload) const;
    unsigned int sizeUnsubscribe(const Mqtt5UnsubscribeData& data, unsigned int& sizePropPayload) const;
    unsigned int sizeDisconnect(const Mqtt5DisconnectData& data, unsigned int& sizePropPayload) const;
    unsigned int sizeAuth(const Mqtt5AuthData& data, unsigned int& sizePropPayload) const;

    void serializeConnect(const Mqtt5ConnectData& data, unsigned int sizePayload, unsigned int sizePropPayload, unsigned int sizePropWillMessage);
    void serializeConnAck(const Mqtt5ConnAckData& data, unsigned int sizePayload, unsigned int sizePropPayload);
    void serializePublish(const Mqtt5PublishData& data, unsigned int sizePayload, unsigned int sizePropPayload);
    void serializePubAck(const Mqtt5PubAckData& data, Mqtt5Command command, unsigned int sizePayload, unsigned int sizePropPayload);
    void serializeSubscribe(const Mqtt5SubscribeData& data, unsigned int sizePayload, unsigned int sizePropPayload);
    void serializeSubAck(const Mqtt5SubAckData& data, unsigned int sizePayload, unsigned int sizePropPayload);
    void serializeUnsubscribe(const Mqtt5UnsubscribeData& data, unsigned int sizePayload, unsigned int sizePropPayload);
    void serializePingReq(const Mqtt5UnsubscribeData& data);
    void serializePingResp(const Mqtt5UnsubscribeData& data);
    void serializeDisconnect(const Mqtt5DisconnectData& data, unsigned int sizePayload, unsigned int sizePropPayload);
    void serializeAuth(const Mqtt5AuthData& data, unsigned int sizePayload, unsigned int sizePropPayload);

    char*           m_buffer = nullptr;
    unsigned int    m_sizeBuffer = 0;
    unsigned int    m_indexBuffer = 0;
};



}   // namespace finalmq
