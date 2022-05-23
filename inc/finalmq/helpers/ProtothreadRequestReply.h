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


#include "finalmq/helpers/Protothread.h"

namespace finalmq {

    class ProtothreadRequestReply : public Protothread
    {
    public:
        template<class R>
        struct ReplyResult
        {
            Status status;
            std::shared_ptr<R> reply;
            IMessage::Metainfo metainfo;
        };
        typedef std::function<bool(ProtothreadRequestReply& protothread)> FuncRun;
        typedef std::function<void()> FuncCallbackThreadDone;
        
        ProtothreadRequestReply(FuncRun callbackRun, FuncCallbackThreadDone callbackDone = nullptr)
            : Protothread()
            , m_callbackRun(std::move(callbackRun))
            , m_callbackDone(std::move(callbackDone))
        {
        }

        ~ProtothreadRequestReply()
        {
            auto entity = m_entity.lock();
            if (entity)
            {
                entity->cancelReply(m_correlationId);
            }
        }

        bool sendRequest(const hybrid_ptr<IRemoteEntity>& entity, const PeerId& peerId, const std::string& path, const StructBase& structBase)
        {
            assert(!m_waitForReply);
            if (m_waitForReply)
            {
                return false;
            }
            m_entity = entity;
            m_correlationId = entity.lock()->sendRequest(peerId, path, structBase,
                [this](PeerId peerId, Status status, const finalmq::StructBasePtr& structBase) {
                    m_status = status;
                    m_reply = structBase;
                    Run();
                });
            m_waitForReply = true;
            return (m_correlationId != CORRELATIONID_NONE);
        }
        bool sendRequest(const hybrid_ptr<IRemoteEntity>& entity, const PeerId& peerId, const std::string& path, IMessage::Metainfo&& metainfo, const StructBase& structBase)
        {
            assert(!m_waitForReply);
            if (m_waitForReply)
            {
                return false;
            }
            m_entity = entity;
            m_correlationId = entity.lock()->sendRequest(peerId, path, std::move(metainfo), structBase,
                [this](PeerId peerId, Status status, IMessage::Metainfo& metainfo, const StructBasePtr& structBase) {
                    m_status = status;
                    m_metainfo = std::move(metainfo);
                    m_reply = structBase;
                    Run();
                });
            m_waitForReply = true;
            return (m_correlationId != CORRELATIONID_NONE);
        }
        bool sendRequest(const hybrid_ptr<IRemoteEntity>& entity, const PeerId& peerId, const StructBase& structBase)
        {
            assert(!m_waitForReply);
            if (m_waitForReply)
            {
                return false;
            }
            m_entity = entity;
            m_correlationId = entity.lock()->sendRequest(peerId, structBase,
                [this](PeerId peerId, Status status, const finalmq::StructBasePtr& structBase) {
                    m_status = status;
                    m_reply = structBase;
                    Run();
                });
            m_waitForReply = true;
            return (m_correlationId != CORRELATIONID_NONE);
        }
        bool sendRequest(const hybrid_ptr<IRemoteEntity>& entity, const PeerId& peerId, IMessage::Metainfo&& metainfo, const StructBase& structBase)
        {
            assert(!m_waitForReply);
            if (m_waitForReply)
            {
                return false;
            }
            m_entity = entity;
            m_correlationId = entity.lock()->sendRequest(peerId, std::move(metainfo), structBase,
                [this](PeerId peerId, Status status, IMessage::Metainfo& metainfo, const StructBasePtr& structBase) {
                    m_status = status;
                    m_metainfo = std::move(metainfo);
                    m_reply = structBase;
                    Run();
                });
            m_waitForReply = true;
            return (m_correlationId != CORRELATIONID_NONE);
        }

        template<class R>
        ReplyResult<R> reply()
        {
            std::shared_ptr<R> r;
            bool typeOk = (!m_reply || m_reply->getStructInfo().getTypeName() == R::structInfo().getTypeName());
            if (m_status == Status::STATUS_OK && m_reply && typeOk)
            {
                r = std::static_pointer_cast<R>(m_reply);
            }
            if (!typeOk)
            {
                m_status = Status::STATUS_WRONG_REPLY_TYPE;
            }
            return { m_status, r, std::move(m_metainfo) };
        }

        Status getStatus() const
        {
            return m_status;
        }

        IMessage::Metainfo& getMetaInfo()
        {
            return m_metainfo;
        }

        virtual bool Run() override
        {
            bool result = false;
            if (m_callbackRun)
            {
                result = m_callbackRun(*this);
            }
            if (!result && m_callbackDone)
            {
                m_callbackDone();
            }
            return result;
        }

        void waitForReplyCalled()
        {
            m_waitForReply = false;
        }

    private:
        hybrid_ptr<IRemoteEntity> m_entity;
        finalmq::CorrelationId m_correlationId = finalmq::CORRELATIONID_NONE;
        Status m_status;
        finalmq::StructBasePtr m_reply;
        IMessage::Metainfo m_metainfo;
        bool m_waitForReply = false;

        FuncRun m_callbackRun;
        FuncCallbackThreadDone m_callbackDone;
    };

#define WAIT_FOR_REPLY(protothread) \
    protothread.waitForReplyCalled(); \
    PT_YIELD(protothread)


}   // namespace finalmq
