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

#include "finalmq/remoteentity/RemoteEntityFormatHl7.h"

#include "finalmq/serializehl7/SerializerHl7.h"
#include "finalmq/serializestruct/ParserStruct.h"
#include "finalmq/serializehl7/ParserHl7.h"
#include "finalmq/serializestruct/SerializerStruct.h"
#include "finalmq/serializestruct/StructFactoryRegistry.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/protocolsession/ProtocolMessage.h"

#include "finalmq/remoteentity/entitydata.fmq.h"


using finalmq::MsgMode;
using finalmq::Header;


namespace finalmq {

const int RemoteEntityFormatHl7::CONTENT_TYPE = 3;
const std::string RemoteEntityFormatHl7::CONTENT_TYPE_NAME = "hl7";

const std::string RemoteEntityFormatHl7::PROPERTY_NAMESPACE = "namespace";
const std::string RemoteEntityFormatHl7::PROPERTY_ENTITY = "entity";



struct RegisterFormatHl7
{
    RegisterFormatHl7()
    {
        RemoteEntityFormatRegistry::instance().registerFormat(RemoteEntityFormatHl7::CONTENT_TYPE_NAME, RemoteEntityFormatHl7::CONTENT_TYPE, std::make_shared<RemoteEntityFormatHl7>());
    }
} g_registerFormatHl7;




void RemoteEntityFormatHl7::serialize(const IProtocolSessionPtr& session, IMessage& message, const Header& /*header*/, const StructBase* structBase)
{
    serializeData(session, message, structBase);
}


void RemoteEntityFormatHl7::serializeData(const IProtocolSessionPtr& /*session*/, IMessage& message, const StructBase* structBase)
{
    if (structBase)
    {
        // payload
        if (structBase->getRawContentType() == CONTENT_TYPE)
        {
            const std::string* rawData = structBase->getRawData();
            assert(rawData);
            char* payload = message.addSendPayload(rawData->size());
            memcpy(payload, rawData->data(), rawData->size());
        }
        else
        {
            SerializerHl7 serializerData(message, 512, true);
            ParserStruct parserData(serializerData, *structBase);
            parserData.parseStruct();
        }
    }
}






std::shared_ptr<StructBase> RemoteEntityFormatHl7::parse(const IProtocolSessionPtr& session, const BufferRef& bufferRef, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& /*name2Entity*/, Header& header, int& formatStatus)
{
    formatStatus = 0;
    char* buffer = bufferRef.first;
    ssize_t sizeBuffer = bufferRef.second;

    if (sizeBuffer == 0)
    {
        return nullptr;
    }

    std::shared_ptr<StructBase> data;

    finalmq::Hl7Header hl7Header;
    SerializerStruct serializer(hl7Header);
    ParserHl7 parserData(serializer, bufferRef.first, bufferRef.second);
    const char* endData = parserData.parseStruct("finalmq.Hl7Header");

    if (endData != nullptr)
    {
        std::string type = hl7Header.msh.messageType.messageStructure;
        if (type.empty())
        {
            type = hl7Header.msh.messageType.messageCode;
            if (!hl7Header.msh.messageType.triggerEvent.empty())
            {
                type += "_" + hl7Header.msh.messageType.triggerEvent;
            }
        }
        const std::string* hl7Namespace = nullptr;
        const std::string* hl7DestName = nullptr;
        const Variant& formatData = session->getFormatData();
        if (formatData.getType() != VARTYPE_NONE)
        {
            hl7Namespace = formatData.getData<std::string>(PROPERTY_NAMESPACE);
            hl7DestName = formatData.getData<std::string>(PROPERTY_ENTITY);
        }
        if (hl7Namespace != nullptr)
        {
            type = *hl7Namespace + "." + type;
        }
        if (hl7DestName != nullptr)
        {
            header.destname = *hl7DestName;
        }
        else
        {
            header.destname = "hl7";
        }
        header.type = type;
        if (header.path.empty())
        {
            header.path = header.type;
        }
        header.corrid = 1;

        BufferRef bufferRefData = { buffer, sizeBuffer };
        data = parseData(session, bufferRefData, storeRawData, header.type, formatStatus);

        formatStatus |= FORMATSTATUS_AUTOMATIC_CONNECT;
    }

    return data;
}



std::shared_ptr<StructBase> RemoteEntityFormatHl7::parseData(const IProtocolSessionPtr& /*session*/, const BufferRef& bufferRef, bool storeRawData, std::string& type, int& formatStatus)
{
    formatStatus = 0;
    const char* buffer = bufferRef.first;
    ssize_t sizeBuffer = bufferRef.second;

    if (sizeBuffer > 0 && buffer[0] == '{')
    {
        ++buffer;
        --sizeBuffer;
    }
    if (sizeBuffer > 0 && buffer[sizeBuffer - 1] == '}')
    {
        --sizeBuffer;
    }

    std::shared_ptr<StructBase> data;

    if (!type.empty())
    {
        data = StructFactoryRegistry::instance().createStruct(type);
        ssize_t sizeData = sizeBuffer;
        assert(sizeData >= 0);

        if (data)
        {
            if (sizeData > 0)
            {
                SerializerStruct serializerData(*data);
                ParserHl7 parserData(serializerData, buffer, sizeData);
                const char* endData = parserData.parseStruct(type);
                if (!endData)
                {
                    formatStatus |= FORMATSTATUS_SYNTAX_ERROR;
                    data = nullptr;
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
    }

    return data;
}





}   // namespace finalmq
