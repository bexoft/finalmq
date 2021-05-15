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
#include "finalmq/Variant/Variant.h"
#include "finalmq/Variant/VariantValueStruct.h"
#include "finalmq/Variant/VariantValues.h"


//#include "finalmq/helpers/ModulenameFinalmq.h"


using finalmq::remoteentity::MsgMode;
using finalmq::remoteentity::Status;
using finalmq::remoteentity::Header;


namespace finalmq {



void RemoteEntityFormatRegistryImpl::registerFormat(int contentType, const std::shared_ptr<IRemoteEntityFormat>& format)
{
    m_formats[contentType] = format;
}


bool RemoteEntityFormatRegistryImpl::isRegistered(int contentType) const
{
    auto it = m_formats.find(contentType);
    if (it != m_formats.end())
    {
        return true;
    }
    return false;
}


bool RemoteEntityFormatRegistryImpl::serialize(IMessage& message, int contentType, const remoteentity::Header& header, const StructBase* structBase)
{
    auto it = m_formats.find(contentType);
    if (it != m_formats.end())
    {
        assert(it->second);
        it->second->serialize(message, header, structBase);
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

static const std::string FMQ_STATUS = "_fmq_status";
static const std::string FMQ_STATUSTEXT = "_fmq_statustext";


static void statusToProtocolStatus(remoteentity::Status status, Variant& controlData)
{
    switch (status)
    {
    case Status::STATUS_OK:
        controlData = VariantStruct{ {FMQ_STATUS, 200},
                                     {FMQ_STATUSTEXT, std::string("OK")} };
        break;
    case Status::STATUS_ENTITY_NOT_FOUND:
        controlData = VariantStruct{ {FMQ_STATUS, 404},
                                     {FMQ_STATUSTEXT, std::string("Not Found")} };
        break;
    case Status::STATUS_SYNTAX_ERROR:
        controlData = VariantStruct{ {FMQ_STATUS, 400},
                                     {FMQ_STATUSTEXT, std::string("Bad Request")} };
        break;
    case Status::STATUS_REQUEST_NOT_FOUND:
    case Status::STATUS_REQUESTTYPE_NOT_KNOWN:
        controlData = VariantStruct{ {FMQ_STATUS, 501},
                                     {FMQ_STATUSTEXT, std::string("Not Implemented")} };
        break;
    default:
        controlData = VariantStruct{ {FMQ_STATUS, 500},
                                     {FMQ_STATUSTEXT, std::string("Internal Server Error")} };
        break;
    }
}



bool RemoteEntityFormatRegistryImpl::send(const IProtocolSessionPtr& session, remoteentity::Header& header, Variant&& echoData, const StructBase* structBase)
{
    bool ok = true;
    assert(session);
    if (shallSend(header, session))
    {
        IMessagePtr message = session->createMessage();
        assert(message);
        if (header.mode == MsgMode::MSG_REPLY)
        {
            message->getEchoData() = std::move(echoData);
        }
        if (session->doesSupportMetainfo())
        {
            Variant& controlData = message->getControlData();
            statusToProtocolStatus(header.status, controlData);

            IMessage::Metainfo& metainfo = message->getAllMetainfo();
            metainfo.insert(metainfo.end(), std::make_move_iterator(header.meta.begin()), std::make_move_iterator(header.meta.end()));
            header.meta.clear();
        }
        ok = serialize(*message, session->getContentType(), header, structBase);
        if (ok)
        {
            ok = session->sendMessage(message);
        }
    }
    return ok;
}


bool RemoteEntityFormatRegistryImpl::addRequestToMessage(IMessage& message, const IProtocolSessionPtr& session, remoteentity::Header& header, const StructBase* structBase)
{
    bool ok = true;
    assert(session);
    ok = serialize(message, session->getContentType(), header, structBase);
    return ok;
}



const std::string FMQ_PATH = "_fmq_path";


std::shared_ptr<StructBase> RemoteEntityFormatRegistryImpl::parse(IMessage& message, int contentType, bool storeRawData, Header& header, bool& syntaxError)
{
    syntaxError = false;
    BufferRef bufferRef = message.getReceivePayload();

    if (bufferRef.second == 0 && !message.getAllMetainfo().empty())
    {
        const std::string* path = message.getControlData().getData<std::string>(FMQ_PATH);
        if (path && !path->empty())
        {
            bufferRef.first = const_cast<char*>(path->data());
            bufferRef.second = path->size();
        }
    }

    std::shared_ptr<StructBase> structBase;

    auto it = m_formats.find(contentType);
    if (it != m_formats.end())
    {
        assert(it->second);
        structBase = it->second->parse(bufferRef, storeRawData, header, syntaxError);
        IMessage::Metainfo& metainfo = message.getAllMetainfo();
        if (!metainfo.empty())
        {
            header.meta.insert(header.meta.end(), std::make_move_iterator(metainfo.begin()), std::make_move_iterator(metainfo.end()));
            metainfo.clear();
        }
        Variant& controlData = message.getControlData();
        if (controlData.getType() == VARTYPE_STRUCT)
        {
            VariantStruct* stru = controlData;
            if (stru)
            {
                for (auto it = stru->begin(); it != stru->end(); ++it)
                {
                    header.meta.emplace_back(it->first);
                    header.meta.push_back(it->second);
                }
            }
        }
    }

    return structBase;
}



//////////////////////////////////////
/// RemoteEntityFormat

std::unique_ptr<IRemoteEntityFormatRegistry> RemoteEntityFormatRegistry::m_instance;

void RemoteEntityFormatRegistry::setInstance(std::unique_ptr<IRemoteEntityFormatRegistry>& instance)
{
    m_instance = std::move(instance);
}




}   // namespace finalmq
