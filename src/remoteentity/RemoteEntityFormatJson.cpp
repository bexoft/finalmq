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

#include "finalmq/remoteentity/RemoteEntityFormatJson.h"

#include "finalmq/serializejson/SerializerJson.h"
#include "finalmq/serializestruct/ParserStruct.h"
#include "finalmq/serializejson/ParserJson.h"
#include "finalmq/serializestruct/SerializerStruct.h"
#include "finalmq/serializestruct/StructFactoryRegistry.h"

#include "finalmq/remoteentity/entitydata.fmq.h"

//#include "finalmq/helpers/ModulenameFinalmq.h"


using finalmq::remoteentity::MsgMode;
using finalmq::remoteentity::Header;


namespace finalmq {

struct RegisterFormatJson
{
    RegisterFormatJson()
    {
        RemoteEntityFormatRegistry::instance().registerFormat(RemoteEntityFormatJson::CONTENT_TYPE, std::make_shared<RemoteEntityFormatJson>());
    }
} g_registerFormatJson;



#define JSONBLOCKSIZE   2048

void RemoteEntityFormatJson::serialize(IMessage& message, const Header& header, const StructBase* structBase)
{
    message.addSendPayload("[", 1, JSONBLOCKSIZE);

    SerializerJson serializerHeader(message);
    ParserStruct parserHeader(serializerHeader, header);
    parserHeader.parseStruct();


    // add end of header
    if (structBase)
    {
        // delimiter between header and payload
        message.addSendPayload(",\t", 2, JSONBLOCKSIZE);

        // payload
        const std::string* typeName = &structBase->getStructInfo().getTypeName();
        if (*typeName == remoteentity::GenericMessage::structInfo().getTypeName())
        {
            const remoteentity::GenericMessage& genericMessage = static_cast<const remoteentity::GenericMessage&>(*structBase);
            assert(genericMessage.contenttype == CONTENT_TYPE);
            char* payload = message.addSendPayload(genericMessage.data.size() + 1);
            memcpy(payload, genericMessage.data.data(), genericMessage.data.size());
            payload[genericMessage.data.size()] = ']';
        }
        else
        {
            SerializerJson serializerData(message);
            ParserStruct parserData(serializerData, *structBase);
            parserData.parseStruct();
            message.addSendPayload("]", 1);
        }
    }
    else
    {
        message.addSendPayload(",\t{}]", 5);
    }
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


std::shared_ptr<StructBase> RemoteEntityFormatJson::parse(const BufferRef& bufferRef, Header& header, bool& syntaxError)
{
    syntaxError = false;
    const char* buffer = bufferRef.first;
    ssize_t sizeBuffer = bufferRef.second;

    if (sizeBuffer == 0)
    {
        return nullptr;
    }

    if (buffer[0] == '[')
    {
        ++buffer;
        --sizeBuffer;
    }
    if (buffer[sizeBuffer - 1] == ']')
    {
        --sizeBuffer;
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
        // skip comma
        ++endHeader;
    }

    std::shared_ptr<StructBase> data;

    if (endHeader && !header.type.empty())
    {
        data = StructFactoryRegistry::instance().createStruct(header.type);

        assert(endHeader);
        ssize_t sizeHeader = endHeader - buffer;
        assert(sizeHeader >= 0);
        buffer += sizeHeader;
        ssize_t sizeData = sizeBuffer - sizeHeader;
        assert(sizeData >= 0);

        if (data)
        {
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
        else
        {
            std::shared_ptr<remoteentity::GenericMessage> genericMessage = std::make_shared<remoteentity::GenericMessage>();
            genericMessage->type = header.type;
            genericMessage->contenttype = CONTENT_TYPE;
            genericMessage->data.resize(sizeData);
            memcpy(genericMessage->data.data(), buffer, sizeData);
            data = genericMessage;
        }
    }

    return data;
}



}   // namespace finalmq
