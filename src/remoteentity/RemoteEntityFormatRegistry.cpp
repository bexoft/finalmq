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


inline static bool shallSend(const remoteentity::Header& header)
{
    if ((header.mode != MsgMode::MSG_REPLY) ||
        (/*header.status == Status::STATUS_ENTITY_NOT_FOUND ||*/ header.corrid != CORRELATIONID_NONE))
    {
        return true;
    }
    return false;
}




bool RemoteEntityFormatRegistryImpl::send(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBase* structBase)
{
    bool ok = true;
    if (shallSend(header))
    {
        assert(session);
        IMessagePtr message = session->createMessage();
        assert(message);
        ok = serialize(*message, session->getContentType(), header, structBase);
        if (ok)
        {
            ok = session->sendMessage(message);
        }
    }
    return ok;
}




std::shared_ptr<StructBase> RemoteEntityFormatRegistryImpl::parse(const IMessage& message, int contentType, bool storeRawData, Header& header, bool& syntaxError)
{
    syntaxError = false;
    BufferRef bufferRef = message.getReceivePayload();
    std::shared_ptr<StructBase> structBase;

    auto it = m_formats.find(contentType);
    if (it != m_formats.end())
    {
        assert(it->second);
        structBase = it->second->parse(bufferRef, storeRawData, header, syntaxError);
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
