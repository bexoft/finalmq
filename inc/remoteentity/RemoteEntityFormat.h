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

#include "protocolconnection/ProtocolSessionContainer.h"


//#include <memory>
//#include <vector>
//#include <unordered_map>
//#include <mutex>
//#include <atomic>


namespace finalmq {

class StructBase;
//class IProtocolSession;
typedef std::shared_ptr<IProtocolSession> IProtocolSessionPtr;


namespace remoteentity {
    class Header;
}


enum RemoteEntityContentType
{
    CONTENTTYPE_PROTO = 1,
    CONTENTTYPE_JSON = 2
};



using EntityId = std::uint32_t;
static constexpr EntityId ENTITYID_INVALID = 0;  // should be 0, so that it matches with the deserialized default value.

using CorrelationId = std::uint64_t;
static constexpr CorrelationId CORRELATIONID_NONE = 0;



class SYMBOLEXP RemoteEntityFormat
{
public:
    static std::shared_ptr<StructBase> parseMessageProto(const BufferRef& bufferRef, remoteentity::Header& header, bool& syntaxError);
    static std::shared_ptr<StructBase> parseMessageJson(const BufferRef& bufferRef, remoteentity::Header& header, bool& syntaxError);
    static std::shared_ptr<StructBase> parseMessage(const IMessage& message, int contentType, remoteentity::Header& header, bool& syntaxError);

    static void serializeProto(IMessage& message, const remoteentity::Header& header);
    static void serializeProto(IMessage& message, const remoteentity::Header& header, const StructBase& structBase);
    static void serializeJson(IMessage& message, const remoteentity::Header& header);
    static void serializeJson(IMessage& message, const remoteentity::Header& header, const StructBase& structBase);
    static void serialize(IMessage& message, int contentType, const remoteentity::Header& header);
    static void serialize(IMessage& message, int contentType, const remoteentity::Header& header, const StructBase& structBase);

    static bool send(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBase& structBase);
    static bool send(const IProtocolSessionPtr& session, const remoteentity::Header& header);
};




}   // namespace finalmq