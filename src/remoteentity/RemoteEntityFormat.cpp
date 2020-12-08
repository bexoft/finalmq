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

#include "remoteentity/RemoteEntityFormat.h"
#include "remoteentity/entitydata.fmq.h"

#include "serializeproto/SerializerProto.h"
#include "serializejson/SerializerJson.h"
#include "serializestruct/ParserStruct.h"

#include "serializeproto/ParserProto.h"
#include "serializejson/ParserJson.h"
#include "serializestruct/SerializerStruct.h"
#include "serializestruct/StructFactoryRegistry.h"


using finalmq::remoteentity::MsgMode;
using finalmq::remoteentity::Status;
using finalmq::remoteentity::Header;


namespace finalmq {


void RemoteEntityFormat::serializeProto(IMessage& message, const Header& header)
{
    char* bufferSizeHeader = message.addSendPayload(2048);
    message.downsizeLastSendPayload(4);

    SerializerProto serializerHeader(message);
    ParserStruct parserHeader(serializerHeader, header);
    parserHeader.parseStruct();
    ssize_t sizeHeader = message.getTotalSendPayloadSize() - 4;
    assert(sizeHeader >= 0);

    *bufferSizeHeader = static_cast<unsigned char>(sizeHeader);
    ++bufferSizeHeader;
    *bufferSizeHeader = static_cast<unsigned char>(sizeHeader >> 8);
    ++bufferSizeHeader;
    *bufferSizeHeader = static_cast<unsigned char>(sizeHeader >> 16);
    ++bufferSizeHeader;
    *bufferSizeHeader = static_cast<unsigned char>(sizeHeader >> 24);
}



void RemoteEntityFormat::serializeProto(IMessage& message, const Header& header, const StructBase& structBase)
{
    serializeProto(message, header);

    SerializerProto serializerData(message);
    ParserStruct parserData(serializerData, structBase);
    parserData.parseStruct();
}


void RemoteEntityFormat::serializeJson(IMessage& message, const Header& header)
{
    SerializerJson serializerHeader(message, 2048);
    ParserStruct parserHeader(serializerHeader, header);
    parserHeader.parseStruct();

    // add end of header
    message.addSendPayload("\t", 1);
}



void RemoteEntityFormat::serializeJson(IMessage& message, const Header& header, const StructBase& structBase)
{
    serializeJson(message, header);

    SerializerJson serializerData(message);
    ParserStruct parserData(serializerData, structBase);
    parserData.parseStruct();
}



void RemoteEntityFormat::serialize(IMessage& message, int contentType, const remoteentity::Header& header)
{
    switch (contentType)
    {
    case CONTENTTYPE_PROTO:
        serializeProto(message, header);
        break;
    case CONTENTTYPE_JSON:
        serializeJson(message, header);
        break;
    default:
        assert(false);
        break;
    }
}


void RemoteEntityFormat::serialize(IMessage& message, int contentType, const remoteentity::Header& header, const StructBase& structBase)
{
    switch (contentType)
    {
    case CONTENTTYPE_PROTO:
        serializeProto(message, header, structBase);
        break;
    case CONTENTTYPE_JSON:
        serializeJson(message, header, structBase);
        break;
    default:
        assert(false);
        break;
    }
}


inline static bool shallSend(const remoteentity::Header& header)
{
    if ((header.mode != MsgMode::MSG_REPLY) ||
        (header.status == Status::STATUS_ENTITY_NOT_FOUND || header.corrid != CORRELATIONID_NONE))
    {
        return true;
    }
    return false;
}




bool RemoteEntityFormat::send(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBase& structBase)
{
    bool ok = true;
    if (shallSend(header))
    {
        assert(session);
        IMessagePtr message = session->createMessage();
        assert(message);
        serialize(*message, session->getContentType(), header, structBase);
        ok = session->sendMessage(message);
    }
    return ok;
}



bool RemoteEntityFormat::send(const IProtocolSessionPtr& session, const remoteentity::Header& header)
{
    bool ok = true;
    if (shallSend(header))
    {
        assert(session);
        IMessagePtr message = session->createMessage();
        assert(message);
        serialize(*message, session->getContentType(), header);
        ok = session->sendMessage(message);
    }
    return ok;
}



std::shared_ptr<StructBase> RemoteEntityFormat::parseMessageProto(const BufferRef& bufferRef, Header& header, bool& syntaxError)
{
    syntaxError = false;
    const char* buffer = bufferRef.first;
    ssize_t sizeBuffer = bufferRef.second;
    if (sizeBuffer < 4)
    {
        streamError << "buffer size too small: " << sizeBuffer;
        return nullptr;
    }

    ssize_t sizePayload = sizeBuffer - 4;
    ssize_t sizeHeader = 0;
    if (sizeBuffer >= 4)
    {
        sizeHeader = (unsigned char)*buffer;
        ++buffer;
        sizeHeader |= ((unsigned char)*buffer) << 8;
        ++buffer;
        sizeHeader |= ((unsigned char)*buffer) << 16;
        ++buffer;
        sizeHeader |= ((unsigned char)*buffer) << 24;
        ++buffer;
    }
    bool ok = false;
    std::shared_ptr<StructBase> data;

    if (sizeHeader <= sizePayload)
    {
        SerializerStruct serializerHeader(header);
        ParserProto parserHeader(serializerHeader, buffer, sizeHeader);
        ok = parserHeader.parseStruct(Header::structInfo().getTypeName());
    }

    if (ok && !header.type.empty())
    {
        buffer += sizeHeader;
        data = StructFactoryRegistry::instance().createStruct(header.type);
    }

    if (data)
    {
        ssize_t sizeData = sizePayload - sizeHeader;
        assert(sizeData >= 0);
        SerializerStruct serializerData(*data);
        ParserProto parserData(serializerData, buffer, sizeData);
        ok = parserData.parseStruct(header.type);
        if (!ok)
        {
            syntaxError = true;
            data = nullptr;
        }
    }

    return data;
}

static ssize_t findFirst(const char* buffer, ssize_t size, char c)
{
    for (ssize_t i = 0; i < size; ++i)
    {
        if (buffer[i] == c)
        {
            return i;
        }
    }
    return -1;
}

static ssize_t findLast(const char* buffer, ssize_t size, char c)
{
    for (ssize_t i = size - 1; i >= 0; --i)
    {
        if (buffer[i] == c)
        {
            return i;
        }
    }
    return -1;
}


std::shared_ptr<StructBase> RemoteEntityFormat::parseMessageJson(const BufferRef& bufferRef, Header& header, bool& syntaxError)
{
    syntaxError = false;
    const char* buffer = bufferRef.first;
    ssize_t sizeBuffer = bufferRef.second;

    if (sizeBuffer == 0)
    {
        return nullptr;
    }

    const char* endHeader = nullptr;
    if (buffer[0] == '/')
    {
        // 012345678901234567890123456789
        // /MyServer/test.TestRequest#1{}
        ssize_t ixEndHeader = findFirst(buffer, sizeBuffer, '{');   //28
        if (ixEndHeader == -1)
        {
            ixEndHeader = sizeBuffer;
        }
        endHeader = &buffer[ixEndHeader];
        ssize_t ixStartCommand = findLast(buffer, ixEndHeader, '/');    //9
        assert(ixStartCommand >= 0);
        if (ixStartCommand > 1)
        {
            header.destname = {&buffer[1], &buffer[ixStartCommand]};
        }
        ssize_t ixCorrelationId = findLast(buffer, ixEndHeader, '#');   //26
        if (ixCorrelationId != -1)
        {
            header.corrid = strtoll(&buffer[ixCorrelationId+1], nullptr, 10);
        }
        else
        {
            ixCorrelationId = ixEndHeader;
        }
        header.type = {&buffer[ixStartCommand+1], &buffer[ixCorrelationId]};

        header.mode = MsgMode::MSG_REQUEST;
    }
    else
    {
        SerializerStruct serializerHeader(header);
        ParserJson parserHeader(serializerHeader, buffer, sizeBuffer);
        endHeader = parserHeader.parseStruct(header.getStructInfo().getTypeName());
    }

    std::shared_ptr<StructBase> data;

    if (endHeader && !header.type.empty())
    {
        data = StructFactoryRegistry::instance().createStruct(header.type);
    }

    if (data)
    {
        assert(endHeader);
        ssize_t sizeHeader = endHeader - buffer;
        assert(sizeHeader >= 0);
        buffer += sizeHeader;
        ssize_t sizeData = sizeBuffer - sizeHeader;
        assert(sizeData >= 0);
        if (sizeData > 0)
        {
            SerializerStruct serializerData(*data);
            ParserJson parserData(serializerData, buffer, sizeData);
            const char* endData = parserData.parseStruct(header.type);
            if (!endData)
            {
                syntaxError = true;
                data = nullptr;
            }
        }
    }

    return data;
}



std::shared_ptr<StructBase> RemoteEntityFormat::parseMessage(const IMessage& message, int contentType, Header& header, bool& syntaxError)
{
    syntaxError = false;
    BufferRef bufferRef = message.getReceivePayload();
    std::shared_ptr<StructBase> structBase;

    switch (contentType)
    {
    case CONTENTTYPE_PROTO:
        structBase = parseMessageProto(bufferRef, header, syntaxError);
        break;
    case CONTENTTYPE_JSON:
        structBase = parseMessageJson(bufferRef, header, syntaxError);
        break;
    default:
        assert(false);
        break;
    }

    return structBase;
}



}   // namespace finalmq
