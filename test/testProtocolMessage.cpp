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

#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "finalmq/protocolsession/ProtocolMessage.h"


//using ::testing::_;
//using ::testing::Return;
//using ::testing::InSequence;
//using ::testing::DoAll;

using namespace finalmq;

static const int PROTOCOL_ID = 123;
static const int SIZE_HEADER = 12;
static const int SIZE_TRAILER = 8;



class TestProtocolMessage: public testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

};


TEST_F(TestProtocolMessage, addSendPayloadMultiple)
{
    ProtocolMessage message(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
    IMessage& imessage = message;
    char* buffer1 = imessage.addSendPayload(4);
    char* buffer2 = imessage.addSendPayload(4);
    char* buffer3 = imessage.addSendPayload(4);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 3);
    ASSERT_EQ(sendPayloads.size(), 3);
    ASSERT_EQ(totalSendBufferSize, 12 + SIZE_HEADER + SIZE_TRAILER);
    ASSERT_EQ(totalSendPayloadSize, 12);

    auto itBuffers = sendBuffers.begin();
    ASSERT_EQ(itBuffers->first, buffer1 - SIZE_HEADER);
    ASSERT_EQ(itBuffers->second, SIZE_HEADER + 4);
    ++itBuffers;
    ASSERT_EQ(itBuffers->first, buffer2);
    ASSERT_EQ(itBuffers->second, 4);
    ++itBuffers;
    ASSERT_EQ(itBuffers->first, buffer3);
    ASSERT_EQ(itBuffers->second, 4 + SIZE_TRAILER);

    auto itPayloads = sendPayloads.begin();
    ASSERT_EQ(itPayloads->first, buffer1);
    ASSERT_EQ(itPayloads->second, 4);
    ++itPayloads;
    ASSERT_EQ(itPayloads->first, buffer2);
    ASSERT_EQ(itPayloads->second, 4);
    ++itPayloads;
    ASSERT_EQ(itPayloads->first, buffer3);
    ASSERT_EQ(itPayloads->second, 4);
}



TEST_F(TestProtocolMessage, addSendPayloadMultipleWithUnchangedDownsize)
{
    ProtocolMessage message(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
    IMessage& imessage = message;
    char* buffer1 = imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(4);
    char* buffer2 = imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(4);
    char* buffer3 = imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(4);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 3);
    ASSERT_EQ(sendPayloads.size(), 3);
    ASSERT_EQ(totalSendBufferSize, 12 + SIZE_HEADER + SIZE_TRAILER);
    ASSERT_EQ(totalSendPayloadSize, 12);

    auto itBuffers = sendBuffers.begin();
    ASSERT_EQ(itBuffers->first, buffer1 - SIZE_HEADER);
    ASSERT_EQ(itBuffers->second, SIZE_HEADER + 4);
    ++itBuffers;
    ASSERT_EQ(itBuffers->first, buffer2);
    ASSERT_EQ(itBuffers->second, 4);
    ++itBuffers;
    ASSERT_EQ(itBuffers->first, buffer3);
    ASSERT_EQ(itBuffers->second, 4 + SIZE_TRAILER);

    auto itPayloads = sendPayloads.begin();
    ASSERT_EQ(itPayloads->first, buffer1);
    ASSERT_EQ(itPayloads->second, 4);
    ++itPayloads;
    ASSERT_EQ(itPayloads->first, buffer2);
    ASSERT_EQ(itPayloads->second, 4);
    ++itPayloads;
    ASSERT_EQ(itPayloads->first, buffer3);
    ASSERT_EQ(itPayloads->second, 4);
}

TEST_F(TestProtocolMessage, addSendPayloadMultipleWithChangedDownsize)
{
    ProtocolMessage message(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
    IMessage& imessage = message;
    char* buffer1 = imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(3);
    char* buffer2 = imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(3);
    char* buffer3 = imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(3);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 3);
    ASSERT_EQ(sendPayloads.size(), 3);
    ASSERT_EQ(totalSendBufferSize, 9 + SIZE_HEADER + SIZE_TRAILER);
    ASSERT_EQ(totalSendPayloadSize, 9);

    auto itBuffers = sendBuffers.begin();
    ASSERT_EQ(itBuffers->first, buffer1 - SIZE_HEADER);
    ASSERT_EQ(itBuffers->second, SIZE_HEADER + 3);
    ++itBuffers;
    ASSERT_EQ(itBuffers->first, buffer2);
    ASSERT_EQ(itBuffers->second, 3);
    ++itBuffers;
    ASSERT_EQ(itBuffers->first, buffer3);
    ASSERT_EQ(itBuffers->second, 3 + SIZE_TRAILER);

    auto itPayloads = sendPayloads.begin();
    ASSERT_EQ(itPayloads->first, buffer1);
    ASSERT_EQ(itPayloads->second, 3);
    ++itPayloads;
    ASSERT_EQ(itPayloads->first, buffer2);
    ASSERT_EQ(itPayloads->second, 3);
    ++itPayloads;
    ASSERT_EQ(itPayloads->first, buffer3);
    ASSERT_EQ(itPayloads->second, 3);
}


TEST_F(TestProtocolMessage, addSendPayloadMultipleWithMultipleDownsize)
{
    ProtocolMessage message(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
    IMessage& imessage = message;
    char* buffer1 = imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(3);
    imessage.downsizeLastSendPayload(1);
    imessage.downsizeLastSendPayload(0);
    imessage.downsizeLastSendPayload(0);
    char* buffer2 = imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(1);
    imessage.downsizeLastSendPayload(0);
    imessage.downsizeLastSendPayload(0);
    char* buffer3 = imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(1);
    imessage.downsizeLastSendPayload(0);
    imessage.downsizeLastSendPayload(0);

    ASSERT_EQ(buffer2, buffer1);
    ASSERT_EQ(buffer3, buffer1);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 1);
    ASSERT_EQ(sendPayloads.size(), 1);
    ASSERT_EQ(totalSendBufferSize, SIZE_HEADER + SIZE_TRAILER);
    ASSERT_EQ(totalSendPayloadSize, 0);

    auto itBuffers = sendBuffers.begin();
    ASSERT_EQ(itBuffers->first, buffer1 - SIZE_HEADER);
    ASSERT_EQ(itBuffers->second, SIZE_HEADER + SIZE_TRAILER);

    auto itPayloads = sendPayloads.begin();
    ASSERT_EQ(itPayloads->first, buffer1);
    ASSERT_EQ(itPayloads->second, 0);
}

TEST_F(TestProtocolMessage, addSendPayloadMultipleWithMultipleDownsize2)
{
    ProtocolMessage message(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
    IMessage& imessage = message;
    char* buffer1 = imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(3);
    imessage.downsizeLastSendPayload(1);
    imessage.downsizeLastSendPayload(0);
    imessage.downsizeLastSendPayload(0);
    char* buffer2 = imessage.addSendPayload(5);
    imessage.downsizeLastSendPayload(1);
    imessage.downsizeLastSendPayload(0);
    imessage.downsizeLastSendPayload(0);
    char* buffer3 = imessage.addSendPayload(5);
    imessage.downsizeLastSendPayload(1);
    imessage.downsizeLastSendPayload(0);
    imessage.downsizeLastSendPayload(0);

    ASSERT_EQ(buffer3, buffer2);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 2);
    ASSERT_EQ(sendPayloads.size(), 2);
    ASSERT_EQ(totalSendBufferSize, SIZE_HEADER + SIZE_TRAILER);
    ASSERT_EQ(totalSendPayloadSize, 0);

    auto itBuffers = sendBuffers.begin();
    ASSERT_EQ(itBuffers->first, buffer1 - SIZE_HEADER);
    ASSERT_EQ(itBuffers->second, SIZE_HEADER);
    ++itBuffers;
    ASSERT_EQ(itBuffers->first, buffer2);
    ASSERT_EQ(itBuffers->second, SIZE_TRAILER);

    auto itPayloads = sendPayloads.begin();
    ASSERT_EQ(itPayloads->first, buffer1);
    ASSERT_EQ(itPayloads->second, 0);
    ++itPayloads;
    ASSERT_EQ(itPayloads->first, buffer2);
    ASSERT_EQ(itPayloads->second, 0);
}

TEST_F(TestProtocolMessage, addSendPayloadMultipleWithMultipleDownsize3)
{
    ProtocolMessage message(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
    IMessage& imessage = message;
    char* buffer1 = imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(3);
    imessage.downsizeLastSendPayload(1);
    imessage.downsizeLastSendPayload(0);
    imessage.downsizeLastSendPayload(0);
    imessage.addSendPayload(5);
    imessage.downsizeLastSendPayload(1);
    imessage.downsizeLastSendPayload(0);
    imessage.downsizeLastSendPayload(0);
    char* buffer3 = imessage.addSendPayload(6);
    imessage.downsizeLastSendPayload(1);
    imessage.downsizeLastSendPayload(0);
    imessage.downsizeLastSendPayload(0);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 2);
    ASSERT_EQ(sendPayloads.size(), 2);
    ASSERT_EQ(totalSendBufferSize, SIZE_HEADER + SIZE_TRAILER);
    ASSERT_EQ(totalSendPayloadSize, 0);

    auto itBuffers = sendBuffers.begin();
    ASSERT_EQ(itBuffers->first, buffer1 - SIZE_HEADER);
    ASSERT_EQ(itBuffers->second, SIZE_HEADER);
    ++itBuffers;
    ASSERT_EQ(itBuffers->first, buffer3);
    ASSERT_EQ(itBuffers->second, SIZE_TRAILER);

    auto itPayloads = sendPayloads.begin();
    ASSERT_EQ(itPayloads->first, buffer1);
    ASSERT_EQ(itPayloads->second, 0);
    ++itPayloads;
    ASSERT_EQ(itPayloads->first, buffer3);
    ASSERT_EQ(itPayloads->second, 0);
}



TEST_F(TestProtocolMessage, addSendPayloadWithDownsizeAndMultipleAddSendPayloadForOneBlock)
{
    ProtocolMessage message(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
    IMessage& imessage = message;
    char* buffer1 = imessage.addSendPayload(50);
    imessage.downsizeLastSendPayload(8);
    char* buffer2 = imessage.addSendPayload(10);
    imessage.downsizeLastSendPayload(8);
    char* buffer3 = imessage.addSendPayload(10);
    imessage.downsizeLastSendPayload(8);
    char* buffer4 = imessage.addSendPayload(10);
    imessage.downsizeLastSendPayload(8);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 1);
    ASSERT_EQ(sendPayloads.size(), 1);
    ASSERT_EQ(totalSendBufferSize, SIZE_HEADER + 4*8 + SIZE_TRAILER);
    ASSERT_EQ(totalSendPayloadSize, 4*8);

    ASSERT_EQ(buffer2, buffer1 + 8);
    ASSERT_EQ(buffer3, buffer1 + 2*8);
    ASSERT_EQ(buffer4, buffer1 + 3*8);

    auto itBuffers = sendBuffers.begin();
    ASSERT_EQ(itBuffers->first, buffer1 - SIZE_HEADER);
    ASSERT_EQ(itBuffers->second, SIZE_HEADER + 4*8 + SIZE_TRAILER);

    auto itPayloads = sendPayloads.begin();
    ASSERT_EQ(itPayloads->first, buffer1);
    ASSERT_EQ(itPayloads->second, 4*8);
}

TEST_F(TestProtocolMessage, addSendPayloadWithDownsizeAndMultipleAddSendPayloadForTwoBlocks)
{
    ProtocolMessage message(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
    IMessage& imessage = message;
    char* buffer1 = imessage.addSendPayload(50);
    imessage.downsizeLastSendPayload(8);
    imessage.addSendPayload(10);
    imessage.downsizeLastSendPayload(8);
    imessage.addSendPayload(10);
    imessage.downsizeLastSendPayload(8);
    char* buffer4 = imessage.addSendPayload(100);
    imessage.downsizeLastSendPayload(8);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 2);
    ASSERT_EQ(sendPayloads.size(), 2);
    ASSERT_EQ(totalSendBufferSize, SIZE_HEADER + 4*8 + SIZE_TRAILER);
    ASSERT_EQ(totalSendPayloadSize, 4*8);

    auto itBuffers = sendBuffers.begin();
    ASSERT_EQ(itBuffers->first, buffer1 - SIZE_HEADER);
    ASSERT_EQ(itBuffers->second, SIZE_HEADER + 3*8);
    ++itBuffers;
    ASSERT_EQ(itBuffers->first, buffer4);
    ASSERT_EQ(itBuffers->second, 1*8 + SIZE_TRAILER);

    auto itPayloads = sendPayloads.begin();
    ASSERT_EQ(itPayloads->first, buffer1);
    ASSERT_EQ(itPayloads->second, 3*8);
    ++itPayloads;
    ASSERT_EQ(itPayloads->first, buffer4);
    ASSERT_EQ(itPayloads->second, 1*8);
}

TEST_F(TestProtocolMessage, addSendPayloadWithDownsizeAndMultipleAddSendPayloadForTwoBlocksLastZero)
{
    ProtocolMessage message(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
    IMessage& imessage = message;
    char* buffer1 = imessage.addSendPayload(50);
    imessage.downsizeLastSendPayload(8);
    imessage.addSendPayload(10);
    imessage.downsizeLastSendPayload(8);
    imessage.addSendPayload(10);
    imessage.downsizeLastSendPayload(8);
    char* buffer4 = imessage.addSendPayload(100);
    imessage.downsizeLastSendPayload(0);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 2);
    ASSERT_EQ(sendPayloads.size(), 2);
    ASSERT_EQ(totalSendBufferSize, SIZE_HEADER + 3*8 + SIZE_TRAILER);
    ASSERT_EQ(totalSendPayloadSize, 3*8);

    auto itBuffers = sendBuffers.begin();
    ASSERT_EQ(itBuffers->first, buffer1 - SIZE_HEADER);
    ASSERT_EQ(itBuffers->second, SIZE_HEADER + 3*8);
    ++itBuffers;
    ASSERT_EQ(itBuffers->first, buffer4);
    ASSERT_EQ(itBuffers->second, SIZE_TRAILER);

    auto itPayloads = sendPayloads.begin();
    ASSERT_EQ(itPayloads->first, buffer1);
    ASSERT_EQ(itPayloads->second, 3*8);
    ++itPayloads;
    ASSERT_EQ(itPayloads->first, buffer4);
    ASSERT_EQ(itPayloads->second, 0);
}


TEST_F(TestProtocolMessage, addSendPayloadMultipleWithDownsizeZeroNoTrailer)
{
    ProtocolMessage message(PROTOCOL_ID, SIZE_HEADER, 0);
    IMessage& imessage = message;
    char* buffer1 = imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(4);
    imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(0);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 1);
    ASSERT_EQ(sendPayloads.size(), 1);
    ASSERT_EQ(totalSendBufferSize, 4 + SIZE_HEADER);
    ASSERT_EQ(totalSendPayloadSize, 4);

    auto itBuffers = sendBuffers.begin();
    ASSERT_EQ(itBuffers->first, buffer1 - SIZE_HEADER);
    ASSERT_EQ(itBuffers->second, SIZE_HEADER + 4);

    auto itPayloads = sendPayloads.begin();
    ASSERT_EQ(itPayloads->first, buffer1);
    ASSERT_EQ(itPayloads->second, 4);
}


TEST_F(TestProtocolMessage, addSendPayloadMultipleWithDownsizeZeroAdAnotherSendPayloadNoTrailer)
{
    ProtocolMessage message(PROTOCOL_ID, SIZE_HEADER, 0);
    IMessage& imessage = message;
    char* buffer1 = imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(4);
    imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(0);
    char* buffer3 = imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(2);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 2);
    ASSERT_EQ(sendPayloads.size(), 2);
    ASSERT_EQ(totalSendBufferSize, 6 + SIZE_HEADER);
    ASSERT_EQ(totalSendPayloadSize, 6);

    auto itBuffers = sendBuffers.begin();
    ASSERT_EQ(itBuffers->first, buffer1 - SIZE_HEADER);
    ASSERT_EQ(itBuffers->second, SIZE_HEADER + 4);
    ++itBuffers;
    ASSERT_EQ(itBuffers->first, buffer3);
    ASSERT_EQ(itBuffers->second, 2);

    auto itPayloads = sendPayloads.begin();
    ASSERT_EQ(itPayloads->first, buffer1);
    ASSERT_EQ(itPayloads->second, 4);
    ++itPayloads;
    ASSERT_EQ(itPayloads->first, buffer3);
    ASSERT_EQ(itPayloads->second, 2);
}


TEST_F(TestProtocolMessage, firstDownsizeZeroBlock)
{
    ProtocolMessage message(PROTOCOL_ID, SIZE_HEADER, 0);
    IMessage& imessage = message;
    char* buffer1 = imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(0);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 1);
    ASSERT_EQ(sendPayloads.size(), 1);
    ASSERT_EQ(totalSendBufferSize, 0 + SIZE_HEADER);
    ASSERT_EQ(totalSendPayloadSize, 0);

    auto itBuffers = sendBuffers.begin();
    ASSERT_EQ(itBuffers->first, buffer1 - SIZE_HEADER);
    ASSERT_EQ(itBuffers->second, SIZE_HEADER + 0);

    auto itPayloads = sendPayloads.begin();
    ASSERT_EQ(itPayloads->first, buffer1);
    ASSERT_EQ(itPayloads->second, 0);
}


TEST_F(TestProtocolMessage, firstDownsizeZeroBlockRemoveBlock)
{
    ProtocolMessage message(PROTOCOL_ID, 0, 0);
    IMessage& imessage = message;
    imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(0);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 0);
    ASSERT_EQ(sendPayloads.size(), 0);
    ASSERT_EQ(totalSendBufferSize, 0);
    ASSERT_EQ(totalSendPayloadSize, 0);
}

TEST_F(TestProtocolMessage, firstDownsizeZeroBlockRemoveBlockRepeatDownsize)
{
    ProtocolMessage message(PROTOCOL_ID, 0, 0);
    IMessage& imessage = message;
    imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(0);
    imessage.downsizeLastSendPayload(0);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 0);
    ASSERT_EQ(sendPayloads.size(), 0);
    ASSERT_EQ(totalSendBufferSize, 0);
    ASSERT_EQ(totalSendPayloadSize, 0);
}


TEST_F(TestProtocolMessage, firstDownsizeZeroBlockRemoveBlockAddSendPayload)
{
    ProtocolMessage message(PROTOCOL_ID, 0, 0);
    IMessage& imessage = message;
    imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(0);
    char* buffer2 = imessage.addSendPayload(4);
    imessage.downsizeLastSendPayload(2);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 1);
    ASSERT_EQ(sendPayloads.size(), 1);
    ASSERT_EQ(totalSendBufferSize, 2);
    ASSERT_EQ(totalSendPayloadSize, 2);

    auto itBuffers = sendBuffers.begin();
    ASSERT_EQ(itBuffers->first, buffer2);
    ASSERT_EQ(itBuffers->second, 2);

    auto itPayloads = sendPayloads.begin();
    ASSERT_EQ(itPayloads->first, buffer2);
    ASSERT_EQ(itPayloads->second, 2);
}


TEST_F(TestProtocolMessage, addSendHeaderAfterAddPayload)
{
    ProtocolMessage message(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
    IMessage& imessage = message;
    char* buffer1 = imessage.addSendPayload(8);
    imessage.downsizeLastSendPayload(4);
    char* buffer2 = imessage.addSendHeader(8);
    imessage.downsizeLastSendHeader(4);
    char* buffer3 = imessage.addSendHeader(8);
    imessage.downsizeLastSendHeader(4);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 3);
    ASSERT_EQ(sendPayloads.size(), 1);
    ASSERT_EQ(totalSendBufferSize, 3*4 + SIZE_HEADER + SIZE_TRAILER);
    ASSERT_EQ(totalSendPayloadSize, 4);

    auto itBuffers = sendBuffers.begin();
    ASSERT_EQ(itBuffers->first, buffer2);
    ASSERT_EQ(itBuffers->second, 4);
    ++itBuffers;
    ASSERT_EQ(itBuffers->first, buffer3);
    ASSERT_EQ(itBuffers->second, 4);
    ++itBuffers;
    ASSERT_EQ(itBuffers->first, buffer1 - SIZE_HEADER);
    ASSERT_EQ(itBuffers->second, SIZE_HEADER + 4 + SIZE_TRAILER);

    auto itPayloads = sendPayloads.begin();
    ASSERT_EQ(itPayloads->first, buffer1);
    ASSERT_EQ(itPayloads->second, 4);
}


TEST_F(TestProtocolMessage, addSendHeaderBeforeAddPayload)
{
    ProtocolMessage message(PROTOCOL_ID, SIZE_HEADER, SIZE_TRAILER);
    IMessage& imessage = message;
    char* buffer2 = imessage.addSendHeader(8);
    imessage.downsizeLastSendHeader(4);
    char* buffer3 = imessage.addSendHeader(8);
    imessage.downsizeLastSendHeader(4);
    char* buffer1 = imessage.addSendPayload(8);
    imessage.downsizeLastSendPayload(4);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 3);
    ASSERT_EQ(sendPayloads.size(), 1);
    ASSERT_EQ(totalSendBufferSize, 3 * 4 + SIZE_HEADER + SIZE_TRAILER);
    ASSERT_EQ(totalSendPayloadSize, 4);

    auto itBuffers = sendBuffers.begin();
    ASSERT_EQ(itBuffers->first, buffer2);
    ASSERT_EQ(itBuffers->second, 4);
    ++itBuffers;
    ASSERT_EQ(itBuffers->first, buffer3);
    ASSERT_EQ(itBuffers->second, 4);
    ++itBuffers;
    ASSERT_EQ(itBuffers->first, buffer1 - SIZE_HEADER);
    ASSERT_EQ(itBuffers->second, SIZE_HEADER + 4 + SIZE_TRAILER);

    auto itPayloads = sendPayloads.begin();
    ASSERT_EQ(itPayloads->first, buffer1);
    ASSERT_EQ(itPayloads->second, 4);
}

TEST_F(TestProtocolMessage, addSendHeaderAndRemoveItWithPayload)
{
    ProtocolMessage message(PROTOCOL_ID, 0, 0);
    IMessage& imessage = message;
    char* buffer1 = imessage.addSendPayload(8);
    imessage.downsizeLastSendPayload(4);
    /*char* buffer2 =*/ imessage.addSendHeader(8);
    imessage.downsizeLastSendHeader(0);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 1);
    ASSERT_EQ(sendPayloads.size(), 1);
    ASSERT_EQ(totalSendBufferSize, 4);
    ASSERT_EQ(totalSendPayloadSize, 4);

    auto itBuffers = sendBuffers.begin();
    ASSERT_EQ(itBuffers->first, buffer1);
    ASSERT_EQ(itBuffers->second, 4);

    auto itPayloads = sendPayloads.begin();
    ASSERT_EQ(itPayloads->first, buffer1);
    ASSERT_EQ(itPayloads->second, 4);
}

TEST_F(TestProtocolMessage, addSendHeaderAndRemoveItWithoutPayload)
{
    ProtocolMessage message(PROTOCOL_ID, 0, 0);
    IMessage& imessage = message;
    /*char* buffer1 =*/ imessage.addSendHeader(8);
    imessage.downsizeLastSendHeader(0);

    const std::list<BufferRef>& sendBuffers = imessage.getAllSendBuffers();
    const std::list<BufferRef>& sendPayloads = imessage.getAllSendPayloads();
    ssize_t totalSendBufferSize = imessage.getTotalSendBufferSize();
    ssize_t totalSendPayloadSize = imessage.getTotalSendPayloadSize();

    ASSERT_EQ(sendBuffers.size(), 0);
    ASSERT_EQ(sendPayloads.size(), 0);
    ASSERT_EQ(totalSendBufferSize, 0);
    ASSERT_EQ(totalSendPayloadSize, 0);
}

