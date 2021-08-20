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


struct Mqtt5WillMessage
{
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
    std::string topic;
    Bytes payload;
};

struct Mqtt5ConnectData
{
    std::string protocol;
    unsigned int version = 0;
    unsigned int connectFlags = 0;
    unsigned int keepAlive = 0;
    std::unordered_map<unsigned int, Variant> properties;
    std::unordered_map<std::string, std::string> metainfo;
    std::string clientId;
    std::unique_ptr<Mqtt5WillMessage> willMessage;
    std::string username;
    std::string password;
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
    Mqtt5Serialization(int remainingSize, int indexRead, unsigned char header, char* buffer);
    int getReadIndex() const;

    bool deserializeConnect(Mqtt5ConnectData& data);
    bool deserializeConnAck(Mqtt5ConnAckData& data);
    bool deserializePublish(Mqtt5PublishData& data);
    bool deserializePubAck(Mqtt5PubAckData& data);
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

    static unsigned int sizeVarByteNumber(unsigned int number);
    static unsigned int sizeString(const std::string& str);
    static unsigned int sizeStringPair(std::string& key, std::string& value);
    static unsigned int sizeBinary(const Bytes& value);
    static unsigned int sizeProperties(const std::unordered_map<unsigned int, Variant>& properties, const std::unordered_map<std::string, std::string>& metainfo, unsigned int& sizePropertyPayload);

    bool serializeConnect(Mqtt5ConnectData& data);

    int             m_remainingSize = 0;
    int             m_indexRead = 0;
    unsigned char   m_header = 0;
    char*           m_buffer = nullptr;
};



}   // namespace finalmq
