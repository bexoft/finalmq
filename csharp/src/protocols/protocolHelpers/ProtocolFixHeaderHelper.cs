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
    class ProtocolFixHeaderHelper
    {
        public delegate int FuncGetPayloadSize(byte[] header);

        public ProtocolFixHeaderHelper(int sizeHeader, FuncGetPayloadSize funcGetPayloadSize)
        {
            m_funcGetPayloadSize = funcGetPayloadSize;
            m_header = new byte[sizeHeader];
        }

        public void Receive(byte[] buffer, int count, IList<IMessage> messages)
        {
            m_indexSource = 0;
            m_bytesToRead = count;
            m_messages = messages;
            while (m_bytesToRead > 0)
            {
                switch (m_state)
                {
                case State.WAITFORHEADER:
                    ReceiveHeader(buffer);
                    break;
                case State.WAITFORPAYLOAD:
                    ReceivePayload(buffer);
                    break;
                default:
                    Debug.Assert(false);
                    break;
                }
            }
            m_messages = null;
        }

        void ReceiveHeader(byte[] buffer)
        {
            Debug.Assert(m_state == State.WAITFORHEADER);
            Debug.Assert(m_sizeCurrent < m_header.Length);

            int sizeRead = m_header.Length - m_sizeCurrent;
            if (m_bytesToRead < sizeRead)
            {
                sizeRead = m_bytesToRead;
            }
            Array.Copy(buffer, m_indexSource, m_header, m_sizeCurrent, sizeRead);

            m_bytesToRead -= sizeRead;
            Debug.Assert(m_bytesToRead >= 0);
            m_indexSource += sizeRead;
            m_sizeCurrent += sizeRead;
            Debug.Assert(m_sizeCurrent <= m_header.Length);
            if (m_sizeCurrent == m_header.Length)
            {
                m_sizeCurrent = 0;
                Debug.Assert(m_funcGetPayloadSize != null);
                int sizePayload = m_funcGetPayloadSize(m_header);
                SetPayloadSize(sizePayload);
            }
        }

        void SetPayloadSize(int sizePayload)
        {
            Debug.Assert(m_state == State.WAITFORHEADER);
            Debug.Assert(sizePayload >= 0);
            m_sizePayload = sizePayload;
            m_message = new ProtocolMessage(0, m_header.Length);
            m_buffer = m_message.ResizeReceiveBuffer(m_header.Length + sizePayload);
            Array.Copy(m_header, 0, m_buffer.Buffer, 0, m_header.Length);
            if (sizePayload != 0)
            {
                m_state = State.WAITFORPAYLOAD;
            }
            else
            {
                HandlePayloadReceived();
            }
        }

        void ReceivePayload(byte[] buffer)
        {
            Debug.Assert(m_state == State.WAITFORPAYLOAD);
            Debug.Assert(m_sizeCurrent < m_sizePayload);

            int sizeRead = m_sizePayload - m_sizeCurrent;
            if (m_bytesToRead < sizeRead)
            {
                sizeRead = m_bytesToRead;
            }
            Debug.Assert(m_buffer != null);
            Array.Copy(buffer, m_indexSource, m_buffer.Buffer, m_header.Length + m_sizeCurrent, sizeRead);
            m_bytesToRead -= sizeRead;
            Debug.Assert(m_bytesToRead >= 0);
            m_sizeCurrent += sizeRead;
            Debug.Assert(m_sizeCurrent <= m_sizePayload);
            if (m_sizeCurrent == m_sizePayload)
            {
                m_sizeCurrent = 0;
                HandlePayloadReceived();
            }
        }

        void HandlePayloadReceived()
        {
            Debug.Assert(m_messages != null);
            Debug.Assert(m_message != null);
            m_messages.Add(m_message);
            ClearState();
        }

        void ClearState()
        {
            m_sizePayload = 0;
            m_message = null;
            m_buffer = null;
            m_sizeCurrent = 0;
            m_state = State.WAITFORHEADER;
        }

        enum State
        {
            WAITFORHEADER,
            WAITFORPAYLOAD
        };

        readonly byte[] m_header;
        State m_state = State.WAITFORHEADER;
        int m_sizeCurrent = 0;
        int m_indexSource = 0;
        int m_bytesToRead = 0;

        int m_sizePayload = 0;
        IMessage? m_message = null;
        BufferRef? m_buffer = null;

        IList<IMessage>? m_messages = null;
        readonly FuncGetPayloadSize m_funcGetPayloadSize;
    };


}
