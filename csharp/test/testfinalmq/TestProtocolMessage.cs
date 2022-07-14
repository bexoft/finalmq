using System;
using System.Threading;
using System.Security.Cryptography.X509Certificates;
using System.Net.Sockets;
using System.Diagnostics;

using Xunit;
using Moq;

using finalmq;

namespace testfinalmq
{




    [Collection("TestCollectionSocket")]
    public class TestProtocolMessage : IDisposable
    {
        readonly static int PROTOCOL_ID = 123;
        readonly static int SIZE_HEADER = 12;
        readonly static int SIZE_TRAILER = 8;

        public TestProtocolMessage()
        {
        }

        public void Dispose()
        {
        }

        [Fact]
        public void TestAddSendPayloadMultiple()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
            BufferRef buffer1 = message.AddSendPayload(4);
            BufferRef buffer2 = message.AddSendPayload(4);
            BufferRef buffer3 = message.AddSendPayload(4);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 3);
            Debug.Assert(sendPayloads.Count == 3);
            Debug.Assert(totalSendBufferSize == 12 + SIZE_HEADER + SIZE_TRAILER);
            Debug.Assert(totalSendPayloadSize == 12);

            Debug.Assert(sendBuffers[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendBuffers[0].Offset == buffer1.Offset - SIZE_HEADER);
            Debug.Assert(sendBuffers[0].Length == SIZE_HEADER + 4);

            Debug.Assert(sendBuffers[1].Buffer == buffer2.Buffer);
            Debug.Assert(sendBuffers[1].Offset == buffer2.Offset);
            Debug.Assert(sendBuffers[1].Length == 4);

            Debug.Assert(sendBuffers[2].Buffer == buffer3.Buffer);
            Debug.Assert(sendBuffers[2].Offset == buffer3.Offset);
            Debug.Assert(sendBuffers[2].Length == 4 + SIZE_TRAILER);

            Debug.Assert(sendPayloads[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendPayloads[0].Offset == buffer1.Offset);
            Debug.Assert(sendPayloads[0].Length == 4);

            Debug.Assert(sendPayloads[1].Buffer == buffer2.Buffer);
            Debug.Assert(sendPayloads[1].Offset == buffer2.Offset);
            Debug.Assert(sendPayloads[1].Length == 4);

            Debug.Assert(sendPayloads[2].Buffer == buffer3.Buffer);
            Debug.Assert(sendPayloads[2].Offset == buffer3.Offset);
            Debug.Assert(sendPayloads[2].Length == 4);
        }


        [Fact]
        public void TestAddSendPayloadMultipleWithUnchangedDownsize()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
            BufferRef buffer1 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(4);
            BufferRef buffer2 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(4);
            BufferRef buffer3 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(4);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 3);
            Debug.Assert(sendPayloads.Count == 3);
            Debug.Assert(totalSendBufferSize == 12 + SIZE_HEADER + SIZE_TRAILER);
            Debug.Assert(totalSendPayloadSize == 12);

            Debug.Assert(sendBuffers[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendBuffers[0].Offset == buffer1.Offset - SIZE_HEADER);
            Debug.Assert(sendBuffers[0].Length == SIZE_HEADER + 4);

            Debug.Assert(sendBuffers[1].Buffer == buffer2.Buffer);
            Debug.Assert(sendBuffers[1].Offset == buffer2.Offset);
            Debug.Assert(sendBuffers[1].Length == 4);

            Debug.Assert(sendBuffers[2].Buffer == buffer3.Buffer);
            Debug.Assert(sendBuffers[2].Offset == buffer3.Offset);
            Debug.Assert(sendBuffers[2].Length == 4 + SIZE_TRAILER);

            Debug.Assert(sendPayloads[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendPayloads[0].Offset == buffer1.Offset);
            Debug.Assert(sendPayloads[0].Length == 4);

            Debug.Assert(sendPayloads[1].Buffer == buffer2.Buffer);
            Debug.Assert(sendPayloads[1].Offset == buffer2.Offset);
            Debug.Assert(sendPayloads[1].Length == 4);

            Debug.Assert(sendPayloads[2].Buffer == buffer3.Buffer);
            Debug.Assert(sendPayloads[2].Offset == buffer3.Offset);
            Debug.Assert(sendPayloads[2].Length == 4);
        }

        [Fact]
        public void TestAddSendPayloadMultipleWithChangedDownsize()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
            BufferRef buffer1 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(3);
            BufferRef buffer2 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(3);
            BufferRef buffer3 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(3);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 3);
            Debug.Assert(sendPayloads.Count == 3);
            Debug.Assert(totalSendBufferSize == 9 + SIZE_HEADER + SIZE_TRAILER);
            Debug.Assert(totalSendPayloadSize == 9);

            Debug.Assert(sendBuffers[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendBuffers[0].Offset == buffer1.Offset - SIZE_HEADER);
            Debug.Assert(sendBuffers[0].Length == SIZE_HEADER + 3);

            Debug.Assert(sendBuffers[1].Buffer == buffer2.Buffer);
            Debug.Assert(sendBuffers[1].Offset == buffer2.Offset);
            Debug.Assert(sendBuffers[1].Length == 3);

            Debug.Assert(sendBuffers[2].Buffer == buffer3.Buffer);
            Debug.Assert(sendBuffers[2].Offset == buffer3.Offset);
            Debug.Assert(sendBuffers[2].Length == 3 + SIZE_TRAILER);

            Debug.Assert(sendPayloads[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendPayloads[0].Offset == buffer1.Offset);
            Debug.Assert(sendPayloads[0].Length == 3);

            Debug.Assert(sendPayloads[1].Buffer == buffer2.Buffer);
            Debug.Assert(sendPayloads[1].Offset == buffer2.Offset);
            Debug.Assert(sendPayloads[1].Length == 3);

            Debug.Assert(sendPayloads[2].Buffer == buffer3.Buffer);
            Debug.Assert(sendPayloads[2].Offset == buffer3.Offset);
            Debug.Assert(sendPayloads[2].Length == 3);
        }


        [Fact]
        public void TestAddSendPayloadMultipleWithMultipleDownsize()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
            BufferRef buffer1 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(3);
            message.DownsizeLastSendPayload(1);
            message.DownsizeLastSendPayload(0);
            message.DownsizeLastSendPayload(0);
            BufferRef buffer2 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(1);
            message.DownsizeLastSendPayload(0);
            message.DownsizeLastSendPayload(0);
            BufferRef buffer3 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(1);
            message.DownsizeLastSendPayload(0);
            message.DownsizeLastSendPayload(0);

            Debug.Assert(buffer2.Buffer == buffer1.Buffer);
            Debug.Assert(buffer2.Offset == buffer1.Offset);
            Debug.Assert(buffer3.Buffer == buffer1.Buffer);
            Debug.Assert(buffer3.Offset == buffer1.Offset);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 1);
            Debug.Assert(sendPayloads.Count == 1);
            Debug.Assert(totalSendBufferSize == 0 + SIZE_HEADER + SIZE_TRAILER);
            Debug.Assert(totalSendPayloadSize == 0);

            Debug.Assert(sendBuffers[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendBuffers[0].Offset == buffer1.Offset - SIZE_HEADER);
            Debug.Assert(sendBuffers[0].Length == SIZE_HEADER + SIZE_TRAILER);

            Debug.Assert(sendPayloads[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendPayloads[0].Offset == buffer1.Offset);
            Debug.Assert(sendPayloads[0].Length == 0);
        }

        [Fact]
        public void TestAddSendPayloadMultipleWithMultipleDownsize2()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
            BufferRef buffer1 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(3);
            message.DownsizeLastSendPayload(1);
            message.DownsizeLastSendPayload(0);
            message.DownsizeLastSendPayload(0);
            BufferRef buffer2 = message.AddSendPayload(5);
            message.DownsizeLastSendPayload(1);
            message.DownsizeLastSendPayload(0);
            message.DownsizeLastSendPayload(0);
            BufferRef buffer3 = message.AddSendPayload(5);
            message.DownsizeLastSendPayload(1);
            message.DownsizeLastSendPayload(0);
            message.DownsizeLastSendPayload(0);

            Debug.Assert(buffer3.Buffer == buffer2.Buffer);
            Debug.Assert(buffer3.Offset == buffer2.Offset);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 2);
            Debug.Assert(sendPayloads.Count == 2);
            Debug.Assert(totalSendBufferSize == 0 + SIZE_HEADER + SIZE_TRAILER);
            Debug.Assert(totalSendPayloadSize == 0);

            Debug.Assert(sendBuffers[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendBuffers[0].Offset == buffer1.Offset - SIZE_HEADER);
            Debug.Assert(sendBuffers[0].Length == SIZE_HEADER);

            Debug.Assert(sendBuffers[1].Buffer == buffer2.Buffer);
            Debug.Assert(sendBuffers[1].Offset == buffer2.Offset);
            Debug.Assert(sendBuffers[1].Length == SIZE_TRAILER);

            Debug.Assert(sendPayloads[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendPayloads[0].Offset == buffer1.Offset);
            Debug.Assert(sendPayloads[0].Length == 0);

            Debug.Assert(sendPayloads[1].Buffer == buffer2.Buffer);
            Debug.Assert(sendPayloads[1].Offset == buffer2.Offset);
            Debug.Assert(sendPayloads[1].Length == 0);
        }

        [Fact]
        public void TestAddSendPayloadMultipleWithMultipleDownsize3()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
            BufferRef buffer1 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(3);
            message.DownsizeLastSendPayload(1);
            message.DownsizeLastSendPayload(0);
            message.DownsizeLastSendPayload(0);
            message.AddSendPayload(5);
            message.DownsizeLastSendPayload(1);
            message.DownsizeLastSendPayload(0);
            message.DownsizeLastSendPayload(0);
            BufferRef buffer3 = message.AddSendPayload(6);
            message.DownsizeLastSendPayload(1);
            message.DownsizeLastSendPayload(0);
            message.DownsizeLastSendPayload(0);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 2);
            Debug.Assert(sendPayloads.Count == 2);
            Debug.Assert(totalSendBufferSize == 0 + SIZE_HEADER + SIZE_TRAILER);
            Debug.Assert(totalSendPayloadSize == 0);

            Debug.Assert(sendBuffers[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendBuffers[0].Offset == buffer1.Offset - SIZE_HEADER);
            Debug.Assert(sendBuffers[0].Length == SIZE_HEADER);

            Debug.Assert(sendBuffers[1].Buffer == buffer3.Buffer);
            Debug.Assert(sendBuffers[1].Offset == buffer3.Offset);
            Debug.Assert(sendBuffers[1].Length == SIZE_TRAILER);

            Debug.Assert(sendPayloads[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendPayloads[0].Offset == buffer1.Offset);
            Debug.Assert(sendPayloads[0].Length == 0);

            Debug.Assert(sendPayloads[1].Buffer == buffer3.Buffer);
            Debug.Assert(sendPayloads[1].Offset == buffer3.Offset);
            Debug.Assert(sendPayloads[1].Length == 0);
        }

        [Fact]
        public void TestAddSendPayloadWithDownsizeAndMultipleAddSendPayloadForOneBlock()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
            BufferRef buffer1 = message.AddSendPayload(50);
            message.DownsizeLastSendPayload(8);
            BufferRef buffer2 = message.AddSendPayload(10);
            message.DownsizeLastSendPayload(8);
            BufferRef buffer3 = message.AddSendPayload(10);
            message.DownsizeLastSendPayload(8);
            BufferRef buffer4 = message.AddSendPayload(10);
            message.DownsizeLastSendPayload(8);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 1);
            Debug.Assert(sendPayloads.Count == 1);
            Debug.Assert(totalSendBufferSize == SIZE_HEADER + 4 * 8 + SIZE_TRAILER);
            Debug.Assert(totalSendPayloadSize == 4 * 8);

            Debug.Assert(buffer2.Buffer == buffer1.Buffer);
            Debug.Assert(buffer2.Offset == buffer1.Offset + 8);
            Debug.Assert(buffer3.Buffer == buffer1.Buffer);
            Debug.Assert(buffer3.Offset == buffer1.Offset + 2 * 8);
            Debug.Assert(buffer4.Buffer == buffer1.Buffer);
            Debug.Assert(buffer4.Offset == buffer1.Offset + 3 * 8);

            Debug.Assert(sendBuffers[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendBuffers[0].Offset == buffer1.Offset - SIZE_HEADER);
            Debug.Assert(sendBuffers[0].Length == SIZE_HEADER + 4 * 8 + SIZE_TRAILER);

            Debug.Assert(sendPayloads[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendPayloads[0].Offset == buffer1.Offset);
            Debug.Assert(sendPayloads[0].Length == 4 * 8);
        }

        [Fact]
        public void TestAddSendPayloadWithDownsizeAndMultipleAddSendPayloadForTwoBlocks()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
            BufferRef buffer1 = message.AddSendPayload(50);
            message.DownsizeLastSendPayload(8);
            message.AddSendPayload(10);
            message.DownsizeLastSendPayload(8);
            message.AddSendPayload(10);
            message.DownsizeLastSendPayload(8);
            BufferRef buffer4 = message.AddSendPayload(100);
            message.DownsizeLastSendPayload(8);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 2);
            Debug.Assert(sendPayloads.Count == 2);
            Debug.Assert(totalSendBufferSize == SIZE_HEADER + 4 * 8 + SIZE_TRAILER);
            Debug.Assert(totalSendPayloadSize == 4 * 8);

            Debug.Assert(sendBuffers[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendBuffers[0].Offset == buffer1.Offset - SIZE_HEADER);
            Debug.Assert(sendBuffers[0].Length == SIZE_HEADER + 3 * 8);

            Debug.Assert(sendBuffers[1].Buffer == buffer4.Buffer);
            Debug.Assert(sendBuffers[1].Offset == buffer4.Offset);
            Debug.Assert(sendBuffers[1].Length == 1 * 8 + SIZE_TRAILER);

            Debug.Assert(sendPayloads[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendPayloads[0].Offset == buffer1.Offset);
            Debug.Assert(sendPayloads[0].Length == 3 * 8);

            Debug.Assert(sendPayloads[1].Buffer == buffer4.Buffer);
            Debug.Assert(sendPayloads[1].Offset == buffer4.Offset);
            Debug.Assert(sendPayloads[1].Length == 1 * 8);
        }

        [Fact]
        public void TestAddSendPayloadWithDownsizeAndMultipleAddSendPayloadForTwoBlocksLastZero()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
            BufferRef buffer1 = message.AddSendPayload(50);
            message.DownsizeLastSendPayload(8);
            message.AddSendPayload(10);
            message.DownsizeLastSendPayload(8);
            message.AddSendPayload(10);
            message.DownsizeLastSendPayload(8);
            BufferRef buffer4 = message.AddSendPayload(100);
            message.DownsizeLastSendPayload(0);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 2);
            Debug.Assert(sendPayloads.Count == 2);
            Debug.Assert(totalSendBufferSize == SIZE_HEADER + 3 * 8 + SIZE_TRAILER);
            Debug.Assert(totalSendPayloadSize == 3 * 8);

            Debug.Assert(sendBuffers[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendBuffers[0].Offset == buffer1.Offset - SIZE_HEADER);
            Debug.Assert(sendBuffers[0].Length == SIZE_HEADER + 3 * 8);

            Debug.Assert(sendBuffers[1].Buffer == buffer4.Buffer);
            Debug.Assert(sendBuffers[1].Offset == buffer4.Offset);
            Debug.Assert(sendBuffers[1].Length == 0 + SIZE_TRAILER);

            Debug.Assert(sendPayloads[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendPayloads[0].Offset == buffer1.Offset);
            Debug.Assert(sendPayloads[0].Length == 3 * 8);

            Debug.Assert(sendPayloads[1].Buffer == buffer4.Buffer);
            Debug.Assert(sendPayloads[1].Offset == buffer4.Offset);
            Debug.Assert(sendPayloads[1].Length == 0);
        }


        [Fact]
        public void TestAddSendPayloadMultipleWithDownsizeZeroNoTrailer()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, SIZE_HEADER, 0);
            BufferRef buffer1 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(4);
            message.AddSendPayload(4);
            message.DownsizeLastSendPayload(0);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 1);
            Debug.Assert(sendPayloads.Count == 1);
            Debug.Assert(totalSendBufferSize == 4 + SIZE_HEADER);
            Debug.Assert(totalSendPayloadSize == 4);

            Debug.Assert(sendBuffers[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendBuffers[0].Offset == buffer1.Offset - SIZE_HEADER);
            Debug.Assert(sendBuffers[0].Length == SIZE_HEADER + 4);

            Debug.Assert(sendPayloads[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendPayloads[0].Offset == buffer1.Offset);
            Debug.Assert(sendPayloads[0].Length == 4);
        }


        [Fact]
        public void TestAddSendPayloadMultipleWithDownsizeZeroAdAnotherSendPayloadNoTrailer()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, SIZE_HEADER, 0);
            BufferRef buffer1 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(4);
            message.AddSendPayload(4);
            message.DownsizeLastSendPayload(0);
            BufferRef buffer3 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(2);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 2);
            Debug.Assert(sendPayloads.Count == 2);
            Debug.Assert(totalSendBufferSize == 6 + SIZE_HEADER);
            Debug.Assert(totalSendPayloadSize == 6);

            Debug.Assert(sendBuffers[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendBuffers[0].Offset == buffer1.Offset - SIZE_HEADER);
            Debug.Assert(sendBuffers[0].Length == SIZE_HEADER + 4);

            Debug.Assert(sendBuffers[1].Buffer == buffer3.Buffer);
            Debug.Assert(sendBuffers[1].Offset == buffer3.Offset);
            Debug.Assert(sendBuffers[1].Length == 2);

            Debug.Assert(sendPayloads[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendPayloads[0].Offset == buffer1.Offset);
            Debug.Assert(sendPayloads[0].Length == 4);

            Debug.Assert(sendPayloads[1].Buffer == buffer3.Buffer);
            Debug.Assert(sendPayloads[1].Offset == buffer3.Offset);
            Debug.Assert(sendPayloads[1].Length == 2);
        }

        [Fact]
        public void TestFirstDownsizeZeroBlock()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, SIZE_HEADER, 0);
            BufferRef buffer1 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(0);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 1);
            Debug.Assert(sendPayloads.Count == 1);
            Debug.Assert(totalSendBufferSize == 0 + SIZE_HEADER);
            Debug.Assert(totalSendPayloadSize == 0);

            Debug.Assert(sendBuffers[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendBuffers[0].Offset == buffer1.Offset - SIZE_HEADER);
            Debug.Assert(sendBuffers[0].Length == SIZE_HEADER + 0);

            Debug.Assert(sendPayloads[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendPayloads[0].Offset == buffer1.Offset);
            Debug.Assert(sendPayloads[0].Length == 0);
        }


        [Fact]
        public void TestFirstDownsizeZeroBlockRemoveBlock()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, 0, 0);
            BufferRef buffer1 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(0);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 0);
            Debug.Assert(sendPayloads.Count == 0);
            Debug.Assert(totalSendBufferSize == 0);
            Debug.Assert(totalSendPayloadSize == 0);
        }

        [Fact]
        public void TestFirstDownsizeZeroBlockRemoveBlockRepeatDownsize()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, 0, 0);
            BufferRef buffer1 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(0);
            message.DownsizeLastSendPayload(0);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 0);
            Debug.Assert(sendPayloads.Count == 0);
            Debug.Assert(totalSendBufferSize == 0);
            Debug.Assert(totalSendPayloadSize == 0);
        }


        [Fact]
        public void TestFirstDownsizeZeroBlockRemoveBlockAddSendPayload()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, 0, 0);
            BufferRef buffer1 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(0);
            BufferRef buffer2 = message.AddSendPayload(4);
            message.DownsizeLastSendPayload(2);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 1);
            Debug.Assert(sendPayloads.Count == 1);
            Debug.Assert(totalSendBufferSize == 2);
            Debug.Assert(totalSendPayloadSize == 2);

            Debug.Assert(sendBuffers[0].Buffer == buffer2.Buffer);
            Debug.Assert(sendBuffers[0].Offset == buffer2.Offset);
            Debug.Assert(sendBuffers[0].Length == 2);

            Debug.Assert(sendPayloads[0].Buffer == buffer2.Buffer);
            Debug.Assert(sendPayloads[0].Offset == buffer2.Offset);
            Debug.Assert(sendPayloads[0].Length == 2);
        }


        [Fact]
        public void TestAddSendHeaderAfterAddPayload()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
            BufferRef buffer1 = message.AddSendPayload(8);
            message.DownsizeLastSendPayload(4);
            BufferRef buffer2 = message.AddSendHeader(8);
            message.DownsizeLastSendHeader(4);
            BufferRef buffer3 = message.AddSendHeader(8);
            message.DownsizeLastSendHeader(4);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 3);
            Debug.Assert(sendPayloads.Count == 1);
            Debug.Assert(totalSendBufferSize == 3 * 4 + SIZE_HEADER + SIZE_TRAILER);
            Debug.Assert(totalSendPayloadSize == 4);

            Debug.Assert(sendBuffers[0].Buffer == buffer2.Buffer);
            Debug.Assert(sendBuffers[0].Offset == buffer2.Offset);
            Debug.Assert(sendBuffers[0].Length == 4);

            Debug.Assert(sendBuffers[1].Buffer == buffer3.Buffer);
            Debug.Assert(sendBuffers[1].Offset == buffer3.Offset);
            Debug.Assert(sendBuffers[1].Length == 4);

            Debug.Assert(sendBuffers[2].Buffer == buffer1.Buffer);
            Debug.Assert(sendBuffers[2].Offset == buffer1.Offset - SIZE_HEADER);
            Debug.Assert(sendBuffers[2].Length == SIZE_HEADER + 4 + SIZE_TRAILER);

            Debug.Assert(sendPayloads[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendPayloads[0].Offset == buffer1.Offset);
            Debug.Assert(sendPayloads[0].Length == 4);
        }

        [Fact]
        public void TestAddSendHeaderBeforeAddPayload()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
            BufferRef buffer2 = message.AddSendHeader(8);
            message.DownsizeLastSendHeader(4);
            BufferRef buffer3 = message.AddSendHeader(8);
            message.DownsizeLastSendHeader(4);
            BufferRef buffer1 = message.AddSendPayload(8);
            message.DownsizeLastSendPayload(4);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 3);
            Debug.Assert(sendPayloads.Count == 1);
            Debug.Assert(totalSendBufferSize == 3 * 4 + SIZE_HEADER + SIZE_TRAILER);
            Debug.Assert(totalSendPayloadSize == 4);

            Debug.Assert(sendBuffers[0].Buffer == buffer2.Buffer);
            Debug.Assert(sendBuffers[0].Offset == buffer2.Offset);
            Debug.Assert(sendBuffers[0].Length == 4);

            Debug.Assert(sendBuffers[1].Buffer == buffer3.Buffer);
            Debug.Assert(sendBuffers[1].Offset == buffer3.Offset);
            Debug.Assert(sendBuffers[1].Length == 4);

            Debug.Assert(sendBuffers[2].Buffer == buffer1.Buffer);
            Debug.Assert(sendBuffers[2].Offset == buffer1.Offset - SIZE_HEADER);
            Debug.Assert(sendBuffers[2].Length == SIZE_HEADER + 4 + SIZE_TRAILER);

            Debug.Assert(sendPayloads[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendPayloads[0].Offset == buffer1.Offset);
            Debug.Assert(sendPayloads[0].Length == 4);
        }

        [Fact]
        public void TestAddSendHeaderAndRemoveItWithPayload()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, 0, 0);
            BufferRef buffer1 = message.AddSendPayload(8);
            message.DownsizeLastSendPayload(4);
            BufferRef buffer2 = message.AddSendHeader(8);
            message.DownsizeLastSendHeader(0);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 1);
            Debug.Assert(sendPayloads.Count == 1);
            Debug.Assert(totalSendBufferSize == 4);
            Debug.Assert(totalSendPayloadSize == 4);

            Debug.Assert(sendBuffers[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendBuffers[0].Offset == buffer1.Offset);
            Debug.Assert(sendBuffers[0].Length == 4);

            Debug.Assert(sendPayloads[0].Buffer == buffer1.Buffer);
            Debug.Assert(sendPayloads[0].Offset == buffer1.Offset);
            Debug.Assert(sendPayloads[0].Length == 4);
        }

        [Fact]
        public void TestAddSendHeaderAndRemoveItWithoutPayload()
        {
            ProtocolMessage message = new ProtocolMessage(PROTOCOL_ID, 0, 0);
            BufferRef buffer1 = message.AddSendHeader(8);
            message.DownsizeLastSendHeader(0);

            IList<BufferRef> sendBuffers = message.GetAllSendBuffers();
            IList<BufferRef> sendPayloads = message.GetAllSendPayloads();
            int totalSendBufferSize = message.GetTotalSendBufferSize();
            int totalSendPayloadSize = message.GetTotalSendPayloadSize();

            Debug.Assert(sendBuffers.Count == 0);
            Debug.Assert(sendPayloads.Count == 0);
            Debug.Assert(totalSendBufferSize == 0);
            Debug.Assert(totalSendPayloadSize == 0);
        }

    }
}
