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

namespace finalmq
{
    public interface Metainfo : IDictionary<string, string>
    {
    }

    public class BufferRef
    {
        public BufferRef(byte[] buffer, int offset = 0, int length = 0)
        {
            Debug.Assert(offset + length <= buffer.Length);
            m_buffer = buffer;
            m_offset = offset;
            m_length = length;
        }
        public void Set(byte[] buffer, int offset = 0, int length = 0)
        {
            Debug.Assert(offset + length <= buffer.Length);
            m_buffer = buffer;
            m_offset = offset;
            m_length = length;
        }
        public byte this[int index] 
        {
            get
            {
                return m_buffer[m_offset + index];
            }
            set
            {
                m_buffer[m_offset + index] = value;
            }
        }
        public byte[] Buffer
        {
            get
            {
                return this.m_buffer;
            }
        }
        public int Offset
        {
            get
            {
                return this.m_offset;
            }
        }
        public int Length
        {
            get
            {
                return this.m_length;
            }
            set
            {
                Debug.Assert(this.Offset + value <= this.Buffer.Length);
                this.m_length = value;
            }
        }
        public static void Copy(BufferRef source, BufferRef destination, int length)
        {
            Debug.Assert(source.Offset + length <= source.Buffer.Length);
            Debug.Assert(destination.Offset + length <= destination.Buffer.Length);
            Debug.Assert(length <= destination.Length);
            Array.Copy(source.Buffer, source.Offset, destination.Buffer, destination.Offset, length);
        }
        public void CopyTo(BufferRef destination, int length)
        {
            Debug.Assert(this.Offset + length <= this.Buffer.Length);
            Debug.Assert(destination.Offset + length <= destination.Buffer.Length);
            Debug.Assert(length <= destination.Length);
            Array.Copy(this.Buffer, this.Offset, destination.Buffer, destination.Offset, length);
        }
        public void CopyTo(BufferRef destination)
        {
            CopyTo(destination, this.Length);
        }
        public void CopyToEnd(BufferRef destination)
        {
            int length = this.Length;
            Debug.Assert(destination.Offset + destination.Length + length <= destination.Buffer.Length);
            destination.Length += this.Length;
            CopyTo(destination, length);
        }
        public void AddOffset(int delta)
        {
            this.m_offset += delta;
            this.m_length -= delta;
            Debug.Assert(this.m_length >= 0);
            Debug.Assert(this.m_offset + this.m_length <= m_buffer.Length);
        }
        public void Clear()
        {
            this.m_length = 0;
        }
        private byte[] m_buffer;
        private int m_offset;
        private int m_length;
    }


    public interface IMessage : IZeroCopyBuffer
    {
        
        // metainfo
        Metainfo GetAllMetainfo();
        void AddMetainfo(string key, string value);
        string GetMetainfo(string key);

        //// controlData
        //virtual Variant& getControlData() = 0;
        //virtual const Variant& getControlData() const = 0;

        //// echoData
        //virtual Variant& getEchoData() = 0;
        //virtual const Variant& getEchoData() const = 0;

        // for send
        void AddSendPayload(byte[] payload);
        void AddSendPayload(byte[] payload, int reserve = 0);
        BufferRef AddSendPayload(int size, int reserve = 0);
        void DownsizeLastSendPayload(int newSize);

        // for receive
        BufferRef GetReceiveHeader();
        BufferRef GetReceivePayload();
        BufferRef ResizeReceiveBuffer(int size);
        void SetReceiveBuffer(byte[] buffer, int offset, int size);
        void SetHeaderSize(int sizeHeader);

        // for the framework
        IList<BufferRef> GetAllSendBuffers();
        int GetTotalSendBufferSize();
        IList<BufferRef> GetAllSendPayloads();
        int GetTotalSendPayloadSize();
        void MoveSendBuffers(IList<BufferRef> payloads);

        // for the protocol to add a header
        void AddSendHeader(byte[] header);
        BufferRef AddSendHeader(int size);
        void DownsizeLastSendHeader(int newSize);

        // for the protocol to prepare the message for send
        void PrepareMessageToSend();

        // for the protocol to check which protocol created the message
        uint ProtocolId { get; }

        bool WasSent { get; }

        void AddMessage(IMessage msg);
        IMessage? GetMessage(uint protocolId);
    };



}   // namespace finalmq
