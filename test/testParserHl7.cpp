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
#include "hl7.fmq.h"

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
    hl7::SSU_U03 cmp;
    cmp.msh.countryCode = "de";
    cmp.uac = std::make_shared<hl7::UAC>();
    cmp.uac->userAuthenticationCredential.typeOfData = "hello";
    cmp.sft.resize(3);
    cmp.sft[0].softwareBinaryID = "world";
    cmp.sac.resize(4);
    cmp.sac[0].sac.positionInTray = "hey";
    cmp.sac[0].sac.specimenSource = "hh";
    cmp.sac[0].sac.carrierId.entityIdentifier = "uu";
    cmp.sac[0].sac.carrierId.universalId = "bbb";
    cmp.sac[0].obx.resize(2);
    cmp.sac[0].nte.resize(1);
    cmp.sac[0].spm.resize(3);
    cmp.sac[0].spm[0].spm.accessionId = "ggg";
    cmp.sac[0].spm[0].spm.containerCondition.alternateText = "tt";
    cmp.sac[0].spm[0].spm.containerCondition.nameOfAlternateCodingSystem = "cc";
    cmp.sac[0].spm[0].obx.resize(5);

    hl7::SSU_U03 msg;
    std::string data = "MSH|^~\\&|||||||SSU^U03^SSU_U03||||||||de\rSFT|||world\rSFT\rSFT\rUAC||^hello\rEQU\rSAC||||||hh||||uu^^bbb||||hey\rOBX\rOBX\rNTE\rSPM||||||||||||||||||||||||||||^^^^tt^cc||ggg\rOBX\rOBX\rOBX\rOBX\rOBX\rSPM\rSPM\rSAC\rSAC\rSAC\r";

    SerializerStruct serializer(msg);
    ParserHl7 parser(serializer, data.data());
    parser.parseStruct("hl7.SSU_U03");

}

