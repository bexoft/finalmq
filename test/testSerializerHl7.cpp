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


#include "finalmq/serializehl7/SerializerHl7.h"
#include "finalmq/metadata/MetaData.h"
#include "finalmq/serializestruct/ParserStruct.h"
#include "MockIZeroCopyBuffer.h"
//#include "test.fmq.h"
#include "testhl7.fmq.h"

#include <cmath>


using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using testing::Invoke;
using testing::DoAll;

using namespace finalmq;


MATCHER_P(MatcherMetaField, metaField, "")
{
    return (arg.type == metaField.type &&
            arg.typeName == metaField.typeName &&
            arg.name == metaField.name);
}


struct String : public std::string
{
    void resize(size_t size)
    {
        std::string::resize(size);
    }
};



class TestSerializerHl7 : public testing::Test
{
public:
protected:
    virtual void SetUp()
    {
        static const int MAX_BLOCK_SIZE = 5000;

        m_data.resize(MAX_BLOCK_SIZE);
        EXPECT_CALL(m_mockBuffer, addBuffer(MAX_BLOCK_SIZE, _)).Times(1).WillOnce(Return((char*)m_data.data()));
        EXPECT_CALL(m_mockBuffer, downsizeLastBuffer(_)).Times(1).WillOnce(Invoke(&m_data, &String::resize));
        EXPECT_CALL(m_mockBuffer, getRemainingSize()).WillRepeatedly(Return(0));
        m_serializer = std::make_unique<SerializerHl7>(m_mockBuffer, MAX_BLOCK_SIZE);
    }

    virtual void TearDown()
    {
    }

    String                          m_data;
    MockIZeroCopyBuffer             m_mockBuffer;
    std::unique_ptr<IParserVisitor> m_serializer;
};



/*
TEST_F(TestSerializerHl7, testSSU_U03)
{
    hl7::SSU_U03 msg;
    msg.msh.countryCode = "de";
    msg.uac = std::make_shared<hl7::UAC>();
    msg.uac->userAuthenticationCredential.typeOfData = "hello";
    msg.sft.resize(3);
    msg.sft[0].softwareBinaryID = "world";
    msg.sac.resize(4);
    msg.sac[0].sac.positionInTray = "hey";
    msg.sac[0].sac.specimenSource = "hh";
    msg.sac[0].sac.carrierId.entityIdentifier = "uu";
    msg.sac[0].sac.carrierId.universalId = "bbb";
    msg.sac[0].obx.resize(2);
    msg.sac[0].nte.resize(1);
    msg.sac[0].spm.resize(3);
    msg.sac[0].spm[0].spm.accessionId = "ggg";
    msg.sac[0].spm[0].spm.containerCondition.alternateText = "tt";
    msg.sac[0].spm[0].spm.containerCondition.nameOfAlternateCodingSystem = "cc";
    msg.sac[0].spm[0].obx.resize(5);

    ParserStruct parser(*m_serializer, msg);
    parser.parseStruct();


    ASSERT_EQ(m_data, "MSH|^~\\&|||||||SSU^U03^SSU_U03||||||||de\rSFT|||world\rSFT\rSFT\rUAC||^hello\rEQU\rSAC||||||hh||||uu^^bbb||||hey\rOBX\rOBX\rNTE\rSPM||||||||||||||||||||||||||||^^^^tt^cc||ggg\rOBX\rOBX\rOBX\rOBX\rOBX\rSPM\rSPM\rSAC\rSAC\rSAC\r");
}
*/


TEST_F(TestSerializerHl7, testMSG_001)
{
    testhl7::MSG_001 msg;
    msg.msh.fieldSeparator = "|";
    msg.msh.encodingCharacters = "^~\\&";
    msg.msh.messageType.messageCode = "MSG";
    msg.msh.messageType.triggerEvent = "001";
    msg.msh.messageType.messageStructure = "MSG_001";
    msg.a.a = "a";

    ParserStruct parser(*m_serializer, msg);
    parser.parseStruct();

    ASSERT_EQ(m_data, "MSH|^~\\&|||||||MSG^001^MSG_001\rAAA|a\r");
}

TEST_F(TestSerializerHl7, testMSG_007)
{
    testhl7::MSG_007 msg;
    msg.msh.fieldSeparator = "|";
    msg.msh.encodingCharacters = "^~\\&";
    msg.msh.messageType.messageCode = "MSG";
    msg.msh.messageType.triggerEvent = "007";
    msg.msh.messageType.messageStructure = "MSG_007";
    msg.e.e.a = "a";
    msg.e.e.b = "b";

    ParserStruct parser(*m_serializer, msg);
    parser.parseStruct();

    ASSERT_EQ(m_data, "MSH|^~\\&|||||||MSG^007^MSG_007\rEEE|a^b\r");
}

TEST_F(TestSerializerHl7, testMSG_010)
{
    testhl7::MSG_010 msg;
    msg.msh.fieldSeparator = "|";
    msg.msh.encodingCharacters = "^~\\&";
    msg.msh.messageType.messageCode = "MSG";
    msg.msh.messageType.triggerEvent = "010";
    msg.msh.messageType.messageStructure = "MSG_010";
    msg.a01.a.push_back("a1");
    msg.a01.a.push_back("a2");
    msg.a01.a.push_back("a3");

    ParserStruct parser(*m_serializer, msg);
    parser.parseStruct();

    ASSERT_EQ(m_data, "MSH|^~\\&|||||||MSG^010^MSG_010\rA01|a1~a2~a3\r");
}

TEST_F(TestSerializerHl7, testMSG_011_1)
{
    testhl7::MSG_011 msg;
    msg.msh.fieldSeparator = "|";
    msg.msh.encodingCharacters = "^~\\&";
    msg.msh.messageType.messageCode = "MSG";
    msg.msh.messageType.triggerEvent = "011";
    msg.msh.messageType.messageStructure = "MSG_011";
    msg.a02.faa.emplace_back("a1", "b1");
    msg.a02.faa.emplace_back("a2", "b2");
    msg.a02.faa.emplace_back("a3", "b3");

    ParserStruct parser(*m_serializer, msg);
    parser.parseStruct();

    ASSERT_EQ(m_data, "MSH|^~\\&|||||||MSG^011^MSG_011\rA02|a1^b1~a2^b2~a3^b3\r");
}

TEST_F(TestSerializerHl7, testMSG_011_2)
{
    testhl7::MSG_011 msg;
    msg.msh.fieldSeparator = "|";
    msg.msh.encodingCharacters = "^~\\&";
    msg.msh.messageType.messageCode = "MSG";
    msg.msh.messageType.triggerEvent = "011";
    msg.msh.messageType.messageStructure = "MSG_011";
    msg.a02.faa.emplace_back("a1", "b1");
    msg.a02.faa.emplace_back("a2", "b2");
    msg.a02.faa.emplace_back("a3", "b3");
    msg.a02.fbb.emplace_back("c1", "d1", testhl7::FAA{});
    msg.a02.fbb.emplace_back("c2", "d2", testhl7::FAA{});
    msg.a02.fbb.emplace_back("c3", "d3", testhl7::FAA{});

    ParserStruct parser(*m_serializer, msg);
    parser.parseStruct();

    ASSERT_EQ(m_data, "MSH|^~\\&|||||||MSG^011^MSG_011\rA02|a1^b1~a2^b2~a3^b3|c1^d1~c2^d2~c3^d3\r");
}

TEST_F(TestSerializerHl7, testMSG_011_3)
{
    testhl7::MSG_011 msg;
    msg.msh.fieldSeparator = "|";
    msg.msh.encodingCharacters = "^~\\&";
    msg.msh.messageType.messageCode = "MSG";
    msg.msh.messageType.triggerEvent = "011";
    msg.msh.messageType.messageStructure = "MSG_011";
    msg.a02.faa.emplace_back("a1", "b1");
    msg.a02.faa.emplace_back("a2", "b2");
    msg.a02.faa.emplace_back("a3", "b3");
    msg.a02.fbb.emplace_back("c1", "d1", testhl7::FAA{ "a1", "b1" });
    msg.a02.fbb.emplace_back("c2", "d2", testhl7::FAA{ "a2", "b2" });
    msg.a02.fbb.emplace_back("c3", "d3", testhl7::FAA{ "a3", "b3" });

    ParserStruct parser(*m_serializer, msg);
    parser.parseStruct();

    ASSERT_EQ(m_data, "MSH|^~\\&|||||||MSG^011^MSG_011\rA02|a1^b1~a2^b2~a3^b3|c1^d1^a1&b1~c2^d2^a2&b2~c3^d3^a3&b3\r");
}

TEST_F(TestSerializerHl7, testMSG_011_4)
{
    testhl7::MSG_011 msg;
    msg.msh.fieldSeparator = "|";
    msg.msh.encodingCharacters = "^~\\&";
    msg.msh.messageType.messageCode = "MSG";
    msg.msh.messageType.triggerEvent = "011";
    msg.msh.messageType.messageStructure = "MSG_011";
    msg.a02.faa.emplace_back("a1", "b1");
    msg.a02.faa.emplace_back("a2", "b2");
    msg.a02.faa.emplace_back("a3", "b3");
    msg.a02.fbb.emplace_back("c1", "d1", testhl7::FAA{ "a1", "b1" });
    msg.a02.fbb.emplace_back("c2", "d2", testhl7::FAA{ "a2", "b2" });
    msg.a02.fbb.emplace_back("c3", "d3", testhl7::FAA{ "a3", "b3" });
    msg.a02.sfaa.a = "a";
    msg.a02.sfaa.b = "b";
    msg.a02.sfbb.a = "c";
    msg.a02.sfbb.b = "d";
    msg.a02.sfbb.faa.a = "a";
    msg.a02.sfbb.faa.b = "b";

    ParserStruct parser(*m_serializer, msg);
    parser.parseStruct();

    ASSERT_EQ(m_data, "MSH|^~\\&|||||||MSG^011^MSG_011\rA02|a1^b1~a2^b2~a3^b3|c1^d1^a1&b1~c2^d2^a2&b2~c3^d3^a3&b3|a^b|c^d^a&b\r");
}
