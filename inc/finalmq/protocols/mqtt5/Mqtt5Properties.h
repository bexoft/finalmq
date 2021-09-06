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

#include "finalmq/serializestruct/StructBase.h"


namespace finalmq {


enum class Mqtt5Type
{
    TypeNone = 0,
    TypeByte = 1,
    TypeTwoByteInteger = 2,
    TypeFourByteInteger = 3,
    TypeVariableByteInteger = 4,
    TypeUTF8String = 5,
    TypeUTF8StringPair = 6,
    TypeBinaryData = 7,

    TypeArrayVariableByteInteger = 16 + TypeVariableByteInteger,
};


class Mqtt5PropertyId
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
        SubscriptionIdentifiersAvailable = 41,   // Byte, CONNACK
        SharedSubscriptionAvailable = 42,   // Byte, CONNACK
    };

    Mqtt5PropertyId();
    Mqtt5PropertyId(Enum en);
    operator const Enum& () const;
    operator Enum& ();
    const Mqtt5PropertyId& operator =(Enum en);
    const std::string& toString() const;
    void fromString(const std::string& name);

    Mqtt5Type getPropertyType() const;

private:
    Enum m_value = Invalid;
    static const finalmq::EnumInfo _enumInfo;
};


}   // namespace finalmq
