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
        public delegate int FuncGetPayloadSize(BufferRef header);

        public ProtocolFixHeaderHelper(int sizeHeader, FuncGetPayloadSize funcGetPayloadSize)
        {
            m_funcGetPayloadSize = funcGetPayloadSize;
            m_sizeHeader = sizeHeader;
            m_headerBuffer = new byte[sizeHeader];
            m_header = new BufferRef(m_headerBuffer);
        }

        public void Receive(byte[] buffer, int count, IList<IMessage> messages)
        {
            BufferRef receiveBuffer = new BufferRef(buffer, 0, count);
            m_messages = messages;
            while (receiveBuffer.Length > 0)
            {
                switch (m_state)
                {
                    case State.STARTHEADER:
                        StartHeader(receiveBuffer);
                        break;
                    case State.WAITFORHEADER:
                        ReceiveHeader(receiveBuffer);
                        break;
                    case State.WAITFORPAYLOAD:
                        ReceivePayload(receiveBuffer);
                        break;
                    default:
                        Debug.Assert(false);
                        break;
                }
            }
            m_messages = null;
        }

        void StartHeader(BufferRef receiveBuffer)
        {
            Debug.Assert(m_state == State.STARTHEADER);

            if (receiveBuffer.Length < m_sizeHeader)
            {
                m_header.Set(m_headerBuffer, 0, m_sizeHeader);
                int sizeRead = receiveBuffer.Length;
                receiveBuffer.CopyTo(m_header, sizeRead);
                receiveBuffer.AddOffset(sizeRead);
                m_header.AddOffset(sizeRead);
                m_state = State.WAITFORHEADER;
            }
            else
            {
                m_header.Set(receiveBuffer.Buffer, receiveBuffer.Offset, m_sizeHeader);
                receiveBuffer.AddOffset(m_sizeHeader);
                Debug.Assert(m_funcGetPayloadSize != null);
                int sizePayload = m_funcGetPayloadSize(m_header);
                SetPayloadSize(receiveBuffer, sizePayload);
            }
        }

        void ReceiveHeader(BufferRef receiveBuffer)
        {
            Debug.Assert(m_state == State.WAITFORHEADER);
            Debug.Assert(m_header != null);

            int sizeRead = m_header.Length;
            if (receiveBuffer.Length < sizeRead)
            {
                sizeRead = receiveBuffer.Length;
            }
            receiveBuffer.CopyTo(m_header, sizeRead);
            receiveBuffer.AddOffset(sizeRead);
            m_header.AddOffset(sizeRead);

            if (m_header.Length == 0)
            {
                m_header.AddOffset(-m_sizeHeader);
                Debug.Assert(m_funcGetPayloadSize != null);
                int sizePayload = m_funcGetPayloadSize(m_header);
                SetPayloadSize(receiveBuffer, sizePayload);
            }
        }

        void SetPayloadSize(BufferRef receiveBuffer, int sizePayload)
        {
            Debug.Assert(m_state == State.STARTHEADER || m_state == State.WAITFORHEADER);
            Debug.Assert(sizePayload >= 0);
            Debug.Assert(m_header != null);
            m_sizePayload = sizePayload;
            m_message = new ProtocolMessage(0, m_sizeHeader);
            if ((m_state == State.STARTHEADER) && (m_sizePayload <= receiveBuffer.Length))
            {
                m_message.SetReceiveBuffer(receiveBuffer.Buffer, receiveBuffer.Offset - m_sizeHeader, m_sizeHeader + sizePayload);
                receiveBuffer.AddOffset(sizePayload);
                HandlePayloadReceived();
            }
            else
            {
                m_buffer = m_message.ResizeReceiveBuffer(m_sizeHeader + sizePayload);
                m_header.CopyTo(m_buffer, m_sizeHeader);
                m_buffer.AddOffset(m_sizeHeader);
                if (sizePayload != 0)
                {
                    m_state = State.WAITFORPAYLOAD;
                }
                else
                {
                    HandlePayloadReceived();
                }
            }
        }

        void ReceivePayload(BufferRef receiveBuffer)
        {
            Debug.Assert(m_state == State.WAITFORPAYLOAD);
            Debug.Assert(m_buffer != null);

            int sizeRead = m_buffer.Length;
            if (receiveBuffer.Length < sizeRead)
            {
                sizeRead = receiveBuffer.Length;
            }

            receiveBuffer.CopyTo(m_buffer, sizeRead);
            receiveBuffer.AddOffset(sizeRead);
            m_buffer.AddOffset(sizeRead);
            if (m_buffer.Length == 0)
            {
                m_buffer.AddOffset(-m_sizePayload);
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
            m_state = State.STARTHEADER;
        }

        enum State
        {
            STARTHEADER,
            WAITFORHEADER,
            WAITFORPAYLOAD
        };

        readonly byte[] m_headerBuffer;
        readonly BufferRef m_header;
        readonly int m_sizeHeader;
        int m_sizePayload = 0;
        State m_state = State.STARTHEADER;

        IMessage? m_message = null;
        BufferRef? m_buffer = null;

        IList<IMessage>? m_messages = null;
        readonly FuncGetPayloadSize m_funcGetPayloadSize;
    };

}
