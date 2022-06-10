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

using System.Collections.Generic;
    

namespace finalmq
{
    public interface Metainfo : IDictionary<string, string>
    {
    }

    public struct BufferRef
    {
        BufferRef(int index = 0, int size = 0)
        {
            this.index = index;
            this.size = size;
        }
        int index;
        int size;
    }


    public interface IMessage : IZeroCopyBuffer
    {
        
        // metainfo
        Metainfo getAllMetainfo();
        void AddMetainfo(string key, string value);
        string getMetainfo(string key);

        //// controlData
        //virtual Variant& getControlData() = 0;
        //virtual const Variant& getControlData() const = 0;

        //// echoData
        //virtual Variant& getEchoData() = 0;
        //virtual const Variant& getEchoData() const = 0;

        // for send
        void AddSendPayload(byte[] payload);
        void AddSendPayload(byte[] payload, int reserve = 0);
        byte[] AddSendPayload(int size, int reserve = 0);
        void DownsizeLastSendPayload(int newSize);

        // for receive
        BufferRef GetReceiveHeader();
        BufferRef GetReceivePayload();
        byte[] ResizeReceiveBuffer(int size);
        void SetHeaderSize(int sizeHeader);

        // for the framework
        IList<BufferRef> GetAllSendBuffers();
        int GetTotalSendBufferSize();
        IList<BufferRef> GetAllSendPayloads();
        int GetTotalSendPayloadSize();
        void MoveSendBuffers(IList<byte[]> payloadBuffers, IList<BufferRef> payloads);
        IList<string> GetSendPayloadBuffers();

        // for the protocol to add a header
        void AddSendHeader(byte[] header);
        byte[] AddSendHeader(int size);
        void DownsizeLastSendHeader(int newSize);

        // for the protocol to prepare the message for send
        void PrepareMessageToSend();

        // for the protocol to check which protocol created the message
        uint GetProtocolId();

        bool WasSent();

        void AddMessage(IMessage msg);
        IMessage GetMessage(int protocolId);
    };



}   // namespace finalmq
