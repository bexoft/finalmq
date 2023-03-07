using System;
using System.Runtime;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Diagnostics;

using Xunit;
using Moq;

using finalmq;
using System.Text;
using System.Reflection;
using System.Reflection.Metadata;


namespace testfinalmq
{

    public class TestParserHl7 : IDisposable
    {
        public TestParserHl7()
        {
        }

        public void Dispose()
        {
        }

        [Fact]
        public void TestSSU_U03()
        {
            testhl7.SSU_U03 cmp = new testhl7.SSU_U03();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "SSU";
            cmp.msh.messageType.triggerEvent = "U03";
            cmp.msh.messageType.messageStructure = "SSU_U03";
            cmp.msh.countryCode = "de";
            cmp.uac = new testhl7.UAC();
            cmp.uac.userAuthenticationCredential.typeOfData = testhl7.MimeTypes.MimeMultipartPackage;
            cmp.sft.Add(new testhl7.SFT());
            cmp.sft.Add(new testhl7.SFT());
            cmp.sft.Add(new testhl7.SFT());
            cmp.sft[0].softwareBinaryId = "world";
            cmp.specimen_container.Add(new testhl7.SPECIMEN_CONTAINER_5());
            cmp.specimen_container.Add(new testhl7.SPECIMEN_CONTAINER_5());
            cmp.specimen_container.Add(new testhl7.SPECIMEN_CONTAINER_5());
            cmp.specimen_container.Add(new testhl7.SPECIMEN_CONTAINER_5());
            cmp.specimen_container[0].sac.positionInTray.value1 = "hey";
            cmp.specimen_container[0].sac.specimenSource = "hh";
            cmp.specimen_container[0].sac.carrierIdentifier.entityIdentifier = "uu";
            cmp.specimen_container[0].sac.carrierIdentifier.universalId = "bbb";
            cmp.specimen_container[0].obx.Add(new testhl7.OBX());
            cmp.specimen_container[0].obx.Add(new testhl7.OBX());
            cmp.specimen_container[0].specimen.Add(new testhl7.SPECIMEN_17());
            cmp.specimen_container[0].specimen.Add(new testhl7.SPECIMEN_17());
            cmp.specimen_container[0].specimen.Add(new testhl7.SPECIMEN_17());
            cmp.specimen_container[0].specimen[0].spm.accessionId.Add(new testhl7.CX());
            cmp.specimen_container[0].specimen[0].spm.accessionId[0].idNumber = "ggg";
            cmp.specimen_container[0].specimen[0].spm.containerCondition.alternateText = "tt";
            cmp.specimen_container[0].specimen[0].spm.containerCondition.nameOfAlternateCodingSystem = testhl7.CodingSystem.WhoAdverseReactionTerms;
            cmp.specimen_container[0].specimen[0].obx.Add(new testhl7.OBX());
            cmp.specimen_container[0].specimen[0].obx.Add(new testhl7.OBX());
            cmp.specimen_container[0].specimen[0].obx.Add(new testhl7.OBX());
            cmp.specimen_container[0].specimen[0].obx.Add(new testhl7.OBX());
            cmp.specimen_container[0].specimen[0].obx.Add(new testhl7.OBX());

            testhl7.SSU_U03 msg = new testhl7.SSU_U03();
            string data = "MSH|^~\\&|||||||SSU^U03^SSU_U03||||||||de\rSFT||||world\rSFT\rSFT\rUAC||^multipart\rEQU\rSAC||||||hh||||uu^^bbb||||hey\rOBX\rOBX\rSPM||||||||||||||||||||||||||||^^^^tt^ART||ggg\rOBX\rOBX\rOBX\rOBX\rOBX\rSPM\rSPM\rSAC\rSAC\rSAC\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.SSU_U03");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestMSG_001()
        {
            testhl7.MSG_001 cmp = new testhl7.MSG_001();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "001";
            cmp.msh.messageType.messageStructure = "MSG_001";
            cmp.a.a = "a";

            testhl7.MSG_001 msg = new testhl7.MSG_001();
            string data = "MSH|^~\\&|||||||MSG^001^MSG_001\rAAA|a\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_001");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestMSG_002_1()
        {
            testhl7.MSG_002 cmp = new testhl7.MSG_002();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "002";
            cmp.msh.messageType.messageStructure = "MSG_002";
            cmp.a = new testhl7.AAA("a");
            cmp.b.b = "b";

            testhl7.MSG_002 msg = new testhl7.MSG_002();
            string data = "MSH|^~\\&|||||||MSG^002^MSG_002\rAAA|a\rBBB|b\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_002");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestMSG_002_2()
        {
            testhl7.MSG_002 cmp = new testhl7.MSG_002();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "002";
            cmp.msh.messageType.messageStructure = "MSG_002";
            cmp.b.b = "b";

            testhl7.MSG_002 msg = new testhl7.MSG_002();
            string data = "MSH|^~\\&|||||||MSG^002^MSG_002\rBBB|b\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_002");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestMSG_003_1()
        {
            testhl7.MSG_003 cmp = new testhl7.MSG_003();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "003";
            cmp.msh.messageType.messageStructure = "MSG_003";
            cmp.c.a.a = "a";
            cmp.b.b = "b";

            testhl7.MSG_003 msg = new testhl7.MSG_003();
            string data = "MSH|^~\\&|||||||MSG^003^MSG_003\rAAA|a\rBBB|b\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_003");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestMSG_003_2()
        {
            testhl7.MSG_003 cmp = new testhl7.MSG_003();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "003";
            cmp.msh.messageType.messageStructure = "MSG_003";
            cmp.c.a.a = "a";
            cmp.b.b = "b";

            testhl7.MSG_003 msg = new testhl7.MSG_003();
            string data = "MSH|^~\\&|||||||MSG^003^MSG_003\rDUM\rAAA|a\rDUM\rBBB|b\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_003");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestMSG_004_1()
        {
            testhl7.MSG_004 cmp = new testhl7.MSG_004();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "004";
            cmp.msh.messageType.messageStructure = "MSG_004";
            cmp.c = new testhl7.GROUP_1();
            cmp.c.a.a = "a";
            cmp.b.b = "b";

            testhl7.MSG_004 msg = new testhl7.MSG_004();
            string data = "MSH|^~\\&|||||||MSG^004^MSG_004\rAAA|a\rDUM\rBBB|b\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_004");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestMSG_004_2()
        {
            testhl7.MSG_004 cmp = new testhl7.MSG_004();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "004";
            cmp.msh.messageType.messageStructure = "MSG_004";
            cmp.b.b = "b";

            testhl7.MSG_004 msg = new testhl7.MSG_004();
            string data = "MSH|^~\\&|||||||MSG^004^MSG_004\rBBB|b\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_004");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestMSG_004_3()
        {
            testhl7.MSG_004 cmp = new testhl7.MSG_004();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "004";
            cmp.msh.messageType.messageStructure = "MSG_004";
            cmp.b.b = "b";

            testhl7.MSG_004 msg = new testhl7.MSG_004();
            string data = "MSH|^~\\&|||||||MSG^004^MSG_004\rDUM\rDUM\rBBB|b\rDUM\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_004");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestMSG_005_1()
        {
            testhl7.MSG_005 cmp = new testhl7.MSG_005();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "005";
            cmp.msh.messageType.messageStructure = "MSG_005";
            cmp.c = new testhl7.GROUP_2();
            cmp.c.b = new testhl7.BBB();
            cmp.c.b.b = "b1";
            cmp.b.b = "b2";

            testhl7.MSG_005 msg = new testhl7.MSG_005();
            string data = "MSH|^~\\&|||||||MSG^005^MSG_005\rDUM\rDUM\rBBB|b1\rBBB|b2\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_005");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestMSG_006_1()
        {
            testhl7.MSG_006 cmp = new testhl7.MSG_006();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "006";
            cmp.msh.messageType.messageStructure = "MSG_006";
            cmp.c.Add(new testhl7.GROUP_2());
            cmp.c[0].b = new testhl7.BBB();
            var b = cmp.c[0].b;
            Debug.Assert(b != null);
            b.b = "b1";
            cmp.d.d = "d";

            testhl7.MSG_006 msg = new testhl7.MSG_006();
            string data = "MSH|^~\\&|||||||MSG^006^MSG_006\rDUM\rDUM\rBBB|b1\rDDD|d\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_006");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestMSG_006_2()
        {
            testhl7.MSG_006 cmp = new testhl7.MSG_006();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "006";
            cmp.msh.messageType.messageStructure = "MSG_006";

            cmp.c.Add(new testhl7.GROUP_2());
            cmp.c[0].b = new testhl7.BBB();
            var b = cmp.c[0].b;
            Debug.Assert(b != null);
            b.b = "b";

            cmp.c.Add(new testhl7.GROUP_2());
            cmp.c[1].a = new testhl7.AAA();
            var a = cmp.c[1].a;
            Debug.Assert(a != null);
            a.a = "a";

            cmp.d.d = "d";

            testhl7.MSG_006 msg = new testhl7.MSG_006();
            string data = "MSH|^~\\&|||||||MSG^006^MSG_006\rDUM\rDUM\rBBB|b\rAAA|a\rDDD|d\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_006");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestMSG_006_3()
        {
            testhl7.MSG_006 cmp = new testhl7.MSG_006();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "006";
            cmp.msh.messageType.messageStructure = "MSG_006";

            cmp.c.Add(new testhl7.GROUP_2());
            cmp.c[0].a = new testhl7.AAA();
            var a = cmp.c[0].a;
            Debug.Assert(a != null);
            a.a = "a";

            cmp.c.Add(new testhl7.GROUP_2());
            cmp.c[1].b = new testhl7.BBB();
            var b = cmp.c[1].b;
            Debug.Assert(b != null);
            b.b = "b";

            cmp.d.d = "d";

            testhl7.MSG_006 msg = new testhl7.MSG_006();
            string data = "MSH|^~\\&|||||||MSG^006^MSG_006\rDUM\rDUM\rAAA|a\rBBB|b\rDDD|d\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_006");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestArrayStruct_MSG_010_1()
        {
            testhl7.MSG_010 cmp = new testhl7.MSG_010();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "010";
            cmp.msh.messageType.messageStructure = "MSG_010";

            cmp.a01.a.Add("");
            cmp.a01.a.Add("a2");

            testhl7.MSG_010 msg = new testhl7.MSG_010();
            string data = "MSH|^~\\&|||||||MSG^010^MSG_010\rA01|^~a2\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_010");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestArrayStruct_MSG_010_2()
        {
            testhl7.MSG_010 cmp = new testhl7.MSG_010();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "010";
            cmp.msh.messageType.messageStructure = "MSG_010";

            testhl7.MSG_010 msg = new testhl7.MSG_010();
            string data = "MSH|^~\\&|||||||MSG^010^MSG_010\rA01||\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_010");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestArrayStruct_MSG_011_1()
        {
            testhl7.MSG_011 cmp = new testhl7.MSG_011();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "011";
            cmp.msh.messageType.messageStructure = "MSG_011";

            cmp.a02.faa.Add(new testhl7.FAA());
            cmp.a02.faa.Add(new testhl7.FAA());
            cmp.a02.faa[0].a = "a";
            cmp.a02.faa[1].a = "b";

            testhl7.MSG_011 msg = new testhl7.MSG_011();
            string data = "MSH|^~\\&|||||||MSG^011^MSG_011\rA02|a&~b\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_011");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestArrayStruct_MSG_012_1()
        {
            testhl7.MSG_012 cmp = new testhl7.MSG_012();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "012";
            cmp.msh.messageType.messageStructure = "MSG_012";

            cmp.a03.faa_arr.Add(new testhl7.FAA());
            cmp.a03.faa_arr[0].b = "b";

            testhl7.MSG_012 msg = new testhl7.MSG_012();
            string data = "MSH|^~\\&|||||||MSG^012^MSG_012\rA03|^b\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_012");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestArrayStruct_MSG_012_2()
        {
            testhl7.MSG_012 cmp = new testhl7.MSG_012();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "012";
            cmp.msh.messageType.messageStructure = "MSG_012";

            cmp.a03.faa_arr.Add(new testhl7.FAA());

            testhl7.MSG_012 msg = new testhl7.MSG_012();
            string data = "MSH|^~\\&|||||||MSG^012^MSG_012\rA03|^\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_012");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestArrayStruct_MSG_012_3()
        {
            testhl7.MSG_012 cmp = new testhl7.MSG_012();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "012";
            cmp.msh.messageType.messageStructure = "MSG_012";

            cmp.a03.faa_arr.Add(new testhl7.FAA());
            cmp.a03.faa_arr.Add(new testhl7.FAA());

            testhl7.MSG_012 msg = new testhl7.MSG_012();
            string data = "MSH|^~\\&|||||||MSG^012^MSG_012\rA03|~\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_012");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestArrayString_MSG_012_1()
        {
            testhl7.MSG_012 cmp = new testhl7.MSG_012();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "012";
            cmp.msh.messageType.messageStructure = "MSG_012";

            cmp.a03.sarr.Add("");
            cmp.a03.sarr.Add("");

            testhl7.MSG_012 msg = new testhl7.MSG_012();
            string data = "MSH|^~\\&|||||||MSG^012^MSG_012\rA03|||~\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_012");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestArrayString_MSG_012_2()
        {
            testhl7.MSG_012 cmp = new testhl7.MSG_012();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "012";
            cmp.msh.messageType.messageStructure = "MSG_012";

            cmp.a03.sarr.Add("");
            cmp.a03.sarr.Add("b");

            testhl7.MSG_012 msg = new testhl7.MSG_012();
            string data = "MSH|^~\\&|||||||MSG^012^MSG_012\rA03|||~b\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_012");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestArrayString_MSG_012_3()
        {
            testhl7.MSG_012 cmp = new testhl7.MSG_012();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "012";
            cmp.msh.messageType.messageStructure = "MSG_012";

            cmp.a03.sarr.Add("a");
            cmp.a03.sarr.Add("b");

            testhl7.MSG_012 msg = new testhl7.MSG_012();
            string data = "MSH|^~\\&|||||||MSG^012^MSG_012\rA03|||a^c~b\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_012");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void TestArrayString_MSG_012_4()
        {
            testhl7.MSG_012 cmp = new testhl7.MSG_012();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "012";
            cmp.msh.messageType.messageStructure = "MSG_012";

            cmp.a03.faa.a = "a";
            cmp.a03.faa.b = "b";

            testhl7.MSG_012 msg = new testhl7.MSG_012();
            string data = "MSH|^~\\&|||||||MSG^012^MSG_012\rA03||a^b~c^d~e^f\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_012");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void Test_MSG_013_1()
        {
            testhl7.MSG_013 cmp = new testhl7.MSG_013();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "013";
            cmp.msh.messageType.messageStructure = "MSG_013";

            cmp.a04.a = "a";
            cmp.a04.b = "b";

            testhl7.MSG_013 msg = new testhl7.MSG_013();
            string data = "MSH|^~\\&|||||||MSG^013^MSG_013\rA04||a~c|b\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_013");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void Test_MSG_013_2()
        {
            testhl7.MSG_013 cmp = new testhl7.MSG_013();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "013";
            cmp.msh.messageType.messageStructure = "MSG_013";

            cmp.a04.fff.a = "a";
            cmp.a04.fff.b = "";

            testhl7.MSG_013 msg = new testhl7.MSG_013();
            string data = "MSH|^~\\&|||||||MSG^013^MSG_013\rA04|a~c^d\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_013");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void Test_MSG_013_3()
        {
            testhl7.MSG_013 cmp = new testhl7.MSG_013();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "013";
            cmp.msh.messageType.messageStructure = "MSG_013";

            cmp.a04.fff.a = "a";
            cmp.a04.fff.b = "b";
            cmp.a04.fff.faa.a = "a";
            cmp.a04.fff.faa.b = "";

            testhl7.MSG_013 msg = new testhl7.MSG_013();
            string data = "MSH|^~\\&|||||||MSG^013^MSG_013\rA04|a^b^a~c\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_013");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void Test_MSG_013_4()
        {
            testhl7.MSG_013 cmp = new testhl7.MSG_013();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "013";
            cmp.msh.messageType.messageStructure = "MSG_013";

            cmp.a04.fff.a = "a";
            cmp.a04.fff.b = "b";
            cmp.a04.fff.faa.a = "a";
            cmp.a04.fff.faa.b = "";

            testhl7.MSG_013 msg = new testhl7.MSG_013();
            string data = "MSH|^~\\&|||||||MSG^013^MSG_013\rA04|a^b^a~\rTST";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_013");

            Debug.Assert(cmp.Equals(msg));
        }

        [Fact]
        public void Test_MSG_014_1()
        {
            testhl7.MSG_014 cmp = new testhl7.MSG_014();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "MSG";
            cmp.msh.messageType.triggerEvent = "014";
            cmp.msh.messageType.messageStructure = "MSG_014";

            cmp.a05.fff.Add(new testhl7.FFF());
            cmp.a05.fff[0].a = "a";
            cmp.a05.fff[0].b = "b";
            cmp.a05.fff[0].faa.a = "fa";
            cmp.a05.fff[0].faa.b = "fb";

            cmp.a05.fff.Add(new testhl7.FFF());
            cmp.a05.fff[1].a = "c";
            cmp.a05.fff[1].b = "d";
            cmp.a05.fff[1].faa.a = "fc";
            cmp.a05.fff[1].faa.b = "fd";

            testhl7.MSG_014 msg = new testhl7.MSG_014();
            string data = "MSH|^~\\&|||||||MSG^014^MSG_014\rA05|a^b^fa&fb~c^d^fc&fd\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("testhl7.MSG_014");

            Debug.Assert(cmp.Equals(msg));
        }

    }


}

