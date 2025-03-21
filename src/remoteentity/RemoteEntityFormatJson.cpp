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
//SOFTWARE.

#include "finalmq/remoteentity/RemoteEntityFormatJson.h"

#include "finalmq/serializejson/SerializerJson.h"
#include "finalmq/serializeproto/SerializerProto.h"
#include "finalmq/serializestruct/ParserStruct.h"
#include "finalmq/serializejson/ParserJson.h"
#include "finalmq/serializeproto/ParserProto.h"
#include "finalmq/serializestruct/SerializerStruct.h"
#include "finalmq/serializestruct/StructFactoryRegistry.h"
#include "finalmq/protocolsession/ProtocolMessage.h"
#include "finalmq/helpers/ZeroCopyBuffer.h"

#include "finalmq/remoteentity/entitydata.fmq.h"

//#include "finalmq/helpers/ModulenameFinalmq.h"


using finalmq::MsgMode;
using finalmq::Header;


namespace finalmq {

const int RemoteEntityFormatJson::CONTENT_TYPE = 2;
const std::string RemoteEntityFormatJson::CONTENT_TYPE_NAME = "json";

const std::string RemoteEntityFormatJson::PROPERTY_SERIALIZE_ENUM_AS_STRING = "enumAsSt";
const std::string RemoteEntityFormatJson::PROPERTY_SERIALIZE_SKIP_DEFAULT_VALUES = "skipDefVal";



struct RegisterFormatJson
{
    RegisterFormatJson()
    {
        RemoteEntityFormatRegistry::instance().registerFormat(RemoteEntityFormatJson::CONTENT_TYPE_NAME, RemoteEntityFormatJson::CONTENT_TYPE, std::make_shared<RemoteEntityFormatJson>());
    }
} g_registerFormatJson;



#define JSONBLOCKSIZE   512

void RemoteEntityFormatJson::serialize(const IProtocolSessionPtr& session, IMessage& message, const Header& header, const StructBase* structBase)
{
    message.addSendPayload("[", 1, JSONBLOCKSIZE);

    SerializerJson serializerHeader(message, JSONBLOCKSIZE);
    ParserStruct parserHeader(serializerHeader, header);
    parserHeader.parseStruct();


    // add end of header
    if (structBase)
    {
        // delimiter between header and payload
        message.addSendPayload(",\t", 2, JSONBLOCKSIZE);

        serializeData(session, message, structBase);

        message.addSendPayload("]\t\t", 3);
    }
    else
    {
        message.addSendPayload(",\t{}]\t\t", 7);
    }
}


void RemoteEntityFormatJson::serializeData(const IProtocolSessionPtr& session, IMessage& message, const StructBase* structBase)
{
    if (structBase)
    {
        bool enumAsString = true;
        bool skipDefaultValues = false;
        const Variant& formatData = session->getFormatData();
        if (formatData.getType() != VARTYPE_NONE)
        {
            const bool* propEnumAsString = formatData.getData<bool>(PROPERTY_SERIALIZE_ENUM_AS_STRING);
            const bool* propSkipDefaultValues = formatData.getData<bool>(PROPERTY_SERIALIZE_SKIP_DEFAULT_VALUES);
            if (propEnumAsString != nullptr)
            {
                enumAsString = *propEnumAsString;
            }
            if (propSkipDefaultValues != nullptr)
            {
                skipDefaultValues = *propSkipDefaultValues;
            }
        }

        // payload
        if (structBase->getRawContentType() == CONTENT_TYPE)
        {
            const std::string* rawData = structBase->getRawData();
            assert(rawData);
            char* payload = message.addSendPayload(rawData->size());
            memcpy(payload, rawData->data(), rawData->size());
        }
        else if (structBase->getStructInfo().getTypeName() == GeneralMessage::structInfo().getTypeName())
        {
            const GeneralMessage* generalMessage = static_cast<const GeneralMessage*>(structBase);
            if (!generalMessage->type.empty())
            {
                SerializerJson serializerData(message, JSONBLOCKSIZE, enumAsString, skipDefaultValues);
                ParserProto parserData(serializerData, generalMessage->data.data(), generalMessage->data.size());
                parserData.parseStruct(generalMessage->type);
            }
            else
            {
                message.addSendPayload("{}", 2);
            }
        }
        else if (structBase->getStructInfo().getTypeName() != RawDataMessage::structInfo().getTypeName())
        {
            SerializerJson serializerData(message, JSONBLOCKSIZE, enumAsString, skipDefaultValues);
            ParserStruct parserData(serializerData, *structBase);
            parserData.parseStruct();
        }
    }
    else
    {
        message.addSendPayload("{}", 2);
    }
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



static size_t findEndOfPath(const char* buffer, ssize_t size)
{
    int i = 0;
    char cOld = 0;
    char c;
    while ((c = buffer[i]) && (i < size))
    {
        if (c == '{')
        {
            char cNext = buffer[i + 1];
            if (cOld != '/' && (cNext == '\"' || cNext == '}'))
            {
                return i;
            }
        }
        cOld = c;
        ++i;
    }
    return i;
}




static const std::string FMQ_PATH = "fmq_path";



std::shared_ptr<StructBase> RemoteEntityFormatJson::parse(const IProtocolSessionPtr& session, const BufferRef& bufferRef, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, int& formatStatus)
{
    formatStatus = 0;
    char* buffer = bufferRef.first;
    ssize_t sizeBuffer = bufferRef.second;

    if (sizeBuffer == 0)
    {
        return nullptr;
    }

    if (sizeBuffer > 0 && buffer[0] == '[')
    {
        ++buffer;
        --sizeBuffer;
    }
    if (sizeBuffer > 0 && buffer[sizeBuffer - 1] == ']')
    {
        --sizeBuffer;
    }

    std::string typeOfGeneralMessage;

    static const std::string WILDCARD = "*";
    const char* endHeader = nullptr;
    if (buffer[0] == '/')
    {
        // 012345678901234567890123456789
        // /MyServer/test.TestRequest!1{}
        size_t ixEndHeader = findEndOfPath(buffer, sizeBuffer);   //28
        endHeader = &buffer[ixEndHeader];

        ssize_t ixCorrelationId = findLast(buffer, ixEndHeader, '!');   //26
        if (ixCorrelationId != -1)
        {
            header.corrid = strtoll(&buffer[ixCorrelationId + 1], nullptr, 10);
        }
        else
        {
            ixCorrelationId = ixEndHeader;
        }

        std::string pathWithoutFirstSlash = { &buffer[1], &buffer[ixCorrelationId] };
        pathWithoutFirstSlash.erase(pathWithoutFirstSlash.find_last_not_of(" \n\r\t") + 1);
        size_t maxMatchLength = 0;
        const std::string* foundEntityName = nullptr;
        hybrid_ptr<IRemoteEntity> remoteEntity;
        for (auto it = name2Entity.begin(); it != name2Entity.end(); ++it)
        {
            const std::string& prefix = it->first;
            if (prefix != WILDCARD &&
                pathWithoutFirstSlash.size() >= prefix.size() &&
                pathWithoutFirstSlash.compare(0, prefix.size(), prefix) == 0 &&
                (pathWithoutFirstSlash.size() == prefix.size() || pathWithoutFirstSlash[prefix.size()] == '/') &&
                prefix.size() > maxMatchLength)
            {
                maxMatchLength = prefix.size();
                foundEntityName = &prefix;
                remoteEntity = it->second;
            }
        }
        if (foundEntityName)
        {
            header.destname = *foundEntityName;   // std::string(pathWithoutFirstSlash.c_str(), foundEntityName->size());
            if (pathWithoutFirstSlash.size() > foundEntityName->size())
            {
                header.path = std::string(&pathWithoutFirstSlash[foundEntityName->size() + 1], pathWithoutFirstSlash.size() - foundEntityName->size() - 1);
            }
        }
        else
        {
            header.destname = "*";
            header.path = pathWithoutFirstSlash;
            auto it = name2Entity.find(header.destname);
            if (it != name2Entity.end())
            {
                remoteEntity = it->second;
            }
        }
        auto entity = remoteEntity.lock();
        if (entity)
        {
            header.type = entity->getTypeOfCommandFunction(header.path, typeOfGeneralMessage);
        }

        std::string path = { &buffer[0], &buffer[ixCorrelationId] };
        path.erase(path.find_last_not_of(" \n\r\t") + 1);
        header.meta.emplace_back(FMQ_PATH);
        header.meta.emplace_back(std::move(path));

        header.mode = MsgMode::MSG_REQUEST;
    }
    else
    {
        SerializerStruct serializerHeader(header);
        ParserJson parserHeader(serializerHeader, buffer, sizeBuffer);
        endHeader = parserHeader.parseStruct(header.getStructInfo().getTypeName());
        if (endHeader)
        {
            // skip comma
            ++endHeader;
        }
        if (header.type.empty() && !header.path.empty())
        {
            hybrid_ptr<IRemoteEntity> remoteEntity;
            auto it = name2Entity.find(header.destname);
            if (it != name2Entity.end())
            {
                remoteEntity = it->second;
            }
            else
            {
                it = name2Entity.find(WILDCARD);
                if (it != name2Entity.end())
                {
                    remoteEntity = it->second;
                }
            }
            auto entity = remoteEntity.lock();
            if (entity)
            {
                header.type = entity->getTypeOfCommandFunction(header.path, typeOfGeneralMessage);
            }
        }
        if (header.path.empty() && !header.type.empty())
        {
            header.path = header.type;
        }
        else if (!header.path.empty() && header.type.empty())
        {
            header.type = header.path;
        }
    }

    std::shared_ptr<StructBase> data;

    if (endHeader)
    {
        ssize_t sizeHeader = endHeader - buffer;
        assert(sizeHeader >= 0);
        buffer += sizeHeader;
        ssize_t sizeData = sizeBuffer - sizeHeader;
        assert(sizeData >= 0);

        BufferRef bufferRefData = {buffer, sizeData};
        data = parseData(session, bufferRefData, storeRawData, header.type, formatStatus, typeOfGeneralMessage);
    }

    return data;
}



std::shared_ptr<StructBase> RemoteEntityFormatJson::parseData(const IProtocolSessionPtr& /*session*/, const BufferRef& bufferRef, bool storeRawData, std::string& type, int& formatStatus, const std::string& typeOfGeneralMessage)
{
    formatStatus = 0;
    const char* buffer = bufferRef.first;
    ssize_t sizeBuffer = bufferRef.second;

    std::shared_ptr<StructBase> data;

    ssize_t sizeData = sizeBuffer;
    assert(sizeData >= 0);
    if (!type.empty())
    {
        data = StructFactoryRegistry::instance().createStruct(type);

        if (data)
        {
            if (sizeData > 0)
            {
                if (type != GeneralMessage::structInfo().getTypeName() || typeOfGeneralMessage.empty())
                {
                    SerializerStruct serializerData(*data);
                    ParserJson parserData(serializerData, buffer, sizeData);
                    const char* endData = parserData.parseStruct(type);
                    if (!endData)
                    {
                        formatStatus |= FORMATSTATUS_SYNTAX_ERROR;
                        data = nullptr;
                    }
                }
                else
                {
                    ZeroCopyBuffer serializationBuffer;
                    SerializerProto serializerData(serializationBuffer);
                    ParserJson parserData(serializerData, buffer, sizeData);
                    const char* endData = parserData.parseStruct(typeOfGeneralMessage);
                    if (endData)
                    {
                        GeneralMessage* generalMessage = static_cast<finalmq::GeneralMessage*>(data.get());
                        generalMessage->type = typeOfGeneralMessage;
                        std::string serializedData = serializationBuffer.getData();
                        serializationBuffer.copyData(generalMessage->data);
                    }
                    else
                    {
                        formatStatus |= FORMATSTATUS_SYNTAX_ERROR;
                        data = nullptr;
                    }
                }
            }
            else
            {
                if (type == GeneralMessage::structInfo().getTypeName() && !typeOfGeneralMessage.empty())
                {
                    GeneralMessage* generalMessage = static_cast<finalmq::GeneralMessage*>(data.get());
                    generalMessage->type = typeOfGeneralMessage;
                }
            }
        }
    }

    if (storeRawData)
    {
        if (data == nullptr)
        {
            data = std::make_shared<RawDataMessage>();
        }
        data->setRawData(type, CONTENT_TYPE, buffer, sizeData);
    }

    return data;
}





}   // namespace finalmq
