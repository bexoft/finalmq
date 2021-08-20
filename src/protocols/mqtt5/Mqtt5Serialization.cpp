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




#define HEADER_Command(header)      (((header) & 0xf0) >> 4)
#define HEADER_Dup(header)          (((header) & 0x08) >> 3)
#define HEADER_QoS(header)          (((header) & 0x06) >> 1)
#define HEADER_Retain(header)       (((header) & 0x01) >> 0)

#define CF_UserName(flags)       (((flags) >> 7) & 0x01) 
#define CF_Password(flags)       (((flags) >> 6) & 0x01) 
#define CF_WillRetain(flags)     (((flags) >> 5) & 0x01) 
#define CF_WillQoS(flags)        (((flags) >> 3) & 0x03) 
#define CF_WillFlag(flags)       (((flags) >> 2) & 0x01) 
#define CF_CleanStart(flags)     (((flags) >> 1) & 0x01) 

#define CF_SetUserName(value)    ((value) & 0x01) << 7)
#define CF_SetPassword(flags)    ((value) & 0x01) << 6) 
#define CF_SetWillRetain(flags)  ((value) & 0x01) << 5)
#define CF_SetWillQoS(flags)     ((value) & 0x03) << 3)
#define CF_SetWillFlag(flags)    ((value) & 0x01) << 2)
#define CF_SetCleanStart(flags)  ((value) & 0x01) << 1)


Mqtt5Serialization::Mqtt5Serialization(int remainingSize, int indexRead, unsigned char header, char* buffer)
    : m_remainingSize(remainingSize)
    , m_indexRead(indexRead)
    , m_header(header)
    , m_buffer(buffer)
{

}


int Mqtt5Serialization::getReadIndex() const
{
    return m_indexRead;
}


bool Mqtt5Serialization::deserializeConnect(Mqtt5ConnectData& data)
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


bool Mqtt5Serialization::serializeConnect(Mqtt5ConnectData& data)
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



#define CAF_SessionPresent(flags)       (((flags) & 0x01) >> 0)
#define CAF_SetSessionPresent(flags)    ((value) & 0x01) << 1)

bool Mqtt5Serialization::deserializeConnAck(Mqtt5ConnAckData& data)
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



bool Mqtt5Serialization::deserializePublish(Mqtt5PublishData& data)
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


bool Mqtt5Serialization::deserializePubAck(Mqtt5PubAckData& data)
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



#define SO_RetainHandling(flags)        (((flags) & 0x30) >> 4)
#define SO_RetainAsPublished(flags)     (((flags) & 0x08) >> 3)
#define SO_NoLocal(flags)               (((flags) & 0x04) >> 2)
#define SO_QoS(flags)                   (((flags) & 0x03) >> 0)


bool Mqtt5Serialization::deserializeSubscribe(Mqtt5SubscribeData& data)
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


bool Mqtt5Serialization::deserializeSubAck(Mqtt5SubAckData& data)
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




bool Mqtt5Serialization::deserializeUnsubscribe(Mqtt5UnsubscribeData& data)
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






bool Mqtt5Serialization::deserializeDisconnect(Mqtt5DisconnectData& data)
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





bool Mqtt5Serialization::deserializeAuth(Mqtt5AuthData& data)
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



bool Mqtt5Serialization::doesFit(int size) const
{
    bool ok = (m_indexRead + size <= m_remainingSize);
    return ok;
}



bool Mqtt5Serialization::read1ByteNumber(unsigned int& number)
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

bool Mqtt5Serialization::read2ByteNumber(unsigned int& number)
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

bool Mqtt5Serialization::read4ByteNumber(unsigned int& number)
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

bool Mqtt5Serialization::readVarByteNumber(unsigned int& number)
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
unsigned int Mqtt5Serialization::sizeVarByteNumber(unsigned int number)
{
    assert(number <= 268435455);
    return (1 + (number > 127) + (number > 16383) + (number > 2097151));
}

bool Mqtt5Serialization::readString(std::string& str)
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
unsigned int Mqtt5Serialization::sizeString(const std::string& str)
{
    assert(str.size() <= 0xffff);
    return static_cast<unsigned int>(2 + str.size());
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
unsigned int Mqtt5Serialization::sizeStringPair(std::string& key, std::string& value)
{
    assert(key.size() <= 0xffff);
    assert(value.size() <= 0xffff);
    return static_cast<unsigned int>(4 + key.size() + value.size());
}


bool Mqtt5Serialization::readBinary(Bytes& value)
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
unsigned int Mqtt5Serialization::sizeBinary(const Bytes& value)
{
    assert(value.size() <= 0xffff);
    return static_cast<unsigned int>(2 + value.size());
}





bool Mqtt5Serialization::readProperties(std::unordered_map<unsigned int, Variant>& properties, std::unordered_map<std::string, std::string>& metainfo)
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

unsigned int Mqtt5Serialization::sizeProperties(const std::unordered_map<unsigned int, Variant>& properties, const std::unordered_map<std::string, std::string>& metainfo, unsigned int& sizePropertyPayload)
{
    sizePropertyPayload = 0;

    for (auto it = properties.begin(); it != properties.end(); ++it)
    {
        unsigned int id = it->first;
        const Variant& value = it->second;
        sizePropertyPayload += sizeVarByteNumber(id);
        Mqtt5PropertyId propertyId(static_cast<Mqtt5PropertyId::Enum>(id));
        assert(propertyId != Mqtt5PropertyId::Invalid);
        Mqtt5Type type = propertyId.getPropertyType();
        switch (type)
        {
        case Mqtt5Type::TypeNone:
            assert(false);
            break;
        case Mqtt5Type::TypeByte:
            sizePropertyPayload += 1;
            break;
        case Mqtt5Type::TypeTwoByteInteger:
            sizePropertyPayload += 2;
            break;
        case Mqtt5Type::TypeFourByteInteger:
            sizePropertyPayload += 4;
            break;
        case Mqtt5Type::TypeVariableByteInteger:
            sizePropertyPayload += sizeVarByteNumber(value);
            break;
        case Mqtt5Type::TypeUTF8String:
            sizePropertyPayload += sizeString(value);
            break;
        case Mqtt5Type::TypeUTF8StringPair:
            assert(false);
            break;
        case Mqtt5Type::TypeBinaryData:
            sizePropertyPayload += sizeBinary(value);
            break;
        default:
            assert(false);
            break;
        }
    }

    //!!! todo: for (metainfo)
    
    return (sizeVarByteNumber(sizePropertyPayload) + sizePropertyPayload);
}


}   // namespace finalmq
