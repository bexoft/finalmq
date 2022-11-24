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

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;
using System.Runtime.CompilerServices;

namespace finalmq
{
    public abstract class ProtocolDelimiter : IProtocol
    {

        public ProtocolDelimiter(byte[] delimiter)
        {
            Debug.Assert(delimiter != null);
            Debug.Assert(delimiter.Length > 0);
            m_delimiter = delimiter;
            m_delimiterStart = delimiter[0];
            m_delimiterPrefix = new byte[delimiter.Length - 1];
        }
        ~ProtocolDelimiter()
        {
            if (m_connection != null)
            {
                m_connection.Disconnect();
            }
        }

        // IStreamConnectionCallback
        public IStreamConnectionCallback? Connected(IStreamConnection connection)
        {
            var callback = m_callback;
            if (callback != null)
            {
                callback.Connected();
            }
            return null;
        }
        public void Disconnected(IStreamConnection connection)
        {
            var callback = m_callback;
            if (callback != null)
            {
                callback.Disconnected();
            }
        }
        public void Received(IStreamConnection connection, byte[] buffer, int count)
        {
            if (m_sizeDelimiterPrefix > 0)
            {
                m_receiveBuffer = new byte[m_sizeDelimiterPrefix + count];
                Array.Copy(m_delimiterPrefix, 0, m_receiveBuffer, 0, m_sizeDelimiterPrefix);
                Array.Copy(buffer, 0, m_receiveBuffer, m_sizeDelimiterPrefix, count);
            }
            else
            {
                m_receiveBuffer = (byte[])buffer;
            }
            var callback = m_callback;
            int bytesReceived = count;
            m_bufferSize = m_sizeDelimiterPrefix + bytesReceived;
            int offsetEnd = m_bufferSize - (m_delimiter.Length - 1);
            for (int i = m_indexStartMessage; i < offsetEnd; ++i)
            {
                byte c = m_receiveBuffer[i];
                if (c == m_delimiterStart)
                {
                    bool match = true;
                    for (int j = 1; j < m_delimiter.Length; ++j)
                    {
                        if (m_receiveBuffer[i + j] != m_delimiter[j])
                        {
                            match = false;
                            break;
                        }
                    }
                    if (match)
                    {
                        IMessage message = new ProtocolMessage(0);
                        if (m_receiveBuffers.Count == 0)
                        {
                            int size = i - m_indexStartMessage;
                            Debug.Assert(size >= 0);
                            message.SetReceiveBuffer(m_receiveBuffer, m_indexStartMessage, size);
                        }
                        else
                        {
                            Debug.Assert(m_indexStartMessage == 0);
                            int sizeLast = i - m_indexStartMessage;
                            Debug.Assert(sizeLast >= 0);
                            m_receiveBuffersTotal += sizeLast;
                            Debug.Assert(m_receiveBuffersTotal >= 0);
                            byte[] receiveBufferHelper = new byte[m_receiveBuffersTotal];
                            int offset = 0;
                            for (int n = 0; n < m_receiveBuffers.Count; ++n)
                            {
                                ReceiveBufferStore receiveBufferStore = m_receiveBuffers[n];
                                int sizeCopy = receiveBufferStore.IndexEndMessage - receiveBufferStore.IndexStartMessage;
                                Array.Copy(receiveBufferStore.ReceiveBuffer, receiveBufferStore.IndexStartMessage, receiveBufferHelper, offset, sizeCopy);
                                offset += sizeCopy;
                            }
                            Array.Copy(m_receiveBuffer, 0, receiveBufferHelper, offset, sizeLast);
                            message.SetReceiveBuffer(receiveBufferHelper, 0, m_receiveBuffersTotal);
                            m_receiveBuffersTotal = 0;
                            m_receiveBuffers.Clear();
                        }
                        if (callback != null)
                        {
                            callback.Received(message);
                        }
                        i += m_delimiter.Length;
                        m_indexStartMessage = i;
                        --i;
                    }
                }
            }
            if (m_indexStartMessage == m_bufferSize)
            {
                m_receiveBuffer = null;
                m_bufferSize = 0;
                m_indexStartMessage = 0;
                m_sizeDelimiterPrefix = 0;
            }
            else
            {
                if (m_indexStartMessage < offsetEnd)
                {
                    int size = offsetEnd - m_indexStartMessage;

                    if (m_indexStartMessage > 0)
                    {
                        byte[] bufferStore = new byte[size];
                        Array.Copy(m_receiveBuffer, m_indexStartMessage, bufferStore, 0, size);
                        m_receiveBuffers.Add(new ReceiveBufferStore(bufferStore, 0, size));
                    }
                    else
                    {
                        m_receiveBuffers.Add(new ReceiveBufferStore((byte[])m_receiveBuffer.Clone(), m_indexStartMessage, offsetEnd));
                    }
                    m_receiveBuffersTotal += size;
                    m_indexStartMessage = offsetEnd;
                }
                m_sizeDelimiterPrefix = m_bufferSize - m_indexStartMessage;
                Debug.Assert(m_sizeDelimiterPrefix > 0);
                Debug.Assert(m_sizeDelimiterPrefix <= m_delimiterPrefix.Length);
                Array.Copy(m_receiveBuffer, m_indexStartMessage, m_delimiterPrefix, 0, m_sizeDelimiterPrefix);
                m_indexStartMessage = 0;
            }
        }
        byte[] m_delimiterPrefix;
        int m_sizeDelimiterPrefix = 0;

        // IProtocol
        public void SetCallback(IProtocolCallback callback)
        {
            m_callback = callback;
        }
        public void SetConnection(IStreamConnection connection)
        {
            m_connection = connection;
        }
        public IStreamConnection? Connection
        {
            get
            {
                return m_connection;
            }
        }
        public void Disconnect()
        {
            IStreamConnection? connection = m_connection;
            if (connection != null)
            {
                connection.Disconnect();
            }
        }

        public abstract uint ProtocolId { get; }

        public bool AreMessagesResendable
        {
            get
            {
                return true;
            }
        }
        public bool DoesSupportMetainfo
        {
            get
            {
                return false;
            }
        }
        public bool DoesSupportSession
        {
            get
            {
                return false;
            }
        }
        public bool NeedsReply
        {
            get
            {
                return false;
            }
        }
        public bool IsMultiConnectionSession
        {
            get
            {
                return false;
            }
        }
        public bool IsSendRequestByPoll
        {
            get
            {
                return false;
            }
        }
        public bool DoesSupportFileTransfer
        {
            get
            {
                return false;
            }
        }
        public FuncCreateMessage MessageFactory
        {
            get
            {
                int sizeDelimiter = m_delimiter.Length;
                uint protocolId = ProtocolId;
                return () =>
                {
                    return new ProtocolMessage(protocolId, 0, sizeDelimiter);
                };
            }
        }

        public void SendMessage(IMessage message)
        {
            if (!message.WasSent)
            {
                IList<BufferRef> buffers = message.GetAllSendBuffers();
                if (buffers.Count != 0 && m_delimiter.Length != 0)
                {
                    BufferRef buffer = buffers.Last<BufferRef>();
                    Debug.Assert(buffer.Length >= m_delimiter.Length);
                    Array.Copy(m_delimiter, 0, buffer.Buffer, buffer.Length - m_delimiter.Length, m_delimiter.Length);
                    message.PrepareMessageToSend();
                }
            }
            Debug.Assert(m_connection != null);
            m_connection.SendMessage(message);
        }
        public void MoveOldProtocolState(IProtocol protocolOld)
        {

        }
        public IMessage? PollReply(IList<IMessage>? messages = null)
        {
            return null;
        }
        public void Subscribe(IList<string> subscribtions)
        {
        }

        IProtocolCallback? m_callback = null;
        IStreamConnection? m_connection = null;

        readonly byte[] m_delimiter;

        readonly byte m_delimiterStart;
        int m_indexStartMessage = 0;
        byte[]? m_receiveBuffer = null;
        int m_bufferSize = 0;

        class ReceiveBufferStore
        {
            public ReceiveBufferStore(byte[] receiveBuffer, int indexStartMessage, int indexEndMessage)
            {
                m_receiveBuffer = receiveBuffer;
                m_indexStartMessage = indexStartMessage;
                m_indexEndMessage = indexEndMessage;
            }

            public byte[] ReceiveBuffer
            {
                get 
                { 
                    return m_receiveBuffer; 
                }
            }

            public int IndexStartMessage
            {
                get 
                { 
                    return m_indexStartMessage; 
                }
            }

            public int IndexEndMessage
            {
                get 
                { 
                    return m_indexEndMessage; 
                }
            }

            byte[] m_receiveBuffer;
            int m_indexStartMessage = -1;
            int m_indexEndMessage = -1;
        };

        readonly IList<ReceiveBufferStore> m_receiveBuffers = new List<ReceiveBufferStore>();
        int m_receiveBuffersTotal = 0;
    };


}
