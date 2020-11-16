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

#pragma once

#include "protocolconnection/ProtocolSessionContainer.h"


#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>


namespace finalmq {

class StructBase;
class IProtocolSession;
typedef std::shared_ptr<IProtocolSession> IProtocolSessionPtr;


namespace remoteentity {
    class Header;
}


enum RemoteEntityContentType
{
    CONTENTTYPE_PROTO = 1,
    CONTENTTYPE_JSON = 2
};



using EntityId = std::uint32_t;
static constexpr EntityId ENTITYID_INVALID = 0;  // should be 0, so that it matches with the deserialized default value.

using PeerId = std::int64_t;
static constexpr PeerId PEERID_ALL = -1;

using CorrelationId = std::uint64_t;
static constexpr CorrelationId CORRELATIONID_NONE = 0;


struct ReplyContext
{
    IProtocolSessionPtr session;
    EntityId            entityId = ENTITYID_INVALID;
    CorrelationId       correlationId = CORRELATIONID_NONE;
};



struct IRemoteEntity
{
    virtual ~IRemoteEntity() {}

    virtual CorrelationId getCorrelationId() const = 0;
    virtual bool request(const PeerId& peerId, CorrelationId correlationId, const StructBase& structBase) = 0;
    virtual void reply(const ReplyContext& replyContext, const StructBase& structBase) = 0;
    virtual PeerId connect(const IProtocolSessionPtr& session, const std::string& entityName, EntityId = ENTITYID_INVALID) = 0;
    virtual void removePeer(PeerId peerId) = 0;
    virtual std::vector<PeerId> getAllPeers() const = 0;

    virtual void initEntity(EntityId entityId) = 0;
    virtual void connected(const IProtocolSessionPtr& sessionPeer, EntityId entityIdPeer) = 0;
    virtual void received(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBase& structBase) = 0;
};
typedef std::shared_ptr<IRemoteEntity> IRemoteEntityPtr;





class RemoteEntity : public IRemoteEntity
{
public:

private:
    // IRemoteEntity
    virtual CorrelationId getCorrelationId() const override;
    virtual bool request(const PeerId& peerId, CorrelationId correlationId, const StructBase& structBase) override;
    virtual void reply(const ReplyContext& replyContext, const StructBase& structBase) override;
    virtual PeerId connect(const IProtocolSessionPtr& session, const std::string& entityName, EntityId = ENTITYID_INVALID) override;
    virtual void removePeer(PeerId peerId) override;
    virtual std::vector<PeerId> getAllPeers() const override;

    virtual void initEntity(EntityId entityId) override;
    virtual void connected(const IProtocolSessionPtr& sessionPeer, EntityId entityIdPeer) override;
    virtual void received(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBase& structBase) override;

    void serializeProto(const IMessagePtr& message, const remoteentity::Header& header, const StructBase& structBase);
    void serializeJson(const IMessagePtr& message, const remoteentity::Header& header, const StructBase& structBase);
    bool send(const IProtocolSessionPtr& session, const remoteentity::Header& header, const StructBase& structBase);


    struct Peer
    {
        IProtocolSessionPtr session;
        EntityId            entityId = ENTITYID_INVALID;
        std::string         entityName;
        CorrelationId       correlationId = CORRELATIONID_NONE;
    };

    EntityId                            m_entityId = 0;

    std::unordered_map<PeerId, Peer>    m_peers;
    PeerId                              m_nextPeerId{1};
    mutable std::atomic_uint64_t        m_nextCorrelationId{1};
    mutable std::mutex                  m_mutex;
};




}   // namespace finalmq
