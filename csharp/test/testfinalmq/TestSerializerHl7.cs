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

    public class TestSerializerHl7 : IDisposable
    {
        static readonly int MAX_BLOCK_SIZE = 5000;

        SerializerHl7 m_serializer;
        byte[] m_data = new byte[MAX_BLOCK_SIZE];
        int m_size = MAX_BLOCK_SIZE;
        BufferRef m_bufferRef;
        Mock<IZeroCopyBuffer>? m_mockBuffer = null;

        public TestSerializerHl7()
        {
            m_mockBuffer = new Mock<IZeroCopyBuffer>();
            m_bufferRef = new BufferRef(m_data, 0, m_data.Length);

            m_mockBuffer.Setup(x => x.AddBuffer(MAX_BLOCK_SIZE, It.IsAny<int>())).Returns(m_bufferRef);
            m_mockBuffer.Setup(x => x.DownsizeLastBuffer(It.IsAny<int>())).Callback((int size) => {
                m_size = size;
            });
            m_mockBuffer.Setup(x => x.RemainingSize).Returns(0);

            m_serializer = new SerializerHl7(m_mockBuffer.Object, MAX_BLOCK_SIZE);
        }

        public void Dispose()
        {
            if (m_mockBuffer != null)
            {
                m_mockBuffer.Verify(x => x.AddBuffer(MAX_BLOCK_SIZE, It.IsAny<int>()), Times.Once);
                m_mockBuffer.Verify(x => x.DownsizeLastBuffer(It.IsAny<int>()), Times.Once);
                m_mockBuffer.Verify(x => x.RemainingSize, Times.Once);
            }
        }
        /*
                [Fact]
                public void TestSSU_U03()
                {
                    hl7.SSU_U03 msg = new hl7.SSU_U03();
                    msg.msh.countryCode = "de";
                    msg.uac = new hl7.UAC();
                    msg.uac.userAuthenticationCredential.typeofData = "hello";
                    msg.sft.Add(new hl7.SFT());
                    msg.sft.Add(new hl7.SFT());
                    msg.sft.Add(new hl7.SFT());
                    msg.sft[0].softwareBinaryID = "world";
                    msg.sac.Add(new hl7.SAC_GROUP());
                    msg.sac.Add(new hl7.SAC_GROUP());
                    msg.sac.Add(new hl7.SAC_GROUP());
                    msg.sac.Add(new hl7.SAC_GROUP());
                    msg.sac[0].sac.positioninTray.value1 = "hey";
                    msg.sac[0].sac.specimenSource = "hh";
                    msg.sac[0].sac.carrierIdentifier.entityIdentifier = "uu";
                    msg.sac[0].sac.carrierIdentifier.universalID = "bbb";
                    msg.sac[0].obx.Add(new hl7.OBX());
                    msg.sac[0].obx.Add(new hl7.OBX());
                    msg.sac[0].nte.Add(new hl7.NTE());
                    msg.sac[0].spm.Add(new hl7.SPM_GROUP());
                    msg.sac[0].spm.Add(new hl7.SPM_GROUP());
                    msg.sac[0].spm.Add(new hl7.SPM_GROUP());
                    msg.sac[0].spm[0].spm.accessionID.iDNumber = "ggg";
                    msg.sac[0].spm[0].spm.containerCondition.alternateText = "tt";
                    msg.sac[0].spm[0].spm.containerCondition.nameofAlternateCodingSystem = "cc";
                    msg.sac[0].spm[0].obx.Add(new hl7.OBX());
                    msg.sac[0].spm[0].obx.Add(new hl7.OBX());
                    msg.sac[0].spm[0].obx.Add(new hl7.OBX());
                    msg.sac[0].spm[0].obx.Add(new hl7.OBX());
                    msg.sac[0].spm[0].obx.Add(new hl7.OBX());

                    ParserStruct parser = new ParserStruct(m_serializer, msg);
                    parser.ParseStruct();

                    string str = Encoding.UTF8.GetString(m_data, 0, m_size);
                    Debug.Assert(str == "MSH|^~\\&|||||||SSU^U03^SSU_U03||||||||de\rSFT|||world\rSFT\rSFT\rUAC||^hello\rEQU\rSAC||||||hh||||uu^^bbb||||hey\rOBX\rOBX\rNTE\rSPM||||||||||||||||||||||||||||^^^^tt^cc||ggg\rOBX\rOBX\rOBX\rOBX\rOBX\rSPM\rSPM\rSAC\rSAC\rSAC\r");
                }
        */

        [Fact]
        public void TestMSG_001()
        {
            testhl7.MSG_001 msg = new testhl7.MSG_001();
            msg.msh.fieldSeparator = "|";
            msg.msh.encodingCharacters = "^~\\&";
            msg.msh.messageType.messageCode = "MSG";
            msg.msh.messageType.triggerEvent = "001";
            msg.msh.messageType.messageStructure = "MSG_001";
            msg.a.a = "a";

            ParserStruct parser = new ParserStruct(m_serializer, msg);
            parser.ParseStruct();

            string str = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(str == "MSH|^~\\&|||||||MSG^001^MSG_001\rAAA|a\r");
        }

        [Fact]
        public void TestMSG_007()
        {
            testhl7.MSG_007 msg = new testhl7.MSG_007();
            msg.msh.fieldSeparator = "|";
            msg.msh.encodingCharacters = "^~\\&";
            msg.msh.messageType.messageCode = "MSG";
            msg.msh.messageType.triggerEvent = "007";
            msg.msh.messageType.messageStructure = "MSG_007";
            msg.e.e.a = "a";
            msg.e.e.b = "b";

            ParserStruct parser = new ParserStruct(m_serializer, msg);
            parser.ParseStruct();

            string str = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(str == "MSH|^~\\&|||||||MSG^007^MSG_007\rEEE|a^b\r");
        }

        [Fact]
        public void TestMSG_010()
        {
            testhl7.MSG_010 msg = new testhl7.MSG_010();
            msg.msh.fieldSeparator = "|";
            msg.msh.encodingCharacters = "^~\\&";
            msg.msh.messageType.messageCode = "MSG";
            msg.msh.messageType.triggerEvent = "010";
            msg.msh.messageType.messageStructure = "MSG_010";
            msg.a01.a.Add("a1");
            msg.a01.a.Add("a2");
            msg.a01.a.Add("a3");

            ParserStruct parser = new ParserStruct(m_serializer, msg);
            parser.ParseStruct();

            string str = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(str == "MSH|^~\\&|||||||MSG^010^MSG_010\rA01|a1~a2~a3\r");
        }

        [Fact]
        public void TestMSG_011_1()
        {
            testhl7.MSG_011 msg = new testhl7.MSG_011();
            msg.msh.fieldSeparator = "|";
            msg.msh.encodingCharacters = "^~\\&";
            msg.msh.messageType.messageCode = "MSG";
            msg.msh.messageType.triggerEvent = "011";
            msg.msh.messageType.messageStructure = "MSG_011";
            msg.a02.faa.Add(new testhl7.FAA("a1", "b1"));
            msg.a02.faa.Add(new testhl7.FAA("a2", "b2"));
            msg.a02.faa.Add(new testhl7.FAA("a3", "b3"));

            ParserStruct parser = new ParserStruct(m_serializer, msg);
            parser.ParseStruct();

            string str = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(str == "MSH|^~\\&|||||||MSG^011^MSG_011\rA02|a1^b1~a2^b2~a3^b3\r");
        }

        [Fact]
        public void TestMSG_011_2()
        {
            testhl7.MSG_011 msg = new testhl7.MSG_011();
            msg.msh.fieldSeparator = "|";
            msg.msh.encodingCharacters = "^~\\&";
            msg.msh.messageType.messageCode = "MSG";
            msg.msh.messageType.triggerEvent = "011";
            msg.msh.messageType.messageStructure = "MSG_011";
            msg.a02.faa.Add(new testhl7.FAA("a1", "b1"));
            msg.a02.faa.Add(new testhl7.FAA("a2", "b2"));
            msg.a02.faa.Add(new testhl7.FAA("a3", "b3"));
            msg.a02.fbb.Add(new testhl7.FBB("c1", "d1", new testhl7.FAA()));
            msg.a02.fbb.Add(new testhl7.FBB("c2", "d2", new testhl7.FAA()));
            msg.a02.fbb.Add(new testhl7.FBB("c3", "d3", new testhl7.FAA()));

            ParserStruct parser = new ParserStruct(m_serializer, msg);
            parser.ParseStruct();

            string str = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(str == "MSH|^~\\&|||||||MSG^011^MSG_011\rA02|a1^b1~a2^b2~a3^b3|c1^d1~c2^d2~c3^d3\r");
        }

        [Fact]
        public void TestMSG_011_3()
        {
            testhl7.MSG_011 msg = new testhl7.MSG_011();
            msg.msh.fieldSeparator = "|";
            msg.msh.encodingCharacters = "^~\\&";
            msg.msh.messageType.messageCode = "MSG";
            msg.msh.messageType.triggerEvent = "011";
            msg.msh.messageType.messageStructure = "MSG_011";
            msg.a02.faa.Add(new testhl7.FAA("a1", "b1"));
            msg.a02.faa.Add(new testhl7.FAA("a2", "b2"));
            msg.a02.faa.Add(new testhl7.FAA("a3", "b3"));
            msg.a02.fbb.Add(new testhl7.FBB("c1", "d1", new testhl7.FAA("a1", "b1")));
            msg.a02.fbb.Add(new testhl7.FBB("c2", "d2", new testhl7.FAA("a2", "b2")));
            msg.a02.fbb.Add(new testhl7.FBB("c3", "d3", new testhl7.FAA("a3", "b3")));

            ParserStruct parser = new ParserStruct(m_serializer, msg);
            parser.ParseStruct();

            string str = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(str == "MSH|^~\\&|||||||MSG^011^MSG_011\rA02|a1^b1~a2^b2~a3^b3|c1^d1^a1&b1~c2^d2^a2&b2~c3^d3^a3&b3\r");
        }

        [Fact]
        public void TestMSG_011_4()
        {
            testhl7.MSG_011 msg = new testhl7.MSG_011();
            msg.msh.fieldSeparator = "|";
            msg.msh.encodingCharacters = "^~\\&";
            msg.msh.messageType.messageCode = "MSG";
            msg.msh.messageType.triggerEvent = "011";
            msg.msh.messageType.messageStructure = "MSG_011";
            msg.a02.faa.Add(new testhl7.FAA("a1", "b1"));
            msg.a02.faa.Add(new testhl7.FAA("a2", "b2"));
            msg.a02.faa.Add(new testhl7.FAA("a3", "b3"));
            msg.a02.fbb.Add(new testhl7.FBB("c1", "d1", new testhl7.FAA("a1", "b1")));
            msg.a02.fbb.Add(new testhl7.FBB("c2", "d2", new testhl7.FAA("a2", "b2")));
            msg.a02.fbb.Add(new testhl7.FBB("c3", "d3", new testhl7.FAA("a3", "b3")));
            msg.a02.sfaa.a = "a";
            msg.a02.sfaa.b = "b";
            msg.a02.sfbb.a = "c";
            msg.a02.sfbb.b = "d";
            msg.a02.sfbb.faa.a = "a";
            msg.a02.sfbb.faa.b = "b";

            ParserStruct parser = new ParserStruct(m_serializer, msg);
            parser.ParseStruct();

            string str = Encoding.UTF8.GetString(m_data, 0, m_size);
            Debug.Assert(str == "MSH|^~\\&|||||||MSG^011^MSG_011\rA02|a1^b1~a2^b2~a3^b3|c1^d1^a1&b1~c2^d2^a2&b2~c3^d3^a3&b3|a^b|c^d^a&b\r");
        }

    }
}

