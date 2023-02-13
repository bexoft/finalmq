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

using System.Diagnostics;

namespace finalmq {

    public class RequestContext
    {
        internal RequestContext(PeerManager sessionIdEntityIdToPeerId, EntityId entityIdSrc, ReceiveData receiveData)
        {
            m_peerManager = sessionIdEntityIdToPeerId;
            m_session = receiveData.Session;
            m_virtualSessionId = receiveData.VirtualSessionId;
            m_entityIdDest = receiveData.Header.srcid;
            m_entityIdSrc = entityIdSrc;
            m_correlationId = receiveData.Header.corrid;
            m_replySent = false;
            m_metainfo = receiveData.Message.AllMetainfo;
            m_echoData = receiveData.Message.EchoData;
            m_delayedReply = false;
        }

        ~RequestContext()
        {
            if (!m_replySent)
            {
                Reply(Status.STATUS_NO_REPLY);
            }
        }
        
        public PeerId PeerId
        {
            get
            {
                if (m_peerId == IRemoteEntity.PEERID_INVALID)
                {
                    // get peer
                    m_peerId = m_peerManager.GetPeerId(m_session.SessionId, m_virtualSessionId, m_entityIdDest, "");
                }
                return m_peerId;
            }
        }

        public void Reply(StructBase structBase, Metainfo? metainfo = null)
        {
            if (!m_replySent)
            {
                Header header = new Header(m_entityIdDest, "", m_entityIdSrc, MsgMode.MSG_REPLY, Status.STATUS_OK, "", structBase.GetType().FullName!, m_correlationId, new List<string>() );
                RemoteEntityFormatRegistry.Instance.Send(m_session.Session, m_virtualSessionId, header, m_echoData, structBase, metainfo);
                m_replySent = true;
            }
        }

        public void Reply(Variant controlData, Metainfo? metainfo = null)
        {
            if (!m_replySent)
            {
                Header header = new Header( m_entityIdDest, "", m_entityIdSrc, MsgMode.MSG_REPLY, Status.STATUS_OK, "" , "", m_correlationId, new List<string>() );
                RemoteEntityFormatRegistry.Instance.Send(m_session.Session, m_virtualSessionId, header, m_echoData, null, metainfo, controlData);
                m_replySent = true;
            }
        }

        public void Reply(Status status)
        {
            if (!m_replySent)
            {
                Header header = new Header( m_entityIdDest, "", m_entityIdSrc, MsgMode.MSG_REPLY, status, "", "", m_correlationId, new List<string>() );
                RemoteEntityFormatRegistry.Instance.Send(m_session.Session, m_virtualSessionId, header, m_echoData);
                m_replySent = true;
            }
        }

        public void ReplyFile(string filename, Metainfo? metainfo = null)
        {
            FileTransferReply.ReplyFile(this, filename, metainfo);
        }

        public void ReplyMemory(byte[] buffer, int offset, int size, Metainfo? metainfo = null)
        {
            RawBytes replyBytes = new RawBytes();
            if (offset == 0 && size == buffer.Length)
            {
                replyBytes.data = buffer;
            }
            else
            {
                replyBytes.data = new byte[size];
                Array.Copy(buffer, offset, replyBytes.data, 0, size);
            }
            Reply(replyBytes, metainfo);
        }

        public CorrelationId CorrelationId
        {
            get
            {
                return m_correlationId;
            }
        }

        public string? GetMetainfo(string key)
        {
            m_metainfo.TryGetValue(key, out string? value);
            return value;
        }

        public Metainfo AllMetainfo
        {
            get
            {
                return m_metainfo;
            }
        }

        public bool DoesSupportFileTransfer
        {
            get
            {
                return m_session.DoesSupportFileTransfer;
            }
        }

        public string VirtualSessionId
        {
            get
            {
                return m_virtualSessionId;
            }
        }

        public SessionInfo Session
        {
            get
            {
                return m_session;
            }
        }
        public EntityId entityId
        {
            get
            {
                return m_entityIdDest;
            }
        }

        public RequestContext DelayReply()
        {
            m_delayedReply = true;
            return this;
        }

        public bool IsDelayedReply
        {
            get
            {
                return m_delayedReply;
            }
        }

        PeerManager m_peerManager;
        SessionInfo m_session;
        string m_virtualSessionId;
        EntityId m_entityIdDest = IRemoteEntity.ENTITYID_INVALID;
        EntityId m_entityIdSrc = IRemoteEntity.ENTITYID_INVALID;
        CorrelationId m_correlationId = IRemoteEntity.CORRELATIONID_NONE;
        PeerId m_peerId = IRemoteEntity.PEERID_INVALID;
        bool m_replySent;
        Metainfo m_metainfo;
        Variant m_echoData;
        bool m_delayedReply;
    }

}   // namespace finalmq
