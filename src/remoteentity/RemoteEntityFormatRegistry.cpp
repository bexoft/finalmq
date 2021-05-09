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


static void statusToProtocolStatus(remoteentity::Status status, IMessage::Metainfo& metainfo)
{
    switch (status)
    {
    case Status::STATUS_OK:
        metainfo.emplace_back(FMQ_STATUS);
        metainfo.emplace_back("200");
        metainfo.emplace_back(FMQ_STATUSTEXT);
        metainfo.emplace_back("OK");
        break;
    case Status::STATUS_ENTITY_NOT_FOUND:
        metainfo.emplace_back(FMQ_STATUS);
        metainfo.emplace_back("404");
        metainfo.emplace_back(FMQ_STATUSTEXT);
        metainfo.emplace_back("Not Found");
        break;
    case Status::STATUS_SYNTAX_ERROR:
        metainfo.emplace_back(FMQ_STATUS);
        metainfo.emplace_back("400");
        metainfo.emplace_back(FMQ_STATUSTEXT);
        metainfo.emplace_back("Bad Request");
        break;
    case Status::STATUS_REQUEST_NOT_FOUND:
    case Status::STATUS_REQUESTTYPE_NOT_KNOWN:
        metainfo.emplace_back(FMQ_STATUS);
        metainfo.emplace_back("501");
        metainfo.emplace_back(FMQ_STATUSTEXT);
        metainfo.emplace_back("Not Implemented");
        break;
    default:
        metainfo.emplace_back(FMQ_STATUS);
        metainfo.emplace_back("500");
        metainfo.emplace_back(FMQ_STATUSTEXT);
        metainfo.emplace_back("Internal Server Error");
        break;
    }
}



bool RemoteEntityFormatRegistryImpl::send(const IProtocolSessionPtr& session, remoteentity::Header& header, const StructBase* structBase)
{
    bool ok = true;
    assert(session);
    if (shallSend(header, session))
    {
        IMessagePtr message = session->createMessage();
        assert(message);
        if (session->doesSupportMetainfo())
        {
            IMessage::Metainfo& metainfo = message->getAllMetainfo();
            statusToProtocolStatus(header.status, metainfo);
            metainfo.insert(metainfo.end(), std::make_move_iterator(header.meta.begin()), std::make_move_iterator(header.meta.end()));
            header.meta.clear();
        }
        else if (!header.meta.empty())
        {
            IMessage::Metainfo& metainfo = message->getAllMetainfo();
            std::vector<std::string> metaHeader;
            metaHeader.reserve(header.meta.size());
            for (size_t i = 0; i < header.meta.size(); i += 2)
            {
                if (i + 1 < header.meta.size())
                {
                    if (header.meta[i].compare(0, 5, "_fmq_") == 0)
                    {
                        metainfo.emplace_back(std::move(header.meta[i]));
                        metainfo.emplace_back(std::move(header.meta[i + 1]));
                    }
                    else
                    {
                        metaHeader.emplace_back(std::move(header.meta[i]));
                        metaHeader.emplace_back(std::move(header.meta[i + 1]));
                    }
                }
            }
            header.meta = std::move(metaHeader);
        }
        ok = serialize(*message, session->getContentType(), header, structBase);
        if (ok)
        {
            ok = session->sendMessage(message);
        }
    }
    return ok;
}


const std::string FMQ_PATH = "_fmq_path";


std::shared_ptr<StructBase> RemoteEntityFormatRegistryImpl::parse(const IMessage& message, int contentType, bool storeRawData, Header& header, bool& syntaxError)
{
    syntaxError = false;
    BufferRef bufferRef = message.getReceivePayload();

    if (bufferRef.second == 0 && !message.getAllMetainfo().empty())
    {
        const std::string* path = message.getMetainfo(FMQ_PATH);
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
        const IMessage::Metainfo& metainfo = message.getAllMetainfo();
        if (!metainfo.empty())
        {
            header.meta.insert(header.meta.end(), metainfo.begin(), metainfo.end());
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
