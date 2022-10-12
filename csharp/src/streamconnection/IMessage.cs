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

    

namespace finalmq
{
    public interface Metainfo : IDictionary<string, string>
    {
    }

    public class BufferRef
    {
        public BufferRef(byte[] buffer, int offset = 0, int length = 0)
        {
            m_buffer = buffer;
            m_offset = offset;
            m_length = length;
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
                this.m_length = value;
            }
        }
        private readonly byte[] m_buffer;
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

        bool WasSent();

        void AddMessage(IMessage msg);
        IMessage? GetMessage(uint protocolId);
    };



}   // namespace finalmq
