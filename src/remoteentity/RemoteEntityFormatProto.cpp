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

#include "finalmq/remoteentity/RemoteEntityFormatProto.h"

#include "finalmq/serializeproto/SerializerProto.h"
#include "finalmq/serializestruct/ParserStruct.h"

#include "finalmq/serializeproto/ParserProto.h"
#include "finalmq/serializestruct/SerializerStruct.h"
#include "finalmq/serializestruct/StructFactoryRegistry.h"

#include "finalmq/remoteentity/entitydata.fmq.h"

#include "finalmq/helpers/ModulenameFinalmq.h"


using finalmq::remoteentity::MsgMode;
using finalmq::remoteentity::Header;



namespace finalmq {


const int RemoteEntityFormatProto::CONTENT_TYPE = 1;
const std::string RemoteEntityFormatProto::CONTENT_TYPE_NAME = "protobuf";

static const std::string FMQ_METHOD = "fmq_method";


struct RegisterFormatProto
{
    RegisterFormatProto()
    {
        RemoteEntityFormatRegistry::instance().registerFormat(RemoteEntityFormatProto::CONTENT_TYPE_NAME, RemoteEntityFormatProto::CONTENT_TYPE, std::make_shared<RemoteEntityFormatProto>());
    }
} g_registerFormatProto;




void RemoteEntityFormatProto::serialize(IMessage& message, const Header& header, const StructBase* structBase)
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

    serializeData(message, structBase);
}


void RemoteEntityFormatProto::serializeData(IMessage& message, const StructBase* structBase)
{
    if (structBase)
    {
        char* bufferSizePayload = message.addSendPayload(4);
        ssize_t sizeStart = message.getTotalSendPayloadSize();
        if (structBase->getRawContentType() == CONTENT_TYPE)
        {
            const std::string* rawData = structBase->getRawData();
            assert(rawData);
            char* payload = message.addSendPayload(rawData->size());
            memcpy(payload, rawData->data(), rawData->size());
        }
        else
        {
            SerializerProto serializerData(message);
            ParserStruct parserData(serializerData, *structBase);
            parserData.parseStruct();
        }
        ssize_t sizeEnd = message.getTotalSendPayloadSize();
        ssize_t sizePayload = sizeEnd - sizeStart;
        assert(sizePayload >= 0);
        *bufferSizePayload = static_cast<unsigned char>(sizePayload);
        ++bufferSizePayload;
        *bufferSizePayload = static_cast<unsigned char>(sizePayload >> 8);
        ++bufferSizePayload;
        *bufferSizePayload = static_cast<unsigned char>(sizePayload >> 16);
        ++bufferSizePayload;
        *bufferSizePayload = static_cast<unsigned char>(sizePayload >> 24);
    }
}

static const std::string FMQ_PATH = "fmq_path";

std::shared_ptr<StructBase> RemoteEntityFormatProto::parse(const BufferRef& bufferRef, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& /*name2Entity*/, Header& header, bool& syntaxError)
{
    syntaxError = false;
    char* buffer = bufferRef.first;
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

    if (sizeHeader <= sizePayload)
    {
        SerializerStruct serializerHeader(header);
        ParserProto parserHeader(serializerHeader, buffer, sizeHeader);
        ok = parserHeader.parseStruct(Header::structInfo().getTypeName());
        if (header.path.empty() && !header.type.empty())
        {
            header.path = header.type;
        }
        else if (!header.path.empty() && header.type.empty())
        {
            header.type = header.path;
        }
        header.meta.emplace_back(FMQ_PATH);
        header.meta.emplace_back(header.destname);
    }

    std::shared_ptr<StructBase> data;

    if (ok)
    {
        ssize_t sizeData = sizePayload - sizeHeader;
        buffer += sizeHeader;

        BufferRef bufferRefData = { buffer, sizeData };
        data = parseData(bufferRefData, storeRawData, header.type, nullptr, syntaxError);
    }

    return data;
}



std::shared_ptr<StructBase> RemoteEntityFormatProto::parseData(const BufferRef& bufferRef, bool storeRawData, std::string& type, const IMessage::Metainfo* metainfo, bool& syntaxError)
{
    syntaxError = false;
    const char* buffer = bufferRef.first;
    ssize_t sizeBuffer = bufferRef.second;
    if (sizeBuffer < 4)
    {
        streamError << "buffer size too small: " << sizeBuffer;
        return nullptr;
    }

    std::shared_ptr<StructBase> data;

    if (!type.empty())
    {
        bool ok = true;
        ssize_t sizeRemaining = sizeBuffer;

        if (sizeBuffer < 4)
        {
            ok = false;
        }

        ssize_t sizeDataInStream = 0;
        if (ok)
        {
            sizeDataInStream = (unsigned char)*buffer;
            ++buffer;
            sizeDataInStream |= ((unsigned char)*buffer) << 8;
            ++buffer;
            sizeDataInStream |= ((unsigned char)*buffer) << 16;
            ++buffer;
            sizeDataInStream |= ((unsigned char)*buffer) << 24;
            ++buffer;
            sizeRemaining -= 4;
        }

        if (sizeDataInStream > sizeRemaining)
        {
            ok = false;
        }

        if (ok)
        {
            data = StructFactoryRegistry::instance().createStruct(type);
            if (data == nullptr && metainfo)
            {
                // try to lookup for type_method
                auto it = metainfo->find(FMQ_METHOD);
                if (it != metainfo->end())
                {
                    std::string trytype = type + '_' + it->second;
                    data = StructFactoryRegistry::instance().createStruct(trytype);
                    if (data != nullptr)
                    {
                        type = trytype;
                    }
                }
            }
            if (data)
            {
                assert(sizeDataInStream >= 0);
                SerializerStruct serializerData(*data);
                ParserProto parserData(serializerData, buffer, sizeDataInStream);
                ok = parserData.parseStruct(type);
                if (!ok)
                {
                    syntaxError = true;
                    data = nullptr;
                }
            }

            if (storeRawData)
            {
                if (data == nullptr)
                {
                    data = std::make_shared<remoteentity::RawDataMessage>();
                }
                data->setRawData(type, CONTENT_TYPE, buffer, sizeDataInStream);
            }
        }
    }

    return data;
}




}   // namespace finalmq
