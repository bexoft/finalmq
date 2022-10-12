// MIT License

// Copyright (c) 2020 bexoft GmbH (mail@bexoft.de)

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


using System.Diagnostics;

namespace finalmq {


    public class ProtocolMessage : IMessage
    {
        public ProtocolMessage(uint protocolId, int sizeHeader = 0, int sizeTrailer = 0)
        {
            m_sizeHeader = sizeHeader;
            m_sizeTrailer = sizeTrailer;
            m_protocolId = protocolId;
        }

        public BufferRef AddBuffer(int size, int reserve = 0)
        {
            Debug.Assert(!m_preparedToSend);
            if (size <= 0)
            {
                throw new System.ApplicationException();
            }

            if (m_sendBufferRefs == null || m_sendPayloadRefs == null)
            {
                Debug.Assert(m_sendBufferRefs == null);
                Debug.Assert(m_sendPayloadRefs == null);
                m_sendBufferRefs = new List<BufferRef>();
                m_sendPayloadRefs = new List<BufferRef>();
            }

            if (m_offset != -1)
            {
                Debug.Assert(m_sendBufferRefs.Count > 0);
                BufferRef bufLast = m_sendBufferRefs.Last<BufferRef>();
                int remaining = bufLast.Buffer.Length - m_offset - m_sizeTrailer;
                Debug.Assert(remaining >= 0);
                if (size <= remaining)
                {
                    Debug.Assert(m_sendPayloadRefs.Count > 0);
                    bufLast.Length += size;
                    m_sendPayloadRefs.Last<BufferRef>().Length += size;
                    int offset = m_offset;
                    m_offset += size;
                    m_sizeLastBlock = size;
                    m_sizeSendBufferTotal += size;
                    m_sizeSendPayloadTotal += size;
                    return new BufferRef(bufLast.Buffer, offset, size);
                }
            }

            if (reserve < size)
            {
                reserve = size;
            }

            int sizeHeader = 0;
            if (m_sendPayloadRefs.Count == 0)
            {
                Debug.Assert(m_sizeSendPayloadTotal == 0);
                sizeHeader = m_sizeHeader;
                m_sizeSendBufferTotal += m_sizeHeader + m_sizeTrailer;
            }
            else
            {
                // remove the trailer of the last payload
                BufferRef lastRef = m_sendBufferRefs.Last<BufferRef>();
                lastRef.Length -= m_sizeTrailer;
                Debug.Assert(lastRef.Length >= 0);
                if (lastRef.Length == 0)
                {
                    m_sendBufferRefs.RemoveAt(m_sendBufferRefs.Count - 1);
                    m_sendPayloadRefs.RemoveAt(m_sendPayloadRefs.Count - 1);
                }
            }

            m_offset = sizeHeader + reserve;
            m_sizeLastBlock = reserve;

            m_sizeSendBufferTotal += reserve;
            m_sizeSendPayloadTotal += reserve;
            int sizeBuffer = sizeHeader + reserve + m_sizeTrailer;
            byte[] buffer = new byte[sizeBuffer];
            m_sendBufferRefs.Add(new BufferRef(buffer, 0, sizeBuffer));
            BufferRef bufferRefPayload = new BufferRef(buffer, sizeHeader, reserve);
            m_sendPayloadRefs.Add(bufferRefPayload);
            if (size < reserve)
            {
                DownsizeLastBuffer(size);
            }
            return bufferRefPayload;
        }
        public void DownsizeLastBuffer(int newSize)
        {
            Debug.Assert(!m_preparedToSend);

            if (m_sendBufferRefs == null || m_sendPayloadRefs == null)
            {
                Debug.Assert(m_sendBufferRefs == null);
                Debug.Assert(m_sendPayloadRefs == null);
                m_sendBufferRefs = new List<BufferRef>();
                m_sendPayloadRefs = new List<BufferRef>();
            }

            if (m_sendPayloadRefs.Count == 0 && newSize == 0)
            {
                return;
            }

            Debug.Assert(newSize <= m_sizeLastBlock);
            Debug.Assert(m_offset != -1 || newSize == 0);

            int diff = m_sizeLastBlock - newSize;
            Debug.Assert(diff >= 0);

            if (diff == 0)
            {
                return;
            }

            BufferRef bufLast = m_sendBufferRefs.Last<BufferRef>();
            bufLast.Length -= diff;
            m_sizeSendBufferTotal -= diff;
            Debug.Assert(bufLast.Length >= 0);
            Debug.Assert(m_sizeSendBufferTotal >= 0);
            m_sizeSendPayloadTotal -= diff;
            m_offset -= diff;
            m_sizeLastBlock = newSize;
            Debug.Assert(m_sizeSendPayloadTotal >= 0);
            Debug.Assert(m_offset >= 0);

            if (bufLast.Length > 0)
            {
                BufferRef bufLastPayload = m_sendPayloadRefs.Last<BufferRef>();
                bufLastPayload.Length -= diff;
                Debug.Assert(bufLastPayload.Length >= 0);
            }
            else
            {
                m_sendBufferRefs.RemoveAt(m_sendBufferRefs.Count - 1);
                m_sendPayloadRefs.RemoveAt(m_sendPayloadRefs.Count - 1);
                m_offset = -1;
            }
        }

        // metainfo
        public Metainfo GetAllMetainfo()
        {
            throw new System.NotImplementedException();
        }

        public void AddMetainfo(string key, string value)
        {
            throw new System.NotImplementedException();
        }
        public string GetMetainfo(string key)
        {
            throw new System.NotImplementedException();
        }

        // controlData
        //public Variant GetControlData()
        //{
        //    throw new System.NotImplementedException();
        //}
        //public Variant GetControlData()
        //{
        //    throw new System.NotImplementedException();
        //}

        // echoData
        //public Variant GetEchoData()
        //{
        //    throw new System.NotImplementedException();
        //}
        //public Variant GetEchoData() const
        //{
        //    throw new System.NotImplementedException();
        //}

        // for send
        public void AddSendPayload(byte[] payload)
        {
            AddSendPayload(payload, 0);
        }
        public void AddSendPayload(byte[] payload, int reserve = 0)
        {
            BufferRef bufferRef = AddSendPayload(payload.Length, reserve);
            payload.CopyTo(bufferRef.Buffer, bufferRef.Offset);
        }
        public BufferRef AddSendPayload(int size, int reserve = 0) => AddBuffer(size, reserve);

        public void DownsizeLastSendPayload(int newSize) => DownsizeLastBuffer(newSize);

        // for receive
        public BufferRef GetReceiveHeader()
        {
            if (m_receiveBuffer == null)
            {
                throw new System.InvalidOperationException();
            }
            return new BufferRef(m_receiveBuffer, 0, m_sizeHeader);
        }
        public BufferRef GetReceivePayload()
        {
            if (m_receiveBuffer == null)
            {
                throw new System.InvalidOperationException();
            }
            return new BufferRef(m_receiveBuffer, m_sizeHeader, m_sizeReceiveBuffer - m_sizeHeader);
        }
        public BufferRef ResizeReceiveBuffer(int size)
        {
            if (m_receiveBuffer == null || size > m_receiveBuffer.Length)
            {
                m_receiveBuffer = new byte[size];
            }
            m_sizeReceiveBuffer = size;
            return new BufferRef(m_receiveBuffer, 0, size);
        }
        public void SetHeaderSize(int sizeHeader)
        {
            m_sizeHeader = sizeHeader;
        }

        // for the framework
        public IList<BufferRef> GetAllSendBuffers()
        {
            if (m_sendBufferRefs == null)
            {
                throw new System.InvalidOperationException();
            }
            return m_sendBufferRefs;
        }
        public int GetTotalSendBufferSize()
        {
            return m_sizeSendBufferTotal;
        }
        public IList<BufferRef> GetAllSendPayloads()
        {
            if (m_sendPayloadRefs == null)
            {
                throw new System.InvalidOperationException();
            }
            return m_sendPayloadRefs;
        }
        public int GetTotalSendPayloadSize()
        {
            return m_sizeSendPayloadTotal;
        }
        public void MoveSendBuffers(IList<BufferRef> sendPayloadRefs)
        {
            if (m_sendBufferRefs == null || m_sendPayloadRefs == null)
            {
                Debug.Assert(m_sendBufferRefs == null);
                Debug.Assert(m_sendPayloadRefs == null);
                m_sendBufferRefs = new List<BufferRef>();
                m_sendPayloadRefs = new List<BufferRef>();
            }
            foreach (var payloadRef in sendPayloadRefs)
            {
                m_sendBufferRefs.Add(new BufferRef(payloadRef.Buffer, 0, payloadRef.Offset + payloadRef.Length));
                m_sendPayloadRefs.Add(payloadRef);
                m_offset = payloadRef.Offset + payloadRef.Length;
                m_sizeLastBlock = payloadRef.Length;
                m_sizeSendBufferTotal += payloadRef.Offset + payloadRef.Length;
                m_sizeSendPayloadTotal += payloadRef.Length;
            }
        }

        // for the protocol to add a header
        public void AddSendHeader(byte[] header)
        {
            BufferRef headerRef = AddSendHeader(header.Length);
            header.CopyTo(headerRef.Buffer, headerRef.Offset);
        }

        public BufferRef AddSendHeader(int size)
        {
            Debug.Assert(!m_preparedToSend);

            if (m_sendBufferRefs == null || m_sendPayloadRefs == null)
            {
                Debug.Assert(m_sendBufferRefs == null);
                Debug.Assert(m_sendPayloadRefs == null);
                m_sendBufferRefs = new List<BufferRef>();
                m_sendPayloadRefs = new List<BufferRef>();
            }

            int indexSendBufferRefsPayloadBegin = m_sendBufferRefs.Count - m_sendPayloadRefs.Count;
            BufferRef bufferRef = new BufferRef(new byte[size], 0, size);
            m_sendBufferRefs.Insert(indexSendBufferRefsPayloadBegin, bufferRef);
            m_sizeSendBufferTotal += size;
            return bufferRef;
        }
        public void DownsizeLastSendHeader(int newSize)
        {
            Debug.Assert(!m_preparedToSend);
            Debug.Assert(m_sendBufferRefs != null);
            Debug.Assert(m_sendPayloadRefs != null);
            int indexLast = m_sendBufferRefs.Count - m_sendPayloadRefs.Count;
            --indexLast;
            Debug.Assert(indexLast >= 0);
            BufferRef bufferRef = m_sendBufferRefs[indexLast];
            Debug.Assert(newSize <= bufferRef.Length);
            m_sizeSendBufferTotal += (newSize - bufferRef.Length);
            bufferRef.Length = newSize;
            if (newSize == 0)
            {
                m_sendBufferRefs.RemoveAt(indexLast);
            }
        }

        // for the protocol to prepare the message for send
        public void PrepareMessageToSend()
        {
            if (m_sendBufferRefs == null)
            {
                return;
            }
            m_preparedToSend = true;
            IList<BufferRef> itemsToRemove = new List<BufferRef>();
            foreach (var sendBufferRef in m_sendBufferRefs)
            {
                if (sendBufferRef.Length == 0)
                {
                    itemsToRemove.Add(sendBufferRef);
                }
            }
            foreach (var item in itemsToRemove)
            {
                m_sendBufferRefs.Remove(item);
            }
        }

        // for the protocol to check if which protocol created the message
        public uint ProtocolId 
        {
            get => m_protocolId;
        }
        public bool WasSent()
        {
            return m_preparedToSend;
        }

        public void AddMessage(IMessage msg)
        {
            if (m_messages == null)
            {
                m_messages = new Dictionary<uint, IMessage>();
            }
            m_messages[msg.ProtocolId] = msg;
        }
        public IMessage? GetMessage(uint protocolId)
        {
            if (m_messages == null)
            {
                return null;
            }
            IMessage? message;
            m_messages.TryGetValue(protocolId, out message);
            return message;
        }


        //Metainfo m_metainfo;
        //Variant m_controlData;
        //Variant m_echoData;

        // send
        IList<BufferRef>? m_sendBufferRefs = null;
        int m_offset = -1;
        int m_sizeLastBlock = 0;
        int m_sizeSendBufferTotal = 0;
        IList<BufferRef>? m_sendPayloadRefs = null;
        int m_sizeSendPayloadTotal = 0;

        // receive
        byte[]? m_receiveBuffer = null;
        int m_sizeReceiveBuffer = 0;

        int m_sizeHeader = 0;
        int m_sizeTrailer = 0;

        bool m_preparedToSend = false;
        readonly uint m_protocolId = 0;

        IDictionary<uint, IMessage>? m_messages = null;
    }


}   // namespace finalmq
