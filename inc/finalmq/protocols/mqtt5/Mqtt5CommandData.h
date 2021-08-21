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


}   // namespace finalmq
