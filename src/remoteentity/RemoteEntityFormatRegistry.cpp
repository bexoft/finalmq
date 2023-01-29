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
#include "finalmq/protocolsession/ProtocolMessage.h"
#include "finalmq/variant/Variant.h"
#include "finalmq/variant/VariantValueStruct.h"
#include "finalmq/variant/VariantValues.h"
#include "finalmq/helpers/ModulenameFinalmq.h"

#include <mutex>
#include <atomic>



using finalmq::MsgMode;
using finalmq::Status;
using finalmq::Header;


namespace finalmq {



static const std::string FMQ_HTTP = "fmq_http";
static const std::string FMQ_METHOD = "fmq_method";
static const std::string FMQ_HTTP_STATUS = "fmq_http_status";
static const std::string FMQ_HTTP_STATUSTEXT = "fmq_http_statustext";
static const std::string HTTP_RESPONSE = "response";
static const std::string FMQ_PATH = "fmq_path";
static const std::string FMQ_DESTNAME = "fmq_destname";
static const std::string FMQ_DESTID = "fmq_destid";
static const std::string FMQ_SRCID = "fmq_srcid";
static const std::string FMQ_MODE = "fmq_mode";
static const std::string FMQ_CORRID = "fmq_corrid";
static const std::string FMQ_STATUS = "fmq_status";
static const std::string FMQ_SUBPATH = "fmq_subpath";
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




bool RemoteEntityFormatRegistryImpl::serialize(IMessage& message, int contentType, const Header& header, const StructBase* structBase)
{
    auto it = m_contentTypeToFormat.find(contentType);
    if (it != m_contentTypeToFormat.end())
    {
        assert(it->second);
        it->second->serialize(message, header, structBase);
        return true;
    }
    streamError << "ContentType not found: " << contentType;
    return false;
}


void RemoteEntityFormatRegistryImpl::serializeHeaderToMetainfo(IMessage& message, const Header& header)
{
    IMessage::Metainfo& metainfo = message.getAllMetainfo();
    if (!header.destname.empty())
    {
        metainfo[FMQ_DESTNAME] = header.destname;
    }
    if (header.destid != 0)
    {
        metainfo[FMQ_DESTID] = std::to_string(header.destid);
    }
    metainfo[FMQ_SRCID] = std::to_string(header.srcid);
    metainfo[FMQ_MODE] = header.mode.toString();
    if (header.corrid != 0)
    {
        metainfo[FMQ_CORRID] = std::to_string(header.corrid);
    }
    metainfo[FMQ_STATUS] = header.status.toString();
    if (!header.path.empty())
    {
        metainfo[FMQ_SUBPATH] = header.path;
    }
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
    streamError << "ContentType not found: " << contentType;
    return false;
}


inline static bool shallSend(const Header& header, const IProtocolSessionPtr& session)
{
    if ((header.mode != MsgMode::MSG_REPLY) ||
        (header.corrid != CORRELATIONID_NONE) || 
        session->needsReply())
    {
        return true;
    }
    return false;
}



static void statusToProtocolStatus(Status status, Variant& controlData, IMessage::Metainfo* metainfo, const IProtocolSessionPtr& session)
{
    controlData.add(FMQ_HTTP, HTTP_RESPONSE);
    switch (status)
    {
    case Status::STATUS_OK:
        {
            bool statusOk = true;
            if (metainfo)
            {
                auto itStatus = metainfo->find(FMQ_HTTP_STATUS);
                if (itStatus != metainfo->end())
                {
                    statusOk = false;
                    auto itStatusText = metainfo->find(FMQ_HTTP_STATUSTEXT);
                    std::string statustext;
                    if (itStatusText != metainfo->end())
                    {
                        statustext = itStatusText->second;
                    }
                    if (statustext.empty())
                    {
                        statustext = "_";
                    }
                    controlData.add(FMQ_HTTP_STATUS, itStatus->second);
                    controlData.add(FMQ_HTTP_STATUSTEXT, std::move(statustext));
                }
            }
            if (statusOk)
            {
                controlData.add(FMQ_HTTP_STATUS, 200);
                controlData.add(FMQ_HTTP_STATUSTEXT, std::string("OK"));
            }
        }
        break;
    case Status::STATUS_ENTITY_NOT_FOUND:
    case Status::STATUS_REQUEST_NOT_FOUND:
    case Status::STATUS_REQUESTTYPE_NOT_KNOWN:
        controlData.add(FMQ_HTTP_STATUS, 404);
        controlData.add(FMQ_HTTP_STATUSTEXT, std::string("Not Found"));
        break;
    case Status::STATUS_SYNTAX_ERROR:
        controlData.add(FMQ_HTTP_STATUS, 400);
        controlData.add(FMQ_HTTP_STATUSTEXT, std::string("Bad Request"));
        break;
    case Status::STATUS_NO_REPLY:
        if (session->needsReply())
        {
            controlData.add(FMQ_HTTP_STATUS, 200);
            controlData.add(FMQ_HTTP_STATUSTEXT, std::string("OK"));
        }
        else
        {
            controlData.add(FMQ_HTTP_STATUS, 500);
            controlData.add(FMQ_HTTP_STATUSTEXT, std::string("Internal Server Error"));
        }
        break;
    default:
        controlData.add(FMQ_HTTP_STATUS, 500);
        controlData.add(FMQ_HTTP_STATUSTEXT, std::string("Internal Server Error"));
        break;
    }
}

static void metainfoToHeader(Header& header, IMessage::Metainfo& metainfo)
{
    header.meta.reserve(metainfo.size() * 2);
    for (auto it = metainfo.begin(); it != metainfo.end(); ++it)
    {
        header.meta.emplace_back(it->first);
        header.meta.emplace_back(std::move(it->second));
    }
    metainfo.clear();
}


void RemoteEntityFormatRegistryImpl::send(const IProtocolSessionPtr& session, const std::string& virtualSessionId, Header& header, Variant&& echoData, const StructBase* structBase, IMessage::Metainfo* metainfo, Variant* controlData)
{
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
                if (structBase && structBase->getStructInfo().getTypeName() == RawBytes::structInfo().getTypeName())
                {
                    pureData = &static_cast<const RawBytes*>(structBase)->data;
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
            bool ok = false;
            if (!session->doesSupportMetainfo() || (session->isSendRequestByPoll() && header.mode == MsgMode::MSG_REQUEST))
            {
                ok = serialize(*message, session->getContentType(), header, structBase);
            }
            else
            {
                ok = serializeData(*message, session->getContentType(), structBase);
            }
            if (!ok)
            {
                streamError << "Could not send, because of error in serialization (content type). should never happen, because bind() or connect() have failed before.";
                throw std::logic_error("Could not send, because of error in serialization (content type). should never happen, because bind() or connect() have failed before.");
            }
        }
        else
        {
            message->addSendPayload(pureData->data(), pureData->size());
        }
        if (!virtualSessionId.empty())
        {
            Variant& controlData = message->getControlData();
            controlData.add(FMQ_VIRTUAL_SESSION_ID, virtualSessionId);
        }
        session->sendMessage(message, (header.mode == MsgMode::MSG_REPLY));
    }
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



inline static bool isDestinationIdDefined(const Header& header)
{
    return (header.destid != ENTITYID_INVALID && header.destid != ENTITYID_DEFAULT);
}

inline static bool isDestinationDefined(const Header& header)
{
    return (!header.destname.empty() || (header.destid != ENTITYID_INVALID && header.destid != ENTITYID_DEFAULT));
}

inline static bool isTypeDefined(const Header& header)
{
    return (!header.type.empty());
}

inline static bool isSubPathDefined(const Header& header)
{
    return (!header.path.empty());
}

inline static bool isDestAndSubPathDefined(const Header& header)
{
    return (isDestinationDefined(header) && isSubPathDefined(header));
}

static size_t findEndOfPath(const char* buffer)
{
    int i = 0;
    char cOld = 0;
    char c;
    while ((c = buffer[i]))
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



std::string RemoteEntityFormatRegistryImpl::parseMetainfo(IMessage& message, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header)
{
    const IMessage::Metainfo& metainfo = message.getAllMetainfo();
    auto itPath = metainfo.find(FMQ_PATH);
    auto itSrcId = metainfo.find(FMQ_SRCID);
    auto itMode = metainfo.find(FMQ_MODE);
    auto itCorrId = metainfo.find(FMQ_CORRID);
    auto itStatus = metainfo.find(FMQ_STATUS);
    auto itDestName = metainfo.find(FMQ_DESTNAME);
    auto itDestId = metainfo.find(FMQ_DESTID);
    auto itSubPath = metainfo.find(FMQ_SUBPATH);
    auto itType = metainfo.find(FMQ_TYPE);

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
        header.destname = destname;
    }

    if (itDestId != metainfo.end())
    {
        const std::string& destid = itDestId->second;
        header.destid = std::atoll(destid.c_str());
    }

    if (itType != metainfo.end())
    {
        const std::string& type = itType->second;
        header.type = type;
    }

    if (itSubPath != metainfo.end())
    {
        const std::string& path = itSubPath->second;
        header.path = path;
    }

    hybrid_ptr<IRemoteEntity> remoteEntity;
    std::string data;
    if (itPath != metainfo.end() && !isDestAndSubPathDefined(header))
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
        size_t ixEndHeader = findEndOfPath(pathWithoutFirstSlash.c_str());   //28
        data = std::string(&pathWithoutFirstSlash[ixEndHeader], pathWithoutFirstSlash.size() - ixEndHeader);

        static const std::string WILDCARD = "*";
        const std::string* foundEntityName = nullptr;
        for (auto it = name2Entity.begin(); it != name2Entity.end() && !foundEntityName; ++it)
        {
            const std::string& prefix = it->first;
            if (prefix != WILDCARD &&
                pathWithoutFirstSlash.size() >= prefix.size() &&
                pathWithoutFirstSlash.compare(0, prefix.size(), prefix) == 0 &&
                (pathWithoutFirstSlash.size() == prefix.size() || pathWithoutFirstSlash[prefix.size()] == '/'))
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
                header.path = std::string(&pathWithoutFirstSlash[foundEntityName->size() + 1], ixEndHeader - foundEntityName->size() - 1);
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
    }

    if (header.type.empty())
    {
        auto entity = remoteEntity.lock();
        if (!entity)
        {
            auto it = name2Entity.find(header.destname);
            if (it != name2Entity.end())
            {
                entity = it->second.lock();
            }
        }
        if (entity)
        {
            const std::string* method = nullptr;
            auto itMethod = metainfo.find(FMQ_METHOD);
            if (itMethod != metainfo.end())
            {
                method = &itMethod->second;
            }
            header.type = entity->getTypeOfCommandFunction(header.path, method);
        }
    }

    if (header.path.empty())
    {
        header.path = header.type;
    }

    return data;
}


std::shared_ptr<StructBase> RemoteEntityFormatRegistryImpl::parseHeaderInMetainfo(IMessage& message, int contentType, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, bool& syntaxError)
{
    std::string data = parseMetainfo(message, name2Entity, header);

    syntaxError = false;
    BufferRef bufferRef = message.getReceivePayload();

    std::shared_ptr<StructBase> structBase;

    if (header.type != RawBytes::structInfo().getTypeName())
    {
        // special feature for the browser: json data can be written into the path
        if (bufferRef.second == 0 && !data.empty())
        {
            bufferRef.first = const_cast<char*>(data.data());
            bufferRef.second = data.size();
        }

        auto it = m_contentTypeToFormat.find(contentType);
        if (it != m_contentTypeToFormat.end())
        {
            assert(it->second);
            structBase = it->second->parseData(bufferRef, storeRawData, header.type, syntaxError);
        }
    }
    else
    {
        std::shared_ptr<RawBytes> structRawBytes = std::make_shared<RawBytes>();
        structRawBytes->data = { bufferRef.first, bufferRef.first + bufferRef.second };
        structBase = structRawBytes;
    }

    return structBase;
}

std::shared_ptr<StructBase> RemoteEntityFormatRegistryImpl::parse(IMessage& message, int contentType, bool storeRawData, const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity, Header& header, bool& syntaxError)
{
    syntaxError = false;
    BufferRef bufferRef = message.getReceivePayload();
    Variant* protocolData = message.getControlData().getVariant(ProtocolMessage::FMQ_PROTOCOLDATA);

    std::shared_ptr<StructBase> structBase;

    auto it = m_contentTypeToFormat.find(contentType);
    if (it != m_contentTypeToFormat.end())
    {
        assert(it->second);
        structBase = it->second->parse(bufferRef, protocolData, storeRawData, name2Entity, header, syntaxError);
        metainfoToMessage(message, header.meta);
    }

    return structBase;
}



//std::shared_ptr<StructBase> RemoteEntityFormatRegistryImpl::parsePureData(IMessage& message, Header& header)
//{
//    const std::unordered_map<std::string, hybrid_ptr<IRemoteEntity>>& name2Entity;
//    parseMetainfo(message, name2Entity, header);
//
//    BufferRef bufferRef = message.getReceivePayload();
//
//    std::shared_ptr<RawBytes> structBytes = std::make_shared<RawBytes>();
//    structBytes->data = { bufferRef.first, bufferRef.first + bufferRef.second };
//    header.type = RawBytes::structInfo().getTypeName();
//
//    return structBytes;
//}




//////////////////////////////////////
/// RemoteEntityFormat

void RemoteEntityFormatRegistry::setInstance(std::unique_ptr<IRemoteEntityFormatRegistry>&& instanceUniquePtr)
{
    getStaticUniquePtrRef() = std::move(instanceUniquePtr);
    getStaticInstanceRef().store(getStaticUniquePtrRef().get(), std::memory_order_release);
}

IRemoteEntityFormatRegistry* RemoteEntityFormatRegistry::createInstance()
{
    static std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    IRemoteEntityFormatRegistry* inst = getStaticInstanceRef().load(std::memory_order_relaxed);
    if (!inst)
    {
        setInstance(std::make_unique<RemoteEntityFormatRegistryImpl>());
        inst = getStaticInstanceRef().load(std::memory_order_relaxed);
    }
    return inst;
}

std::atomic<IRemoteEntityFormatRegistry*>& RemoteEntityFormatRegistry::getStaticInstanceRef()
{
    static std::atomic<IRemoteEntityFormatRegistry*> instance;
    return instance;
}

std::unique_ptr<IRemoteEntityFormatRegistry>& RemoteEntityFormatRegistry::getStaticUniquePtrRef()
{
    static std::unique_ptr<IRemoteEntityFormatRegistry> instanceUniquePtr;
    return instanceUniquePtr;
}



}   // namespace finalmq
