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


#include "finalmq/serializehl7/ParserHl7.h"
#include "finalmq/serializestruct/SerializerStruct.h"
#include "finalmq/metadata/MetaData.h"
#include "MockIParserVisitor.h"
#include "testhl7.fmq.h"

#include <cmath>
//#include <thread>
//#include <chrono>

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::ElementsAreArray;

using namespace finalmq;




class TestParserHl7 : public testing::Test
{
public:

protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};





TEST_F(TestParserHl7, testSSU_U03)
{
    testhl7::SSU_U03 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "SSU";
    cmp.msh.messageType.triggerEvent = "U03";
    cmp.msh.messageType.messageStructure = "SSU_U03";
    cmp.msh.countryCode = "de";
    cmp.uac = std::make_shared<testhl7::UAC>();
    cmp.uac->userAuthenticationCredential.typeOfData = testhl7::MimeTypes::MimeMultipartPackage;
    cmp.sft.resize(3);
    cmp.sft[0].softwareBinaryId = "world";
    cmp.specimen_container.resize(4);
    cmp.specimen_container[0].sac.positionInTray.value1 = "hey";
    cmp.specimen_container[0].sac.specimenSource = "hh";
    cmp.specimen_container[0].sac.carrierIdentifier.entityIdentifier = "uu";
    cmp.specimen_container[0].sac.carrierIdentifier.universalId = "bbb";
    cmp.specimen_container[0].obx.resize(2);
    cmp.specimen_container[0].specimen.resize(3);
    cmp.specimen_container[0].specimen[0].spm.accessionId.resize(1);
    cmp.specimen_container[0].specimen[0].spm.accessionId[0].idNumber = "ggg";
    cmp.specimen_container[0].specimen[0].spm.containerCondition.alternateText = "tt";
    cmp.specimen_container[0].specimen[0].spm.containerCondition.nameOfAlternateCodingSystem = testhl7::CodingSystem::WhoAdverseReactionTerms;
    cmp.specimen_container[0].specimen[0].obx.resize(5);

    testhl7::SSU_U03 msg;
    std::string data = "MSH|^~\\&|||||||SSU^U03^SSU_U03||||||||de\rSFT||||world\rSFT\rSFT\rUAC||^multipart\rEQU\rSAC||||||hh||||uu^^bbb||||hey\rOBX\rOBX\rSPM||||||||||||||||||||||||||||^^^^tt^ART||ggg\rOBX\rOBX\rOBX\rOBX\rOBX\rSPM\rSPM\rSAC\rSAC\rSAC\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.data());
    parser.parseStruct("testhl7.SSU_U03");

    ASSERT_EQ(cmp, msg);
}


TEST_F(TestParserHl7, testMSG_001)
{
    testhl7::MSG_001 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "001";
    cmp.msh.messageType.messageStructure = "MSG_001";
    cmp.a.a = "a";

    testhl7::MSG_001 msg;
    std::string data = "MSH|^~\\&|||||||MSG^001^MSG_001\rAAA|a\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_001");

    ASSERT_EQ(cmp, msg);
}

TEST_F(TestParserHl7, testMSG_002_1)
{
    testhl7::MSG_002 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "002";
    cmp.msh.messageType.messageStructure = "MSG_002";
    cmp.a = std::make_shared<testhl7::AAA>("a");
    cmp.b.b = "b";

    testhl7::MSG_002 msg;
    std::string data = "MSH|^~\\&|||||||MSG^002^MSG_002\rAAA|a\rBBB|b\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_002");

    ASSERT_EQ(cmp, msg);
}

TEST_F(TestParserHl7, testMSG_002_2)
{
    testhl7::MSG_002 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "002";
    cmp.msh.messageType.messageStructure = "MSG_002";
    cmp.b.b = "b";

    testhl7::MSG_002 msg;
    std::string data = "MSH|^~\\&|||||||MSG^002^MSG_002\rBBB|b\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_002");

    ASSERT_EQ(cmp, msg);
}

TEST_F(TestParserHl7, testMSG_003_1)
{
    testhl7::MSG_003 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "003";
    cmp.msh.messageType.messageStructure = "MSG_003";
    cmp.c.a.a = "a";
    cmp.b.b = "b";

    testhl7::MSG_003 msg;
    std::string data = "MSH|^~\\&|||||||MSG^003^MSG_003\rAAA|a\rBBB|b\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_003");

    ASSERT_EQ(cmp, msg);
}

TEST_F(TestParserHl7, testMSG_003_2)
{
    testhl7::MSG_003 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "003";
    cmp.msh.messageType.messageStructure = "MSG_003";
    cmp.c.a.a = "a";
    cmp.b.b = "b";

    testhl7::MSG_003 msg;
    std::string data = "MSH|^~\\&|||||||MSG^003^MSG_003\rDUM\rAAA|a\rDUM\rBBB|b\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_003");

    ASSERT_EQ(cmp, msg);
}

TEST_F(TestParserHl7, testMSG_004_1)
{
    testhl7::MSG_004 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "004";
    cmp.msh.messageType.messageStructure = "MSG_004";
    cmp.c = std::make_shared<testhl7::GROUP_1>();
    cmp.c->a.a = "a";
    cmp.b.b = "b";

    testhl7::MSG_004 msg;
    std::string data = "MSH|^~\\&|||||||MSG^004^MSG_004\rAAA|a\rDUM\rBBB|b\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_004");

    ASSERT_EQ(cmp, msg);
}

TEST_F(TestParserHl7, testMSG_004_2)
{
    testhl7::MSG_004 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "004";
    cmp.msh.messageType.messageStructure = "MSG_004";
    cmp.b.b = "b";

    testhl7::MSG_004 msg;
    std::string data = "MSH|^~\\&|||||||MSG^004^MSG_004\rBBB|b\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_004");

    ASSERT_EQ(cmp, msg);
}

TEST_F(TestParserHl7, testMSG_004_3)
{
    testhl7::MSG_004 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "004";
    cmp.msh.messageType.messageStructure = "MSG_004";
    cmp.b.b = "b";

    testhl7::MSG_004 msg;
    std::string data = "MSH|^~\\&|||||||MSG^004^MSG_004\rDUM\rDUM\rBBB|b\rDUM\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_004");

    ASSERT_EQ(cmp, msg);
}

TEST_F(TestParserHl7, testMSG_005_1)
{
    testhl7::MSG_005 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "005";
    cmp.msh.messageType.messageStructure = "MSG_005";
    cmp.c = std::make_shared<testhl7::GROUP_2>();
    cmp.c->b = std::make_shared<testhl7::BBB>();
    cmp.c->b->b = "b1";
    cmp.b.b = "b2";

    testhl7::MSG_005 msg;
    std::string data = "MSH|^~\\&|||||||MSG^005^MSG_005\rDUM\rDUM\rBBB|b1\rBBB|b2\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_005");

    ASSERT_EQ(cmp, msg);
}

TEST_F(TestParserHl7, testMSG_006_1)
{
    testhl7::MSG_006 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "006";
    cmp.msh.messageType.messageStructure = "MSG_006";
    cmp.c.push_back(testhl7::GROUP_2());
    cmp.c[0].b = std::make_shared<testhl7::BBB>();
    auto b = cmp.c[0].b;
    ASSERT_NE(b, nullptr);
    b->b = "b1";
    cmp.d.d = "d";

    testhl7::MSG_006 msg;
    std::string data = "MSH|^~\\&|||||||MSG^006^MSG_006\rDUM\rDUM\rBBB|b1\rDDD|d\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_006");

    ASSERT_EQ(cmp, msg);
}

TEST_F(TestParserHl7, testMSG_006_2)
{
    testhl7::MSG_006 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "006";
    cmp.msh.messageType.messageStructure = "MSG_006";

    cmp.c.push_back(testhl7::GROUP_2());
    cmp.c[0].b = std::make_shared<testhl7::BBB>();
    auto b = cmp.c[0].b;
    ASSERT_NE(b, nullptr);
    b->b = "b";

    cmp.c.push_back(testhl7::GROUP_2());
    cmp.c[1].a = std::make_shared<testhl7::AAA>();
    auto a = cmp.c[1].a;
    ASSERT_NE(a, nullptr);
    a->a = "a";

    cmp.d.d = "d";

    testhl7::MSG_006 msg;
    std::string data = "MSH|^~\\&|||||||MSG^006^MSG_006\rDUM\rDUM\rBBB|b\rAAA|a\rDDD|d\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_006");

    ASSERT_EQ(cmp, msg);
}

TEST_F(TestParserHl7, testMSG_006_3)
{
    testhl7::MSG_006 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "006";
    cmp.msh.messageType.messageStructure = "MSG_006";

    cmp.c.push_back(testhl7::GROUP_2());
    cmp.c[0].a = std::make_shared<testhl7::AAA>();
    auto a = cmp.c[0].a;
    ASSERT_NE(a, nullptr);
    a->a = "a";

    cmp.c.push_back(testhl7::GROUP_2());
    cmp.c[1].b = std::make_shared<testhl7::BBB>();
    auto b = cmp.c[1].b;
    ASSERT_NE(b, nullptr);
    b->b = "b";

    cmp.d.d = "d";

    testhl7::MSG_006 msg;
    std::string data = "MSH|^~\\&|||||||MSG^006^MSG_006\rDUM\rDUM\rAAA|a\rBBB|b\rDDD|d\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_006");

    ASSERT_EQ(cmp, msg);
}

TEST_F(TestParserHl7, testArrayStruct_MSG_012_1)
{
    testhl7::MSG_012 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "012";
    cmp.msh.messageType.messageStructure = "MSG_012";

    cmp.a03.faa_arr.resize(1);
    cmp.a03.faa_arr[0].b = "b";

    testhl7::MSG_012 msg;
    std::string data = "MSH|^~\\&|||||||MSG^012^MSG_012\rA03|^b\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_012");

    ASSERT_EQ(cmp, msg);
}

TEST_F(TestParserHl7, testArrayStruct_MSG_012_2)
{
    testhl7::MSG_012 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "012";
    cmp.msh.messageType.messageStructure = "MSG_012";

    cmp.a03.faa_arr.resize(1);

    testhl7::MSG_012 msg;
    std::string data = "MSH|^~\\&|||||||MSG^012^MSG_012\rA03|^\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_012");

    ASSERT_EQ(cmp, msg);
}

TEST_F(TestParserHl7, testArrayStruct_MSG_012_3)
{
    testhl7::MSG_012 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "012";
    cmp.msh.messageType.messageStructure = "MSG_012";

    cmp.a03.faa_arr.resize(2);

    testhl7::MSG_012 msg;
    std::string data = "MSH|^~\\&|||||||MSG^012^MSG_012\rA03|~\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_012");

    ASSERT_EQ(cmp, msg);
}

TEST_F(TestParserHl7, testArrayString_MSG_012_1)
{
    testhl7::MSG_012 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "012";
    cmp.msh.messageType.messageStructure = "MSG_012";

    cmp.a03.sarr.resize(2);

    testhl7::MSG_012 msg;
    std::string data = "MSH|^~\\&|||||||MSG^012^MSG_012\rA03|||~\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_012");

    ASSERT_EQ(cmp, msg);
}

TEST_F(TestParserHl7, testArrayString_MSG_012_2)
{
    testhl7::MSG_012 cmp;
    cmp.msh.fieldSeparator = "|";
    cmp.msh.encodingCharacters = "^~\\&";
    cmp.msh.messageType.messageCode = "MSG";
    cmp.msh.messageType.triggerEvent = "012";
    cmp.msh.messageType.messageStructure = "MSG_012";

    cmp.a03.sarr.resize(2);
    cmp.a03.sarr[1] = "b";

    testhl7::MSG_012 msg;
    std::string data = "MSH|^~\\&|||||||MSG^012^MSG_012\rA03|||~b\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.c_str());
    parser.parseStruct("testhl7.MSG_012");

    ASSERT_EQ(cmp, msg);
}

