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


#include "finalmq/protocols/mqtt5/Mqtt5Properties.h"

namespace finalmq {


//////////////////////////////////////////////////////////
// Properties



Mqtt5PropertyId::Mqtt5PropertyId()
{
}
Mqtt5PropertyId::Mqtt5PropertyId(Enum en)
    : m_value(en)
{
}
Mqtt5PropertyId::operator const Enum& () const
{
    return m_value;
}
Mqtt5PropertyId::operator Enum& ()
{
    return m_value;
}
const Mqtt5PropertyId& Mqtt5PropertyId::operator =(Enum en)
{
    m_value = en;
    return *this;
}
const std::string& Mqtt5PropertyId::toString() const
{
    return _enumInfo.getMetaEnum().getNameByValue(m_value);
}
void Mqtt5PropertyId::fromString(const std::string& name)
{
    m_value = static_cast<Enum>(_enumInfo.getMetaEnum().getValueByName(name));
}
const finalmq::EnumInfo Mqtt5PropertyId::_enumInfo = {
    "private.Mqtt5PropertyId", "", {
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


static const Mqtt5Type PropertyType[] = {
    Mqtt5Type::TypeNone,                   //  0
    Mqtt5Type::TypeByte,                   //  1 PayloadFormatIndicator
    Mqtt5Type::TypeFourByteInteger,        //  2 MessageExpiryInterval
    Mqtt5Type::TypeUTF8String,             //  3 ContentType
    Mqtt5Type::TypeNone,                   //  4 
    Mqtt5Type::TypeNone,                   //  5
    Mqtt5Type::TypeNone,                   //  6
    Mqtt5Type::TypeNone,                   //  7
    Mqtt5Type::TypeUTF8String,             //  8 ResponseTopic
    Mqtt5Type::TypeBinaryData,             //  9 CorrelationData
    Mqtt5Type::TypeNone,                   // 10
    Mqtt5Type::TypeVariableByteInteger,    // 11 SubscriptionIdentifier
    Mqtt5Type::TypeNone,                   // 12
    Mqtt5Type::TypeNone,                   // 13
    Mqtt5Type::TypeNone,                   // 14
    Mqtt5Type::TypeNone,                   // 15
    Mqtt5Type::TypeNone,                   // 16
    Mqtt5Type::TypeFourByteInteger,        // 17 SessionExpiryInterval
    Mqtt5Type::TypeUTF8String,             // 18 AssignedClientIdentifier
    Mqtt5Type::TypeTwoByteInteger,         // 19 ServerKeepAlive
    Mqtt5Type::TypeNone,                   // 20
    Mqtt5Type::TypeUTF8String,             // 21 AuthenticationMethod
    Mqtt5Type::TypeBinaryData,             // 22 AuthenticationData
    Mqtt5Type::TypeByte,                   // 23 RequestProblemInformation
    Mqtt5Type::TypeFourByteInteger,        // 24 WillDelayInterval
    Mqtt5Type::TypeByte,                   // 25 RequestResponseInformation
    Mqtt5Type::TypeUTF8String,             // 26 ResponseInformation
    Mqtt5Type::TypeNone,                   // 27
    Mqtt5Type::TypeUTF8String,             // 28 ServerReference
    Mqtt5Type::TypeNone,                   // 29
    Mqtt5Type::TypeNone,                   // 30
    Mqtt5Type::TypeUTF8String,             // 31 ReasonString
    Mqtt5Type::TypeNone,                   // 32
    Mqtt5Type::TypeTwoByteInteger,         // 33 ReceiveMaximum
    Mqtt5Type::TypeTwoByteInteger,         // 34 TopicAliasMaximum
    Mqtt5Type::TypeTwoByteInteger,         // 35 TopicAlias
    Mqtt5Type::TypeByte,                   // 36 MaximumQoS
    Mqtt5Type::TypeByte,                   // 37 RetainAvailable
    Mqtt5Type::TypeUTF8StringPair,         // 38 UserProperty
    Mqtt5Type::TypeFourByteInteger,        // 39 MaximumPacketSize
    Mqtt5Type::TypeByte,                   // 40 WildcardSubscriptionAvailable
    Mqtt5Type::TypeByte,                   // 41 SubscriptionIdentifierAvailable
    Mqtt5Type::TypeByte,                   // 42 SharedSubscriptionAvailable
};

Mqtt5Type Mqtt5PropertyId::getPropertyType() const
{
    if (m_value > Mqtt5PropertyId::Invalid && m_value <= Mqtt5PropertyId::SharedSubscriptionAvailable)
    {
        return PropertyType[static_cast<int>(m_value)];
    }
    return Mqtt5Type::TypeNone;
}

}   // namespace finalmq
