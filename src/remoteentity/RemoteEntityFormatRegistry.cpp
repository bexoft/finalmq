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

#include "finalmq/remoteentity/RemoteEntityFormatRegistry.h"
#include "finalmq/remoteentity/entitydata.fmq.h"
#include "finalmq/variant/Variant.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValues.h"


//#include "finalmq/helpers/ModulenameFinalmq.h"


using finalmq::remoteentity::MsgMode;
using finalmq::remoteentity::Status;
using finalmq::remoteentity::Header;


namespace finalmq {



static const std::string FMQ_HTTP = "fmq_http";
static const std::string FMQ_STATUS = "fmq_status";
static const std::string FMQ_STATUSTEXT = "fmq_statustext";
static const std::string HTTP_RESPONSE = "response";
static const std::string FMQ_PATH = "fmq_path";
static const std::string FMQ_RE_SRCID = "fmq_re_srcid";
static const std::string FMQ_RE_MODE = "fmq_re_mode";
static const std::string FMQ_RE_CORRID = "fmq_re_corrid";
static const std::string FMQ_RE_STATUS = "fmq_re_status";
static const std::string FMQ_RE_TYPE = "fmq_re_type";
static const std::string MSG_REPLY = "MSG_REPLY";




void RemoteEntityFormatRegistryImpl::registerFormat(const std::string& contentTypeName, int contentType, const std::shared_ptr<IRemoteEntityFormat>& format)
{
    m_contentTypeToFormat[contentType] = format;
    m_contentTypeNameToContentType[contentTypeName] = contentType;
}


bool RemoteEntityFormatRegistryImpl::isRegistered(int contentType) const
{
    auto it = m_contentTypeToFormat.find(contentType);
    if (it != m_contentTypeToFormat.end())
    {
        return true;
    }
    return false;
}


int RemoteEntityFormatRegistryImpl::getContentType(const std::string& contentTypeName) const
{
    auto it = m_contentTypeNameToContentType.find(contentTypeName);
    if (it != m_contentTypeNameToContentType.end())
    {
        return it->second;
    }
    return 0;
}




bool RemoteEntityFormatRegistryImpl::serialize(IMessage& message, int contentType, const remoteentity::Header& header, const StructBase* structBase)
{
    auto it = m_contentTypeToFormat.find(contentType);
    if (it != m_contentTypeToFormat.end())
    {
        assert(it->second);
        it->second->serialize(message, header, structBase);
        return true;
    }
    return false;
}


void RemoteEntityFormatRegistryImpl::serializeHeaderToMetainfo(IMessage& message, const remoteentity::Header& header)
{
    IMessage::Metainfo& metainfo = message.getAllMetainfo();
    metainfo[FMQ_RE_SRCID] = std::to_string(header.srcid);
    metainfo[FMQ_RE_MODE] = header.mode.toString();
    metainfo[FMQ_RE_CORRID] = std::to_string(header.corrid);
    metainfo[FMQ_RE_STATUS] = header.status.toString();
    metainfo[FMQ_RE_TYPE] = header.type;
}


bool RemoteEntityFormatRegistryImpl::serializeData(IMessage& message, int contentType, const StructBase* structBase)
{
    auto it = m_contentTypeToFormat.find(contentType);
    if (it != m_contentTypeToFormat.end())
    {
        assert(it->second);
        it->second->serializeData(message, structBase);
        return true;
    }
    return false;
}


inline static bool shallSend(const remoteentity::Header& header, const IProtocolSessionPtr& session)
{
    if ((header.mode != MsgMode::MSG_REPLY) ||
        (header.corrid != CORRELATIONID_NONE) || 
        session->needsReply())
    {
        return true;
    }
    return false;
}



static void statusToProtocolStatus(remoteentity::Status status, Variant& controlData, const IProtocolSessionPtr& session)
{
    switch (status)
    {
    case Status::STATUS_OK:
        controlData = VariantStruct{ {FMQ_HTTP, HTTP_RESPONSE},
                                     {FMQ_STATUS, 200},
                                     {FMQ_STATUSTEXT, std::string("OK")} };
        break;
    case Status::STATUS_ENTITY_NOT_FOUND:
        controlData = VariantStruct{ {FMQ_HTTP, HTTP_RESPONSE},
                                     {FMQ_STATUS, 404},
                                     {FMQ_STATUSTEXT, std::string("Not Found")} };
        break;
    case Status::STATUS_SYNTAX_ERROR:
        controlData = VariantStruct{ {FMQ_HTTP, HTTP_RESPONSE},
                                     {FMQ_STATUS, 400},
                                     {FMQ_STATUSTEXT, std::string("Bad Request")} };
        break;
    case Status::STATUS_REQUEST_NOT_FOUND:
    case Status::STATUS_REQUESTTYPE_NOT_KNOWN:
        controlData = VariantStruct{ {FMQ_HTTP, HTTP_RESPONSE},
                                     {FMQ_STATUS, 501},
                                     {FMQ_STATUSTEXT, std::string("Not Implemented")} };
        break;
    case Status::STATUS_NO_REPLY:
        if (session->needsReply())
        {
            controlData = VariantStruct{ {FMQ_HTTP, HTTP_RESPONSE},
                                         {FMQ_STATUS, 200},
                                         {FMQ_STATUSTEXT, std::string("OK")} };
        }
        else
        {
            controlData = VariantStruct{ {FMQ_HTTP, HTTP_RESPONSE},
                                         {FMQ_STATUS, 500},
                                         {FMQ_STATUSTEXT, std::string("Internal Server Error")} };
        }
        break;
    default:
        controlData = VariantStruct{ {FMQ_HTTP, HTTP_RESPONSE},
                                     {FMQ_STATUS, 500},
                                     {FMQ_STATUSTEXT, std::string("Internal Server Error")} };
        break;
    }
}

static void metainfoToHeader(remoteentity::Header& header, IMessage::Metainfo& metainfo)
{
    header.meta.reserve(metainfo.size() * 2);
    for (auto it = metainfo.begin(); it != metainfo.end(); ++it)
    {
        header.meta.emplace_back(it->first);
        header.meta.emplace_back(std::move(it->second));
    }
    metainfo.clear();
}


bool RemoteEntityFormatRegistryImpl::send(const IProtocolSessionPtr& session, remoteentity::Header& header, Variant&& echoData, const StructBase* structBase, IMessage::Metainfo* metainfo, Variant* controlData)
{
    bool ok = true;
    assert(session);
    if (shallSend(header, session))
    {
        const finalmq::Bytes* pureData = nullptr;
        IMessagePtr message = session->createMessage();
        assert(message);
        if (header.mode == MsgMode::MSG_REPLY)
        {
            message->getEchoData() = std::move(echoData);
        }
        bool writeMetainfoToHeader = metainfo;
        if (session->doesSupportMetainfo())
        {
            if (header.mode == MsgMode::MSG_REPLY)
            {
                Variant& controlData = message->getControlData();
                statusToProtocolStatus(header.status, controlData, session);
                if (structBase && structBase->getStructInfo().getTypeName() == remoteentity::Bytes::structInfo().getTypeName())
                {
                    pureData = &static_cast<const remoteentity::Bytes*>(structBase)->data;
                }
            }

            // not a poll request
            if (metainfo && (!session->isSendRequestByPoll() || header.mode == MsgMode::MSG_REPLY))
            {
                IMessage::Metainfo& metainfoMessage = message->getAllMetainfo();
                metainfoMessage.insert(std::make_move_iterator(metainfo->begin()), std::make_move_iterator(metainfo->end()));
                metainfo->clear();
                writeMetainfoToHeader = false;
            }

            serializeHeaderToMetainfo(*message, header);
        }

        if (writeMetainfoToHeader && metainfo)
        {
            metainfoToHeader(header, *metainfo);
        }

        if (controlData)
        {
            if (controlData->getType() != VARTYPE_STRUCT)
            {
                message->getControlData() = std::move(*controlData);
            }
            else
            {
                VariantStruct* varstruct = message->getControlData().getData<VariantStruct>("");
                VariantStruct* varstruct2 = controlData->getData<VariantStruct>("");
                if (varstruct && varstruct2)
                {
                    varstruct->insert(varstruct->begin(), std::make_move_iterator(varstruct2->begin()), std::make_move_iterator(varstruct2->end()));
                    varstruct2->clear();
                }
            }
        }
        if (pureData == nullptr)
        {
            if (!session->doesSupportMetainfo() || (session->isSendRequestByPoll() && header.mode == MsgMode::MSG_REQUEST))
            {
                ok = serialize(*message, session->getContentType(), header, structBase);
            }
            else
            {
                ok = serializeData(*message, session->getContentType(), structBase);
            }
        }
        else
        {
            ok = true;
            message->addSendPayload(pureData->data(), pureData->size());
        }
        if (ok)
        {
            ok = session->sendMessage(message, (header.mode == MsgMode::MSG_REPLY));
        }
    }
    return ok;
}



static void metainfoToMessage(IMessage& message, std::vector<std::string>& meta)
{
    size_t size = meta.size();
    if (size > 0)
    {
        --size;
    }
    IMessage::Metainfo& metainfo = message.getAllMetainfo();
    for (size_t i = 0; i < size; i += 2)
    {
        metainfo[std::move(meta[i])] = std::move(meta[i + 1]);
    }
    meta.clear();
}







void RemoteEntityFormatRegistryImpl::parseMetainfo(IMessage& message, remoteentity::Header& header)
{
    const IMessage::Metainfo& metainfo = message.getAllMetainfo();
    auto itPath = metainfo.find(FMQ_PATH);
    auto itSrcId = metainfo.find(FMQ_RE_SRCID);
    auto itMode = metainfo.find(FMQ_RE_MODE);
    auto itCorrId = metainfo.find(FMQ_RE_CORRID);
    auto itStatus = metainfo.find(FMQ_RE_STATUS);
    if (itPath != metainfo.end())
    {
        const std::string& path = itPath->second;

        // 012345678901234567890123456789
        // /MyServer/test.TestRequest!1{}
        ssize_t ixStartDestName = 0;
        if (path[0] == '/')
        {
            ixStartDestName = 1;
        }
        size_t ixEndHeader = path.find_first_of('{');   //28
        if (ixEndHeader == std::string::npos)
        {
            ixEndHeader = path.size();
        }
        //endHeader = &buffer[ixEndHeader];
        ssize_t ixStartCommand = path.find_last_of('/', ixEndHeader);    //9
        if (ixStartCommand != 0 && ixStartCommand != (ssize_t)std::string::npos)
        {
            header.destname = { &path[ixStartDestName], &path[ixStartCommand] };
            header.type = { &path[ixStartCommand + 1], &path[ixEndHeader] };
        }
        else
        {
            header.destname = { &path[ixStartDestName], &path[ixEndHeader] };
        }
    }

    if (itSrcId != metainfo.end())
    {
        const std::string& srcid = itSrcId->second;
        header.srcid = std::atoll(srcid.c_str());
    }

    if (itMode != metainfo.end())
    {
        const std::string& mode = itSrcId->second;
        if (mode == MSG_REPLY)
        {
            header.mode = MsgMode::MSG_REPLY;
        }
        else
        {
            header.mode = MsgMode::MSG_REQUEST;
        }
    }

    if (itCorrId != metainfo.end())
    {
        const std::string& corrid = itCorrId->second;
        header.corrid = std::atoll(corrid.c_str());
    }

    if (itStatus != metainfo.end())
    {
        const std::string& status = itStatus->second;
        header.status.fromString(status);
    }
}


std::shared_ptr<StructBase> RemoteEntityFormatRegistryImpl::parseHeaderInMetainfo(IMessage& message, int contentType, bool storeRawData, Header& header, bool& syntaxError)
{
    parseMetainfo(message, header);

    syntaxError = false;
    BufferRef bufferRef = message.getReceivePayload();

    // special feature for the browser: json data can be written into the path
    if (bufferRef.second == 0)
    {
        std::string* path = message.getMetainfo(FMQ_PATH);
        if (path && !path->empty())
        {
            size_t pos = path->find_first_of('{');
            if (pos != std::string::npos)
            {
                bufferRef.first = &path->at(pos);
                bufferRef.second = path->size() - pos;
            }
        }
    }

    std::shared_ptr<StructBase> structBase;

    auto it = m_contentTypeToFormat.find(contentType);
    if (it != m_contentTypeToFormat.end())
    {
        assert(it->second);
        structBase = it->second->parseData(bufferRef, storeRawData, header.type, syntaxError);
    }

    return structBase;
}

std::shared_ptr<StructBase> RemoteEntityFormatRegistryImpl::parse(IMessage& message, int contentType, bool storeRawData, Header& header, bool& syntaxError)
{
    syntaxError = false;
    BufferRef bufferRef = message.getReceivePayload();

    std::shared_ptr<StructBase> structBase;

    auto it = m_contentTypeToFormat.find(contentType);
    if (it != m_contentTypeToFormat.end())
    {
        assert(it->second);
        structBase = it->second->parse(bufferRef, storeRawData, header, syntaxError);
        metainfoToMessage(message, header.meta);
    }

    return structBase;
}



std::shared_ptr<StructBase> RemoteEntityFormatRegistryImpl::parsePureData(IMessage& message, Header& header)
{
    std::string* path = message.getMetainfo(FMQ_PATH);
    if (path && !path->empty())
    {
        header.destname = *path;
    }

    std::shared_ptr<remoteentity::Bytes> structBytes = std::make_shared<remoteentity::Bytes>();
    BufferRef rec = message.getReceivePayload();
    structBytes->data = { rec.first, rec.first + rec.second };
    header.type = remoteentity::Bytes::structInfo().getTypeName();

    return structBytes;
}




//////////////////////////////////////
/// RemoteEntityFormat

std::unique_ptr<IRemoteEntityFormatRegistry> RemoteEntityFormatRegistry::m_instance;

void RemoteEntityFormatRegistry::setInstance(std::unique_ptr<IRemoteEntityFormatRegistry>& instance)
{
    m_instance = std::move(instance);
}




}   // namespace finalmq
