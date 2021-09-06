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


#include "finalmq/protocols/mqtt5/Mqtt5Serialization.h"
#include "finalmq/protocols/mqtt5/Mqtt5Properties.h"
#include "finalmq/variant/VariantValues.h"

namespace finalmq {

static const ssize_t HEADERSIZE = 1;


#define HEADER_Low4bits(header)     (((header) >> 0) & 0x0f)
#define HEADER_Dup(header)          (((header) >> 3) & 0x01)
#define HEADER_QoS(header)          (((header) >> 1) & 0x03)
#define HEADER_Retain(header)       (((header) >> 0) & 0x01)

#define HEADER_SetCommand(value)    ((static_cast<unsigned int>(value) & 0x0f) << 4)
#define HEADER_SetLow4bits(value)   (((value) & 0x0f) << 0)
#define HEADER_SetDup(value)        (((value) & 0x01) << 3)
#define HEADER_SetQoS(value)        (((value) & 0x03) << 1)
#define HEADER_SetRetain(value)     (((value) & 0x01) << 0)

#define CF_UserName(flags)       (((flags) >> 7) & 0x01) 
#define CF_Password(flags)       (((flags) >> 6) & 0x01) 
#define CF_WillRetain(flags)     (((flags) >> 5) & 0x01) 
#define CF_WillQoS(flags)        (((flags) >> 3) & 0x03) 
#define CF_WillFlag(flags)       (((flags) >> 2) & 0x01) 
#define CF_CleanStart(flags)     (((flags) >> 1) & 0x01) 

#define CF_SetUserName(value)    (((value) & 0x01) << 7)
#define CF_SetPassword(value)    (((value) & 0x01) << 6) 
#define CF_SetWillRetain(value)  (((value) & 0x01) << 5)
#define CF_SetWillQoS(value)     (((value) & 0x03) << 3)
#define CF_SetWillFlag(value)    (((value) & 0x01) << 2)
#define CF_SetCleanStart(value)  (((value) & 0x01) << 1)


Mqtt5Serialization::Mqtt5Serialization(char* buffer, unsigned int sizeBuffer, unsigned int indexBuffer)
    : m_buffer(reinterpret_cast<std::uint8_t*>(buffer))
    , m_sizeBuffer(sizeBuffer)
    , m_indexBuffer(indexBuffer)
{

}


int Mqtt5Serialization::getReadIndex() const
{
    return m_indexBuffer;
}


bool Mqtt5Serialization::deserializeConnect(Mqtt5ConnectData& data)
{
    if (HEADER_Low4bits(m_buffer[0]) != 0)
    {
        return false;
    }

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
    unsigned int connectFlags = 0;
    ok = read1ByteNumber(connectFlags);
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
    ok = readString(data.clientId);
    if (!ok)
    {
        return false;
    }

    // will message
    if (CF_WillFlag(connectFlags))
    {
        data.willMessage = std::make_unique<Mqtt5WillMessage>();
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

        data.willMessage->retain = CF_WillRetain(connectFlags);
        data.willMessage->qos = CF_WillQoS(connectFlags);
    }

    // username
    if (CF_UserName(connectFlags))
    {
        ok = readString(data.username);
        if (!ok)
        {
            return false;
        }
    }

    // password
    if (CF_Password(connectFlags))
    {
        ok = readString(data.password);
        if (!ok)
        {
            return false;
        }
    }

    data.cleanStart = CF_CleanStart(connectFlags);

    return ok;
}

unsigned int Mqtt5Serialization::sizeConnect(const Mqtt5ConnectData& data, unsigned int& sizePropPayload, unsigned int& sizePropWillMessage)
{
    unsigned int size = 0;

    // protocol
    size += sizeString("MQTT");

    // version (1), connect flags (1), keep alive (2)
    size += 1 + 1 + 2;

    // properties
    size += sizeProperties(data.properties, data.metainfo, sizePropPayload);

    // client ID
    size += sizeString(data.clientId);

    // will message
    sizePropWillMessage = 0;
    if (data.willMessage)
    {
        // will properties
        size += sizeProperties(data.willMessage->properties, data.willMessage->metainfo, sizePropWillMessage);

        // will topic
        size += sizeString(data.willMessage->topic);

        // will payload
        size += sizeBinary(data.willMessage->payload);
    }

    // username
    if (!data.username.empty())
    {
        size += sizeString(data.username);
    }

    // password
    if (!data.password.empty())
    {
        size += sizeString(data.password);
    }

    return size;
}

void Mqtt5Serialization::serializeConnect(const Mqtt5ConnectData& data, unsigned int sizePayload, unsigned int sizePropPayload, unsigned int sizePropWillMessage)
{
    unsigned int header = HEADER_SetCommand(Mqtt5Command::COMMAND_CONNECT);
    write1ByteNumber(header);
    writeVarByteNumber(sizePayload);

    // protocol
    writeString("MQTT");

    // version
    write1ByteNumber(5);

    // connect flags
    unsigned int connectFlags = 0;
    if (data.willMessage)
    {
        connectFlags |= CF_SetWillFlag(1);
        connectFlags |= CF_SetWillRetain(data.willMessage->retain ? 1 : 0);
        connectFlags |= CF_SetWillQoS(data.willMessage->qos);
    }
    if (!data.username.empty())
    {
        connectFlags |= CF_SetUserName(1);
    }
    if (!data.password.empty())
    {
        connectFlags |= CF_SetPassword(1);
    }
    connectFlags |= CF_SetCleanStart(data.cleanStart ? 1 : 0);


    write1ByteNumber(connectFlags);

    // keep alive
    write2ByteNumber(data.keepAlive);

    // properties
    writeProperties(data.properties, data.metainfo, sizePropPayload);

    // client ID
    writeString(data.clientId);

    // will message
    if (data.willMessage)
    {
        // will properties
        writeProperties(data.willMessage->properties, data.willMessage->metainfo, sizePropWillMessage);

        // will topic
        writeString(data.willMessage->topic);

        // will payload
        writeBinary(data.willMessage->payload);
    }

    // username
    if (!data.username.empty())
    {
        writeString(data.username);
    }

    // password
    if (!data.password.empty())
    {
        writeString(data.password);
    }

    assert(m_indexBuffer == m_sizeBuffer);
}



#define CAF_SessionPresent(flags)       (((flags) & 0x01) >> 0)
#define CAF_SetSessionPresent(value)    (((value) & 0x01) << 1)

bool Mqtt5Serialization::deserializeConnAck(Mqtt5ConnAckData& data)
{
    if (HEADER_Low4bits(m_buffer[0]) != 0)
    {
        return false;
    }

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

unsigned int Mqtt5Serialization::sizeConnAck(const Mqtt5ConnAckData& data, unsigned int& sizePropPayload)
{
    unsigned int size = 0;

    // connect ack flags (1), reason code (1)
    size += 1 + 1;

    // properties
    size += sizeProperties(data.properties, data.metainfo, sizePropPayload);

    return size;
}

void Mqtt5Serialization::serializeConnAck(const Mqtt5ConnAckData& data, unsigned int sizePayload, unsigned int sizePropPayload)
{
    unsigned int header = HEADER_SetCommand(Mqtt5Command::COMMAND_CONNACK);
    write1ByteNumber(header);
    writeVarByteNumber(sizePayload);

    // connect ack flags
    unsigned int connackflags = 0;
    connackflags |= CAF_SetSessionPresent(data.sessionPresent);
    write1ByteNumber(connackflags);

    // reason code
    write1ByteNumber(data.reasoncode);

    // properties
    writeProperties(data.properties, data.metainfo, sizePropPayload);

    assert(m_indexBuffer == m_sizeBuffer);
}

bool Mqtt5Serialization::deserializePublish(Mqtt5PublishData& data)
{
    data.qos = HEADER_QoS(m_buffer[0]);
    data.dup = HEADER_Dup(m_buffer[0]);
    data.retain = HEADER_Retain(m_buffer[0]);

    // topic name
    bool ok = readString(data.topic);
    if (!ok)
    {
        return false;
    }

    // packet identifier
    if (data.qos > 0)
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

unsigned int Mqtt5Serialization::sizePublish(const Mqtt5PublishData& data, unsigned int& sizePropPayload)
{
    unsigned int size = 0;

    // topic name
    size += sizeString(data.topic);

    // packet identifier
    if (data.qos > 0)
    {
        size += 2;
    }

    // properties
    size += sizeProperties(data.properties, data.metainfo, sizePropPayload);

    return size;
}

void Mqtt5Serialization::serializePublish(const Mqtt5PublishData& data, unsigned int sizePayload, unsigned int sizePropPayload, std::uint8_t*& bufferPacketId)
{
    bufferPacketId = nullptr;
    unsigned int header = 0;
    header |= HEADER_SetCommand(Mqtt5Command::COMMAND_PUBLISH);
    header |= HEADER_SetQoS(data.qos);
    header |= HEADER_SetDup(data.dup);
    header |= HEADER_SetRetain(data.retain);
    write1ByteNumber(header);
    writeVarByteNumber(sizePayload);

    // topic name
    writeString(data.topic);

    // packet identifier
    if (data.qos > 0)
    {
        bufferPacketId = &m_buffer[m_indexBuffer];
        write2ByteNumber(data.packetId);
    }

    // properties
    writeProperties(data.properties, data.metainfo, sizePropPayload);

    assert(m_indexBuffer == m_sizeBuffer);
}

bool Mqtt5Serialization::deserializePubAck(Mqtt5PubAckData& data, Mqtt5Command command)
{
    if (command == Mqtt5Command::COMMAND_PUBREL)
    {
        if (HEADER_Low4bits(m_buffer[0]) != 0x02)
        {
            return false;
        }
    }
    else
    {
        if (HEADER_Low4bits(m_buffer[0]) != 0x00)
        {
            return false;
        }
    }

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

unsigned int Mqtt5Serialization::sizePubAck(const Mqtt5PubAckData& data, unsigned int& sizePropPayload)
{
    unsigned int size = 0;

    // packet identifier
    size += 2;

    if (data.reasoncode != 0 || !data.properties.empty() || !data.metainfo.empty())
    {
        // reason code
        size += 1;

        // properties
        size += sizeProperties(data.properties, data.metainfo, sizePropPayload);
    }

    return size;
}

void Mqtt5Serialization::serializePubAck(const Mqtt5PubAckData& data, Mqtt5Command command, unsigned int sizePayload, unsigned int sizePropPayload)
{
    unsigned int header = 0;
    header |= HEADER_SetCommand(command);
    if (command == Mqtt5Command::COMMAND_PUBREL)
    {
        header |= HEADER_SetLow4bits(0x02);
    }
    write1ByteNumber(header);
    writeVarByteNumber(sizePayload);

    // packet identifier
    write2ByteNumber(data.packetId);

    if (data.reasoncode != 0 || !data.properties.empty() || !data.metainfo.empty())
    {
        // reason code
        write1ByteNumber(data.reasoncode);

        // properties
        writeProperties(data.properties, data.metainfo, sizePropPayload);
    }

    assert(m_indexBuffer == m_sizeBuffer);
}



#define SO_RetainHandling(flags)        (((flags) >> 4) & 0x03)
#define SO_RetainAsPublished(flags)     (((flags) >> 3) & 0x01)
#define SO_NoLocal(flags)               (((flags) >> 2) & 0x01)
#define SO_QoS(flags)                   (((flags) >> 0) & 0x03)

#define SO_SetRetainHandling(value)     (((value) & 0x03) << 4)
#define SO_SetRetainAsPublished(value)  (((value) & 0x01) << 3)
#define SO_SetNoLocal(value)            (((value) & 0x01) << 2)
#define SO_SetQoS(value)                (((value) & 0x03) << 0)


bool Mqtt5Serialization::deserializeSubscribe(Mqtt5SubscribeData& data)
{
    if (HEADER_Low4bits(m_buffer[0]) != 0x02)
    {
        return false;
    }

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
        Mqtt5SubscribeEntry entry;
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


unsigned int Mqtt5Serialization::sizeSubscribe(const Mqtt5SubscribeData& data, unsigned int& sizePropPayload)
{
    unsigned int size = 0;

    // packet identifier
    size += 2;

    // properties
    size += sizeProperties(data.properties, data.metainfo, sizePropPayload);

    // at least one subscription must be available
    assert(!data.subscriptions.empty());

    for (size_t i = 0; i < data.subscriptions.size(); ++i)
    {
        const Mqtt5SubscribeEntry& entry = data.subscriptions[i];
        size += sizeString(entry.topic);
    }
    // options
    size += static_cast<unsigned int>(data.subscriptions.size());

    return size;
}


void Mqtt5Serialization::serializeSubscribe(const Mqtt5SubscribeData& data, unsigned int sizePayload, unsigned int sizePropPayload, std::uint8_t*& bufferPacketId)
{
    unsigned int header = 0;
    header |= HEADER_SetCommand(Mqtt5Command::COMMAND_SUBSCRIBE);
    header |= HEADER_SetLow4bits(0x02);
    write1ByteNumber(header);
    writeVarByteNumber(sizePayload);

    // packet identifier
    bufferPacketId = &m_buffer[m_indexBuffer];
    write2ByteNumber(data.packetId);

    // properties
    writeProperties(data.properties, data.metainfo, sizePropPayload);

    // at least one subscription must be available
    assert(!data.subscriptions.empty());

    for (size_t i = 0; i < data.subscriptions.size(); ++i)
    {
        const Mqtt5SubscribeEntry& entry = data.subscriptions[i];
        writeString(entry.topic);
        unsigned int option = 0;
        option |= SO_SetRetainHandling(entry.retainHandling);
        option |= SO_SetRetainAsPublished(entry.retainAsPublished);
        option |= SO_SetNoLocal(entry.noLocal);
        option |= SO_SetQoS(entry.qos);
        write1ByteNumber(option);
    }

    assert(m_indexBuffer == m_sizeBuffer);
}



bool Mqtt5Serialization::deserializeSubAck(Mqtt5SubAckData& data)
{
    if (HEADER_Low4bits(m_buffer[0]) != 0x00)
    {
        return false;
    }

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
        unsigned int reasoncode = 0x80;
        ok = read1ByteNumber(reasoncode);
        if (!ok)
        {
            return false;
        }
        data.reasoncodes.push_back(reasoncode);
    }

    return ok;
}

unsigned int Mqtt5Serialization::sizeSubAck(const Mqtt5SubAckData& data, unsigned int& sizePropPayload)
{
    unsigned int size = 0;

    // packet identifier
    size += 2;
    
    // properties
    size += sizeProperties(data.properties, data.metainfo, sizePropPayload);

    // at least one ack must be available
    assert(!data.reasoncodes.empty());

    size += static_cast<unsigned int>(data.reasoncodes.size());

    return size;
}

void Mqtt5Serialization::serializeSubAck(const Mqtt5SubAckData& data, Mqtt5Command command, unsigned int sizePayload, unsigned int sizePropPayload)
{
    unsigned int header = HEADER_SetCommand(command);
    write1ByteNumber(header);
    writeVarByteNumber(sizePayload);

    // packet identifier
    write2ByteNumber(data.packetId);

    // properties
    writeProperties(data.properties, data.metainfo, sizePropPayload);

    // at least one ack must be available
    assert(!data.reasoncodes.empty());

    for (size_t i = 0; i < data.reasoncodes.size(); ++i)
    {
        unsigned int reasoncode = data.reasoncodes[i];
        write1ByteNumber(reasoncode);
    }

    assert(m_indexBuffer == m_sizeBuffer);
}



bool Mqtt5Serialization::deserializeUnsubscribe(Mqtt5UnsubscribeData& data)
{
    if (HEADER_Low4bits(m_buffer[0]) != 0x02)
    {
        return false;
    }

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


unsigned int Mqtt5Serialization::sizeUnsubscribe(const Mqtt5UnsubscribeData& data, unsigned int& sizePropPayload)
{
    unsigned int size = 0;

    // packet identifier
    size += 2;

    // properties
    size += sizeProperties(data.properties, data.metainfo, sizePropPayload);

    // at least one unsubscription must be available
    assert(!data.topics.empty());

    for (size_t i = 0; i < data.topics.size(); ++i)
    {
        const std::string& topic = data.topics[i];
        size += sizeString(topic);
    }

    return size;
}

void Mqtt5Serialization::serializeUnsubscribe(const Mqtt5UnsubscribeData& data, unsigned int sizePayload, unsigned int sizePropPayload, std::uint8_t*& bufferPacketId)
{
    unsigned int header = 0;
    header |= HEADER_SetCommand(Mqtt5Command::COMMAND_UNSUBSCRIBE);
    header |= HEADER_SetLow4bits(0x02);
    write1ByteNumber(header);
    writeVarByteNumber(sizePayload);

    // packet identifier
    bufferPacketId = &m_buffer[m_indexBuffer];
    write2ByteNumber(data.packetId);

    // properties
    writeProperties(data.properties, data.metainfo, sizePropPayload);

    // at least one unsubscription must be available
    assert(!data.topics.empty());

    for (size_t i = 0; i < data.topics.size(); ++i)
    {
        const std::string& topic = data.topics[i];
        writeString(topic);
    }

    assert(m_indexBuffer == m_sizeBuffer);
}


void Mqtt5Serialization::serializePingReq()
{
    unsigned int header = HEADER_SetCommand(Mqtt5Command::COMMAND_PINGREQ);
    write1ByteNumber(header);
    writeVarByteNumber(0);
    assert(m_indexBuffer == m_sizeBuffer);
}

void Mqtt5Serialization::serializePingResp()
{
    unsigned int header = HEADER_SetCommand(Mqtt5Command::COMMAND_PINGRESP);
    write1ByteNumber(header);
    writeVarByteNumber(0);
    assert(m_indexBuffer == m_sizeBuffer);
}



bool Mqtt5Serialization::deserializeDisconnect(Mqtt5DisconnectData& data)
{
    if (HEADER_Low4bits(m_buffer[0]) != 0x00)
    {
        return false;
    }

    bool ok = true;

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

unsigned int Mqtt5Serialization::sizeDisconnect(const Mqtt5DisconnectData& data, unsigned int& sizePropPayload)
{
    unsigned int size = 0;

    if (data.reasoncode != 0 || !data.properties.empty() || !data.metainfo.empty())
    {
        // reason code
        size += 1;

        // properties
        size += sizeProperties(data.properties, data.metainfo, sizePropPayload);
    }

    return size;
}

void Mqtt5Serialization::serializeDisconnect(const Mqtt5DisconnectData& data, unsigned int sizePayload, unsigned int sizePropPayload)
{
    unsigned int header = HEADER_SetCommand(Mqtt5Command::COMMAND_DISCONNECT);
    write1ByteNumber(header);
    writeVarByteNumber(sizePayload);

    if (data.reasoncode != 0 || !data.properties.empty() || !data.metainfo.empty())
    {
        // reason code
        write1ByteNumber(data.reasoncode);

        // properties
        writeProperties(data.properties, data.metainfo, sizePropPayload);
    }

    assert(m_indexBuffer == m_sizeBuffer);
}

bool Mqtt5Serialization::deserializeAuth(Mqtt5AuthData& data)
{
    if (HEADER_Low4bits(m_buffer[0]) != 0x00)
    {
        return false;
    }

    bool ok = true;

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

unsigned int Mqtt5Serialization::sizeAuth(const Mqtt5AuthData& data, unsigned int& sizePropPayload)
{
    unsigned int size = 0;

    if (data.reasoncode != 0 || !data.properties.empty() || !data.metainfo.empty())
    {
        // reason code
        size += 1;

        // properties
        size += sizeProperties(data.properties, data.metainfo, sizePropPayload);
    }

    return size;
}

void Mqtt5Serialization::serializeAuth(const Mqtt5AuthData& data, unsigned int sizePayload, unsigned int sizePropPayload)
{
    unsigned int header = HEADER_SetCommand(Mqtt5Command::COMMAND_AUTH);
    write1ByteNumber(header);
    writeVarByteNumber(sizePayload);

    if (data.reasoncode != 0 || !data.properties.empty() || !data.metainfo.empty())
    {
        // reason code
        write1ByteNumber(data.reasoncode);

        // properties
        writeProperties(data.properties, data.metainfo, sizePropPayload);
    }

    assert(m_indexBuffer == m_sizeBuffer);
}


bool Mqtt5Serialization::doesFit(int size) const
{
    bool ok = (m_indexBuffer + size <= m_sizeBuffer);
    return ok;
}



bool Mqtt5Serialization::read1ByteNumber(unsigned int& number)
{
    if (doesFit(1))
    {
        unsigned int byte1 = m_buffer[m_indexBuffer];
        ++m_indexBuffer;
        number = byte1;
        return true;
    }
    number = 0;
    return false;
}

void Mqtt5Serialization::write1ByteNumber(unsigned int number)
{
    assert(m_indexBuffer + 1 <= m_sizeBuffer);
    m_buffer[m_indexBuffer] = number & 0xff;
    ++m_indexBuffer;
}

bool Mqtt5Serialization::read2ByteNumber(unsigned int& number)
{
    if (doesFit(2))
    {
        unsigned int byte1 = m_buffer[m_indexBuffer];
        ++m_indexBuffer;
        unsigned int byte2 = m_buffer[m_indexBuffer];
        ++m_indexBuffer;
        number = (byte1 << 8) | byte2;
        return true;
    }
    number = 0;
    return false;
}

void Mqtt5Serialization::write2ByteNumber(unsigned int number)
{
    assert(m_indexBuffer + 2 <= m_sizeBuffer);
    m_buffer[m_indexBuffer] = (number >> 8) & 0xff;
    ++m_indexBuffer;
    m_buffer[m_indexBuffer] = (number >> 0) & 0xff;
    ++m_indexBuffer;
}

bool Mqtt5Serialization::read4ByteNumber(unsigned int& number)
{
    if (doesFit(4))
    {
        unsigned int byte1 = m_buffer[m_indexBuffer];
        ++m_indexBuffer;
        unsigned int byte2 = m_buffer[m_indexBuffer];
        ++m_indexBuffer;
        unsigned int byte3 = m_buffer[m_indexBuffer];
        ++m_indexBuffer;
        unsigned int byte4 = m_buffer[m_indexBuffer];
        ++m_indexBuffer;
        number = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
        return true;
    }
    number = 0;
    return false;
}

void Mqtt5Serialization::write4ByteNumber(unsigned int number)
{
    assert(m_indexBuffer + 4 <= m_sizeBuffer);
    m_buffer[m_indexBuffer] = (number >> 24) & 0xff;
    ++m_indexBuffer;
    m_buffer[m_indexBuffer] = (number >> 16) & 0xff;
    ++m_indexBuffer;
    m_buffer[m_indexBuffer] = (number >> 8) & 0xff;
    ++m_indexBuffer;
    m_buffer[m_indexBuffer] = (number >> 0) & 0xff;
    ++m_indexBuffer;
}

bool Mqtt5Serialization::readVarByteNumber(unsigned int& number)
{
    bool ok = true;
    bool done = false;
    int shift = 0;
    while (!done && ok)
    {
        ok = false;
        if (m_indexBuffer < m_sizeBuffer && shift <= 21)
        {
            ok = true;
            char data = m_buffer[m_indexBuffer];
            ++m_indexBuffer;
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
void Mqtt5Serialization::writeVarByteNumber(unsigned int number)
{
    assert(number <= 268435455);
    do
    {
        assert(m_indexBuffer + 1 <= m_sizeBuffer);
        m_buffer[m_indexBuffer] = number & 0x7f;
        number >>= 7;
        if (number > 0)
        {
            m_buffer[m_indexBuffer] |= 0x80;
        }
        ++m_indexBuffer;
    } while (number > 0);
}

bool Mqtt5Serialization::readString(std::string& str)
{
    unsigned int size = 0;
    bool ok = read2ByteNumber(size);
    if (ok && doesFit(size))
    {
        str.insert(0, reinterpret_cast<char*>(&m_buffer[m_indexBuffer]), size);
        m_indexBuffer += size;
    }
    else
    {
        str.clear();
    }
    return ok;
}
unsigned int Mqtt5Serialization::sizeString(const std::string& str)
{
    assert(str.size() <= 0xffff);
    return static_cast<unsigned int>(2 + str.size());
}
void Mqtt5Serialization::writeString(const std::string& str)
{
    unsigned int size = static_cast<unsigned int>(str.size());
    write2ByteNumber(size);

    assert(m_indexBuffer + size <= m_sizeBuffer);
    memcpy(&m_buffer[m_indexBuffer], str.data(), str.size());
    m_indexBuffer += size;
}

bool Mqtt5Serialization::readStringPair(std::string& key, std::string& value)
{
    bool ok = readString(key);
    if (!ok)
    {
        return false;
    }
    ok = readString(value);
    return ok;
}
unsigned int Mqtt5Serialization::sizeStringPair(const std::string& key, const std::string& value)
{
    assert(key.size() <= 0xffff);
    assert(value.size() <= 0xffff);
    return static_cast<unsigned int>(4 + key.size() + value.size());
}
void Mqtt5Serialization::writeStringPair(const std::string& key, const std::string& value)
{
    writeString(key);
    writeString(value);
}


bool Mqtt5Serialization::readBinary(Bytes& value)
{
    unsigned int size = 0;
    bool ok = read2ByteNumber(size);
    if (ok && doesFit(size))
    {
        value.insert(value.end(), &m_buffer[m_indexBuffer], &m_buffer[m_indexBuffer + size]);
        m_indexBuffer += size;
    }
    else
    {
        value.clear();
    }
    return ok;
}
unsigned int Mqtt5Serialization::sizeBinary(const Bytes& value)
{
    assert(value.size() <= 0xffff);
    return static_cast<unsigned int>(2 + value.size());
}
void Mqtt5Serialization::writeBinary(const Bytes& value)
{
    unsigned int size = static_cast<unsigned int>(value.size());

    write2ByteNumber(size);

    assert(m_indexBuffer + size <= m_sizeBuffer);
    memcpy(&m_buffer[m_indexBuffer], value.data(), size);
    m_indexBuffer += size;
}





bool Mqtt5Serialization::readProperties(std::unordered_map<unsigned int, Variant>& properties, std::unordered_map<std::string, std::string>& metainfo)
{
    if (!doesFit(1))
    {
        return true;
    }

    unsigned int size = 0;
    bool ok = readVarByteNumber(size);
    if (!ok || !doesFit(size))
    {
        return false;
    }

    unsigned int indexEndProperties = m_indexBuffer + size;
    bool done = (size == 0);
    while (!done && ok)
    {
        ok = false;
        if (m_indexBuffer < indexEndProperties)
        {
            ok = true;
            unsigned int id = 0;
            ok = readVarByteNumber(id);
            if (!ok)
            {
                return false;
            }
            Mqtt5PropertyId propertyId(static_cast<Mqtt5PropertyId::Enum>(id));
            if (propertyId == Mqtt5PropertyId::Invalid)
            {
                return false;
            }
            Mqtt5Type type = propertyId.getPropertyType();
            switch (type)
            {
            case Mqtt5Type::TypeNone:
                ok = false;
                break;
            case Mqtt5Type::TypeByte:
                {
                    unsigned int value;
                    ok = read1ByteNumber(value);
                    if (ok)
                    {
                        properties[propertyId] = value;
                    }
                }
                break;
            case Mqtt5Type::TypeTwoByteInteger:
                {
                    unsigned int value;
                    ok = read2ByteNumber(value);
                    if (ok)
                    {
                        properties[propertyId] = value;
                    }
                }
                break;
            case Mqtt5Type::TypeFourByteInteger:
                {
                    unsigned int value;
                    ok = read4ByteNumber(value);
                    if (ok)
                    {
                        properties[propertyId] = value;
                    }
                }
                break;
            case Mqtt5Type::TypeVariableByteInteger:
                {
                    unsigned int value;
                    ok = readVarByteNumber(value);
                    if (ok)
                    {
                        properties[propertyId] = value;
                    }
                }
                break;
            case Mqtt5Type::TypeUTF8String:
                {
                    std::string value;
                    ok = readString(value);
                    if (ok)
                    {
                        properties[propertyId] = value;
                    }
                }
                break;
            case Mqtt5Type::TypeUTF8StringPair:
                {
                    std::string key;
                    std::string value;
                    ok = readStringPair(key, value);
                    if (ok)
                    {
                        if (propertyId == Mqtt5PropertyId::UserProperty)
                        {
                            metainfo[std::move(key)] = std::move(value);
                        }
                    }
                }
                break;
            case Mqtt5Type::TypeBinaryData:
                {
                    Bytes value;
                    ok = readBinary(value);
                    if (ok)
                    {
                        properties[propertyId] = value;
                    }
                }
                break;
            case Mqtt5Type::TypeArrayVariableByteInteger:
            {
                unsigned int value;
                ok = readVarByteNumber(value);
                if (ok)
                {
                    Variant& property = properties[propertyId];
                    std::vector<std::uint32_t>* arr = property;
                    if (arr == nullptr)
                    {
                        property = std::vector<std::uint32_t>();
                    }
                    arr = property;
                    assert(arr != nullptr);
                    arr->push_back(value);
                }
            }
            break;
            default:
                assert(false);
                ok = false;
                break;
            }
            if (m_indexBuffer == indexEndProperties)
            {
                done = true;
            }
        }
    }

    return ok;
}

unsigned int Mqtt5Serialization::sizeProperties(const std::unordered_map<unsigned int, Variant>& properties, const std::unordered_map<std::string, std::string>& metainfo, unsigned int& sizePropertyPayload)
{
    sizePropertyPayload = 0;

    for (auto it = properties.begin(); it != properties.end(); ++it)
    {
        unsigned int id = it->first;
        const Variant& value = it->second;
        Mqtt5PropertyId propertyId(static_cast<Mqtt5PropertyId::Enum>(id));
        assert(propertyId != Mqtt5PropertyId::Invalid);
        Mqtt5Type type = propertyId.getPropertyType();
        switch (type)
        {
        case Mqtt5Type::TypeNone:
            assert(false);
            break;
        case Mqtt5Type::TypeByte:
            sizePropertyPayload += sizeVarByteNumber(id);
            sizePropertyPayload += 1;
            break;
        case Mqtt5Type::TypeTwoByteInteger:
            sizePropertyPayload += sizeVarByteNumber(id);
            sizePropertyPayload += 2;
            break;
        case Mqtt5Type::TypeFourByteInteger:
            sizePropertyPayload += sizeVarByteNumber(id);
            sizePropertyPayload += 4;
            break;
        case Mqtt5Type::TypeVariableByteInteger:
            sizePropertyPayload += sizeVarByteNumber(id);
            sizePropertyPayload += sizeVarByteNumber(value);
            break;
        case Mqtt5Type::TypeUTF8String:
            sizePropertyPayload += sizeVarByteNumber(id);
            sizePropertyPayload += sizeString(value);
            break;
        case Mqtt5Type::TypeUTF8StringPair:
            assert(false);
            break;
        case Mqtt5Type::TypeBinaryData:
            sizePropertyPayload += sizeVarByteNumber(id);
            sizePropertyPayload += sizeBinary(value);
            break;
        case Mqtt5Type::TypeArrayVariableByteInteger:
            {
                const std::vector<std::uint32_t>* arr = value;
                if (arr != nullptr)
                {
                    for (size_t i = 0; i < arr->size(); ++i)
                    {
                        sizePropertyPayload += sizeVarByteNumber(id);
                        sizePropertyPayload += sizeVarByteNumber(arr->at(i));
                    }
                }
            }
            break;
        default:
            assert(false);
            break;
        }
    }

    for (auto it = metainfo.begin(); it != metainfo.end(); ++it)
    {
        const std::string& key = it->first;
        const std::string& value = it->second;
        sizePropertyPayload += sizeVarByteNumber(Mqtt5PropertyId::UserProperty);
        sizePropertyPayload += sizeStringPair(key, value);
    }
    
    return (sizeVarByteNumber(sizePropertyPayload) + sizePropertyPayload);
}

void Mqtt5Serialization::writeProperties(const std::unordered_map<unsigned int, Variant>& properties, const std::unordered_map<std::string, std::string>& metainfo, unsigned int sizePropertyPayload)
{
    writeVarByteNumber(sizePropertyPayload);

    for (auto it = properties.begin(); it != properties.end(); ++it)
    {
        unsigned int id = it->first;
        const Variant& value = it->second;
        Mqtt5PropertyId propertyId(static_cast<Mqtt5PropertyId::Enum>(id));
        assert(propertyId != Mqtt5PropertyId::Invalid);
        Mqtt5Type type = propertyId.getPropertyType();
        switch (type)
        {
        case Mqtt5Type::TypeNone:
            assert(false);
            break;
        case Mqtt5Type::TypeByte:
            writeVarByteNumber(id);
            write1ByteNumber(value);
            break;
        case Mqtt5Type::TypeTwoByteInteger:
            writeVarByteNumber(id);
            write2ByteNumber(value);
            break;
        case Mqtt5Type::TypeFourByteInteger:
            writeVarByteNumber(id);
            write4ByteNumber(value);
            break;
        case Mqtt5Type::TypeVariableByteInteger:
            writeVarByteNumber(id);
            writeVarByteNumber(value);
            break;
        case Mqtt5Type::TypeUTF8String:
            writeVarByteNumber(id);
            writeString(value);
            break;
        case Mqtt5Type::TypeUTF8StringPair:
            assert(false);
            break;
        case Mqtt5Type::TypeBinaryData:
            writeVarByteNumber(id);
            writeBinary(value);
            break;
        case Mqtt5Type::TypeArrayVariableByteInteger:
            {
                const std::vector<std::uint32_t>* arr = value;
                if (arr != nullptr)
                {
                    for (size_t i = 0; i < arr->size(); ++i)
                    {
                        writeVarByteNumber(id);
                        writeVarByteNumber(arr->at(i));
                    }
                }
            }
            break;
        default:
            assert(false);
            break;
        }
    }

    for (auto it = metainfo.begin(); it != metainfo.end(); ++it)
    {
        const std::string& key = it->first;
        const std::string& value = it->second;
        writeVarByteNumber(Mqtt5PropertyId::UserProperty);
        writeStringPair(key, value);
    }
}



}   // namespace finalmq
