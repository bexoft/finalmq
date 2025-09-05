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

#include "finalmq/protocolsession/ProtocolMessage.h"
#include "finalmq/remoteentity/entitydata.fmq.h"
#include "finalmq/serializehl7/ParserHl7.h"
#include "finalmq/serializehl7/SerializerHl7.h"
#include "finalmq/serializestruct/ParserStruct.h"
#include "finalmq/serializestruct/SerializerStruct.h"
#include "finalmq/serializestruct/StructFactoryRegistry.h"
#include "finalmq/variant/VariantValues.h"

using finalmq::Header;
using finalmq::MsgMode;

namespace finalmq
{
const int RemoteEntityFormatHl7::CONTENT_TYPE = 3;
const std::string RemoteEntityFormatHl7::CONTENT_TYPE_NAME = "hl7";

const std::string RemoteEntityFormatHl7::PROPERTY_NAMESPACE = "namespace";
const std::string RemoteEntityFormatHl7::PROPERTY_ENTITY = "entity";
const std::string RemoteEntityFormatHl7::PROPERTY_LINEEND = "lineend";
const std::string RemoteEntityFormatHl7::PROPERTY_MESSAGESTART = "messagestart";
const std::string RemoteEntityFormatHl7::PROPERTY_MESSAGEEND = "messageend";

struct RegisterFormatHl7
{
    RegisterFormatHl7()
    {
        RemoteEntityFormatRegistry::instance().registerFormat(RemoteEntityFormatHl7::CONTENT_TYPE_NAME, RemoteEntityFormatHl7::CONTENT_TYPE, std::make_shared<RemoteEntityFormatHl7>());
    }
} g_registerFormatHl7;

static char* fmq_strnstr(const char* haystack, const char* needle, ssize_t len)
{
    ssize_t i;
    ssize_t j;

    if (needle[0] == '\0')
        return (const_cast<char*>(haystack));
    j = 0;
    while (j < len && haystack[j])
    {
        i = 0;
        while (j < len && needle[i] && haystack[j] && needle[i] == haystack[j])
        {
            ++i;
            ++j;
        }
        if (needle[i] == '\0')
            return (const_cast<char*>(&haystack[j - i]));
        j = j - i + 1;
    }
    return nullptr;
}

static void replaceSerialize(IMessage& source, const std::string& lineend, IMessage& destination)
{
    static const char* SOURCE_LINEEND = "\r";

    ssize_t sizeSource = source.getTotalSendPayloadSize();
    // just to reserve enough memory
    destination.addSendPayload(static_cast<ssize_t>(0), 2 * sizeSource);

    const std::list<BufferRef>& payloads = source.getAllSendPayloads();
    for (auto it = payloads.begin(); it != payloads.end(); ++it)
    {
        const BufferRef& payload = *it;

        const char* current = payload.first;
        const char* found = nullptr;
        ssize_t sizeRest = payload.second;

        while ((sizeRest > 0) && (nullptr != (found = fmq_strnstr(current, SOURCE_LINEEND, sizeRest))))
        {
            ssize_t sizeData = found - current;
            ssize_t sizeTotal = sizeData + lineend.size();
            char* buffer = destination.addSendPayload(sizeTotal, 512);
            memcpy(buffer, current, sizeData);
            memcpy(buffer + sizeData, lineend.c_str(), lineend.size());
            current = found + 1; // 1: size of character '\r'
            sizeRest -= sizeData + 1;
        }

        if (sizeRest > 0)
        {
            char* buffer = destination.addSendPayload(sizeRest, 512);
            memcpy(buffer, current, sizeRest);
        }
    }
}

static void replaceParser(const char* source, ssize_t sizeSource, const std::string& lineend, std::string& destination)
{
    static const char DESTINATION_LINEEND = '\r';

    const char* found = nullptr;
    ssize_t sizeRest = sizeSource;

    const char* current = fmq_strnstr(source, "MSH", sizeRest);
    if (!current)
    {
        return;
    }

    sizeRest -= current - source;

    // just to reserve enough memory
    destination.reserve(sizeSource);

    while ((sizeRest > 0) && (nullptr != (found = fmq_strnstr(current, lineend.c_str(), sizeRest))))
    {
        ssize_t sizeData = found - current;
        destination.append(current, sizeData);
        destination += DESTINATION_LINEEND;
        current = found + lineend.size();
        sizeRest -= sizeData + lineend.size();
    }

    if (sizeRest > 0)
    {
        destination.append(current, sizeRest);
    }
}

static bool isReplaceNeeded(const IProtocolSessionPtr& session, std::string& lineend, std::string& messagestart, std::string& messageend)
{
    lineend.clear();
    messagestart.clear();
    messageend.clear();
    const std::string* hl7lineend = nullptr;
    const std::string* hl7messagestart = nullptr;
    const std::string* hl7messageend = nullptr;
    const Variant& formatData = session->getFormatData();
    bool replaceNeeded = false;
    if (formatData.getType() != VARTYPE_NONE)
    {
        hl7lineend = formatData.getData<std::string>(RemoteEntityFormatHl7::PROPERTY_LINEEND);
        hl7messagestart = formatData.getData<std::string>(RemoteEntityFormatHl7::PROPERTY_MESSAGESTART);
        hl7messageend = formatData.getData<std::string>(RemoteEntityFormatHl7::PROPERTY_MESSAGEEND);

        replaceNeeded = ((hl7lineend != nullptr && *hl7lineend != "\r"));

        if (hl7lineend)
        {
            lineend = *hl7lineend;
        }
        if (hl7messagestart)
        {
            messagestart = *hl7messagestart;
        }
        if (hl7messageend)
        {
            messageend = *hl7messageend;
        }
    }
    return replaceNeeded;
}

void RemoteEntityFormatHl7::serialize(const IProtocolSessionPtr& session, IMessage& message, const Header& /*header*/, const StructBase* structBase)
{
    serializeData(session, message, structBase);
}

void RemoteEntityFormatHl7::serializeData(const IProtocolSessionPtr& session, IMessage& message, const StructBase* structBase)
{
    if (structBase)
    {
        std::string lineend;
        std::string messagestart;
        std::string messageend;
        bool replaceNeeded = isReplaceNeeded(session, lineend, messagestart, messageend);

        IMessagePtr messageHelper = replaceNeeded ? std::make_shared<ProtocolMessage>(0) : nullptr;
        IMessage& messageToSerialize = replaceNeeded ? *messageHelper : message;

        if (!messagestart.empty())
        {
            messageToSerialize.addSendPayload(messagestart, 512);
        }

        // payload
        if (structBase->getRawContentType() == CONTENT_TYPE)
        {
            const std::string* rawData = structBase->getRawData();
            assert(rawData);
            char* payload = messageToSerialize.addSendPayload(rawData->size());
            memcpy(payload, rawData->data(), rawData->size());
        }
        else if (structBase->getStructInfo().getTypeName() != finalmq::RawDataMessage::structInfo().getTypeName())
        {
            SerializerHl7 serializerData(messageToSerialize, 512);
            ParserStruct parserData(serializerData, *structBase);
            parserData.parseStruct();
        }

        if (!messageend.empty())
        {
            messageToSerialize.addSendPayload(messageend);
        }

        if (replaceNeeded)
        {
            replaceSerialize(messageToSerialize, lineend, message);
        }
    }
}

std::shared_ptr<StructBase> RemoteEntityFormatHl7::parse(const IProtocolSessionPtr& session, const BufferRef& bufferRef, bool storeRawData, const std::unordered_map<std::string, std::pair<EntityId, hybrid_ptr<IRemoteEntity>>>& /*name2Entity*/, Header& header, int& formatStatus)
{
    formatStatus = FORMATSTATUS_HEADER_PARSED_BY_FORMAT;
    char* buffer = bufferRef.first;
    ssize_t sizeBuffer = bufferRef.second;

    std::string bufferReplaced;
    std::string lineend;
    std::string messagestart;
    std::string messageend;
    bool replaceNeeded = isReplaceNeeded(session, lineend, messagestart, messageend);

    // skip messagestart
    if (!messagestart.empty())
    {
        if (sizeBuffer >= static_cast<ssize_t>(messagestart.size()))
        {
            if (memcmp(buffer, messagestart.c_str(), messagestart.size()) == 0)
            {
                buffer += messagestart.size();
                sizeBuffer -= messagestart.size();
            }
        }
    }

    // skip messageend
    if (!messageend.empty())
    {
        if (sizeBuffer >= static_cast<ssize_t>(messageend.size()))
        {
            ssize_t posEnd = sizeBuffer - messageend.size();
            if (memcmp(buffer + posEnd, messageend.c_str(), messageend.size()) == 0)
            {
                sizeBuffer -= messageend.size();
            }
        }
    }

    if (replaceNeeded)
    {
        replaceParser(buffer, sizeBuffer, lineend, bufferReplaced);
        buffer = bufferReplaced.data();
        sizeBuffer = bufferReplaced.size();
    }

    if (sizeBuffer == 0)
    {
        return nullptr;
    }

    std::shared_ptr<StructBase> data;

    finalmq::Hl7Header hl7Header;
    SerializerStruct serializer(hl7Header);
    ParserHl7 parserData(serializer, buffer, sizeBuffer);
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
        else
        {
            type = "hl7." + type;
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

        BufferRef bufferRefData = {buffer, sizeBuffer};
        data = parseData(session, bufferRefData, storeRawData, header.type, formatStatus, {});

        formatStatus |= FORMATSTATUS_AUTOMATIC_CONNECT;
    }

    return data;
}

std::shared_ptr<StructBase> RemoteEntityFormatHl7::parseData(const IProtocolSessionPtr& session, const BufferRef& bufferRef, bool storeRawData, std::string& type, int& formatStatus, const std::string& /*typeOfGeneralMessage*/)
{
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

    std::string bufferReplaced;
    if ((formatStatus & static_cast<int>(FORMATSTATUS_HEADER_PARSED_BY_FORMAT)) == 0)
    {
        std::string lineend;
        std::string messagestart;
        std::string messageend;
        bool replaceNeeded = isReplaceNeeded(session, lineend, messagestart, messageend);

        // skip messagestart
        if (!messagestart.empty())
        {
            if (sizeBuffer >= static_cast<ssize_t>(messagestart.size()))
            {
                if (memcmp(buffer, messagestart.c_str(), messagestart.size()) == 0)
                {
                    buffer += messagestart.size();
                    sizeBuffer -= messagestart.size();
                }
            }
        }

        // skip messageend
        if (!messageend.empty())
        {
            if (sizeBuffer >= static_cast<ssize_t>(messageend.size()))
            {
                ssize_t posEnd = sizeBuffer - messageend.size();
                if (memcmp(buffer + posEnd, messageend.c_str(), messageend.size()) == 0)
                {
                    sizeBuffer -= messageend.size();
                }
            }
        }

        if (replaceNeeded)
        {
            replaceParser(buffer, sizeBuffer, lineend, bufferReplaced);
            buffer = bufferReplaced.data();
            sizeBuffer = bufferReplaced.size();
        }
    }

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

} // namespace finalmq
