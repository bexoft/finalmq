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
    class ProtocolHeaderBinarySize : IProtocol
    {
        public static readonly uint PROTOCOL_ID = 2;
        public static readonly string PROTOCOL_NAME = "headersize";

        private static readonly int HEADERSIZE = 4;

        public ProtocolHeaderBinarySize()
        {
            m_headerHelper = new ProtocolFixHeaderHelper(HEADERSIZE, (BufferRef header) => {
                    Debug.Assert(header.Length == 4);
                    uint value = 0;
                    for (int i = 0; i < 4; ++i)
                    {
                        uint a = header[i];
                        value |= (a & 0xff) << (8 * i);
                    }
                    int sizePayload = (int)value;
                    Debug.Assert(sizePayload >= 0);
                    return sizePayload;
                });
        }

        ~ProtocolHeaderBinarySize()
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
            IList<IMessage> messages = new List<IMessage>();
            m_headerHelper.Receive(buffer, count, messages);
            var callback = m_callback;
            if (callback != null)
            {
                foreach (var message in messages)
                {
                    callback.Received(message);
                }
            }
        }

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
        public uint ProtocolId 
        { 
            get
            {
                return PROTOCOL_ID;
            }
        }
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
                return () =>
                    {
                        return new ProtocolMessage(PROTOCOL_ID, HEADERSIZE);
                    };
            }
        }
        public void SendMessage(IMessage message)
        {
            if (message == null)
            {
                return;
            }
            if (!message.WasSent)
            {
                int sizePayload = message.GetTotalSendPayloadSize();
                Debug.Assert(sizePayload >= 0);
                IList<BufferRef> buffers = message.GetAllSendBuffers();
                Debug.Assert(buffers.Count != 0);
                Debug.Assert(buffers[0].Length >= HEADERSIZE);
                byte[] header = buffers[0].Buffer;
                uint value = (uint)sizePayload;
                for (int i = 0; i < HEADERSIZE; ++i)
                {
                    header[i] = (byte)((value >> (i * 8)) & 0xff);
                }
                message.PrepareMessageToSend();
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
        ProtocolFixHeaderHelper m_headerHelper;
    };


    //---------------------------------------
    // register factory
    //---------------------------------------
    class RegisterProtocolHeaderBinarySizeFactory
    {
#pragma warning disable CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        [ModuleInitializer]
#pragma warning restore CA2255 // Attribut "ModuleInitializer" nicht in Bibliotheken verwenden
        internal static void Register()
        {
            ProtocolRegistry.Instance.RegisterProtocolFactory(ProtocolHeaderBinarySize.PROTOCOL_NAME, ProtocolHeaderBinarySize.PROTOCOL_ID, () => { return new ProtocolHeaderBinarySize(); } );
        }
    }

}
