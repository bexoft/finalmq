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
static const std::string FMQ_DESTNAME = "fmq_destname";
static const std::string FMQ_RE_DESTID = "fmq_re_destid";
static const std::string FMQ_RE_SRCID = "fmq_re_srcid";
static const std::string FMQ_RE_MODE = "fmq_re_mode";
static const std::string FMQ_CORRID = "fmq_corrid";
static const std::string FMQ_RE_STATUS = "fmq_re_status";
static const std::string FMQ_TYPE = "fmq_type";
static const std::string MSG_REPLY = "MSG_REPLY";

static const std::string FMQ_VIRTUAL_SESSION_ID = "fmq_virtsessid";



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
    if (!header.destname.empty())
    {
        metainfo[FMQ_DESTNAME] = header.destname;
    }
    if (header.destid != 0)
    {
        metainfo[FMQ_RE_DESTID] = std::to_string(header.destid);
    }
    metainfo[FMQ_RE_SRCID] = std::to_string(header.srcid);
    metainfo[FMQ_RE_MODE] = header.mode.toString();
    if (header.corrid != 0)
    {
        metainfo[FMQ_CORRID] = std::to_string(header.corrid);
    }
    metainfo[FMQ_RE_STATUS] = header.status.toString();
    metainfo[FMQ_TYPE] = header.type;
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



static void statusToProtocolStatus(remoteentity::Status status, Variant& controlData, IMessage::Metainfo* metainfo, const IProtocolSessionPtr& session)
{
    controlData.add(FMQ_HTTP, HTTP_RESPONSE);
    switch (status)
    {
    case Status::STATUS_OK:
        {
            bool statusOk = true;
            if (metainfo)
            {
                auto itStatus = metainfo->find(FMQ_STATUS);
                if (itStatus != metainfo->end())
                {
                    statusOk = false;
                    auto itStatusText = metainfo->find(FMQ_STATUSTEXT);
                    std::string statustext;
                    if (itStatusText != metainfo->end())
                    {
                        statustext = itStatusText->second;
                    }
                    if (statustext.empty())
                    {
                        statustext = "_";
                    }
                    controlData.add(FMQ_STATUS, itStatus->second);
                    controlData.add(FMQ_STATUSTEXT, std::move(statustext));
                }
            }
            if (statusOk)
            {
                controlData.add(FMQ_STATUS, 200);
                controlData.add(FMQ_STATUSTEXT, std::string("OK"));
            }
        }
        break;
    case Status::STATUS_ENTITY_NOT_FOUND:
        controlData.add(FMQ_STATUS, 404);
        controlData.add(FMQ_STATUSTEXT, std::string("Not Found"));
        break;
    case Status::STATUS_SYNTAX_ERROR:
        controlData.add(FMQ_STATUS, 400);
        controlData.add(FMQ_STATUSTEXT, std::string("Bad Request"));
        break;
    case Status::STATUS_REQUEST_NOT_FOUND:
    case Status::STATUS_REQUESTTYPE_NOT_KNOWN:
        controlData.add(FMQ_STATUS, 501);
        controlData.add(FMQ_STATUSTEXT, std::string("Not Implemented"));
        break;
    case Status::STATUS_NO_REPLY:
        if (session->needsReply())
        {
            controlData.add(FMQ_STATUS, 200);
            controlData.add(FMQ_STATUSTEXT, std::string("OK"));
        }
        else
        {
            controlData.add(FMQ_STATUS, 500);
            controlData.add(FMQ_STATUSTEXT, std::string("Internal Server Error"));
        }
        break;
    default:
        controlData.add(FMQ_STATUS, 500);
        controlData.add(FMQ_STATUSTEXT, std::string("Internal Server Error"));
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


bool RemoteEntityFormatRegistryImpl::send(const IProtocolSessionPtr& session, const std::string& virtualSessionId, remoteentity::Header& header, Variant&& echoData, const StructBase* structBase, IMessage::Metainfo* metainfo, Variant* controlData)
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
        if (!header.destname.empty())
        {
            Variant& controlDataTmp = message->getControlData();
            controlDataTmp.add(FMQ_DESTNAME, header.destname);
        }
        if (!virtualSessionId.empty())
        {
            Variant& controlDataTmp = message->getControlData();
            controlDataTmp.add(FMQ_VIRTUAL_SESSION_ID, virtualSessionId);
        }
        bool writeMetainfoToHeader = metainfo;
        if (session->doesSupportMetainfo())
        {
            if (header.mode == MsgMode::MSG_REPLY)
            {
                Variant& controlDataTmp = message->getControlData();
                statusToProtocolStatus(header.status, controlDataTmp, metainfo, session);
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
            VariantStruct* varstruct = message->getControlData().getData<VariantStruct>("");
            VariantStruct* varstruct2 = controlData->getData<VariantStruct>("");
            if (varstruct && varstruct2)
            {
                varstruct->insert(varstruct->begin(), std::make_move_iterator(varstruct2->begin()), std::make_move_iterator(varstruct2->end()));
                varstruct2->clear();
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
            if (!virtualSessionId.empty())
            {
                Variant& controlData = message->getControlData();
                controlData.add(FMQ_VIRTUAL_SESSION_ID, virtualSessionId);
            }
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







void RemoteEntityFormatRegistryImpl::parseMetainfo(IMessage& message, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, remoteentity::Header& header)
{
    const IMessage::Metainfo& metainfo = message.getAllMetainfo();
    auto itPath = metainfo.find(FMQ_PATH);
    auto itSrcId = metainfo.find(FMQ_RE_SRCID);
    auto itMode = metainfo.find(FMQ_RE_MODE);
    auto itCorrId = metainfo.find(FMQ_CORRID);
    auto itStatus = metainfo.find(FMQ_RE_STATUS);
    auto itDestName = metainfo.find(FMQ_DESTNAME);
    auto itDestId = metainfo.find(FMQ_RE_DESTID);
    auto itType = metainfo.find(FMQ_TYPE);
    if (itPath != metainfo.end())
    {
        const std::string& path = itPath->second;

        // 012345678901234567890123456789
        // /MyServer/test.TestRequest!1{}
        std::string pathWithoutFirstSlash;
        if (path[0] == '/')
        {
            pathWithoutFirstSlash = { &path[1], path.size() - 1 };
        }
        else
        {
            pathWithoutFirstSlash = path;
        }
        size_t ixEndHeader = path.find_first_of('{');   //28
        if (ixEndHeader == std::string::npos)
        {
            ixEndHeader = path.size();
        }

        const std::string* foundEntityName = nullptr;
        hybrid_ptr<IRemoteEntity> remoteEntity;
        for (auto it = name2Entity.begin(); it != name2Entity.end() && !foundEntityName; ++it)
        {
            const std::string& prefix = it->first;
            if (pathWithoutFirstSlash.size() >= prefix.size() && pathWithoutFirstSlash.compare(0, prefix.size(), prefix) == 0)
            {
                foundEntityName = &prefix;
                remoteEntity = it->second;
            }
        }
        if (foundEntityName)
        {
            header.destname = std::string(pathWithoutFirstSlash.c_str(), foundEntityName->size());
            if (pathWithoutFirstSlash.size() > foundEntityName->size())
            {
                header.path = std::string(&pathWithoutFirstSlash[foundEntityName->size() + 1], pathWithoutFirstSlash.size() - foundEntityName->size() - 1);
                auto entity = remoteEntity.lock();
                if (entity)
                {
                    header.type = entity->getTypeOfCommandFunction(header.path);
                }
            }
        }
        else
        {
            header.destname = pathWithoutFirstSlash;
        }
    }

    if (itSrcId != metainfo.end())
    {
        const std::string& srcid = itSrcId->second;
        header.srcid = std::atoll(srcid.c_str());
    }

    if (itMode != metainfo.end())
    {
        const std::string& mode = itMode->second;
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

    if (itDestName != metainfo.end())
    {
        const std::string& destname = itDestName->second;
        if (!destname.empty())
        {
            header.destname = destname;
        }
    }

    if (itDestId != metainfo.end())
    {
        const std::string& destid = itDestId->second;
        header.destid = std::atoll(destid.c_str());
    }

    if (itType != metainfo.end())
    {
        const std::string& type = itType->second;
        if (!type.empty())
        {
            header.type = type;
        }
    }
}


std::shared_ptr<StructBase> RemoteEntityFormatRegistryImpl::parseHeaderInMetainfo(IMessage& message, int contentType, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, bool& syntaxError)
{
    parseMetainfo(message, name2Entity, header);

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
        structBase = it->second->parseData(bufferRef, storeRawData, header.type, &message.getAllMetainfo(), syntaxError);
    }

    return structBase;
}

std::shared_ptr<StructBase> RemoteEntityFormatRegistryImpl::parse(IMessage& message, int contentType, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, bool& syntaxError)
{
    syntaxError = false;
    BufferRef bufferRef = message.getReceivePayload();

    std::shared_ptr<StructBase> structBase;

    auto it = m_contentTypeToFormat.find(contentType);
    if (it != m_contentTypeToFormat.end())
    {
        assert(it->second);
        structBase = it->second->parse(bufferRef, storeRawData, name2Entity, header, syntaxError);
        metainfoToMessage(message, header.meta);
    }

    return structBase;
}



std::shared_ptr<StructBase> RemoteEntityFormatRegistryImpl::parsePureData(IMessage& message, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header)
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
