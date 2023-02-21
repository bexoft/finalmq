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

        [Fact]
        public void TestSSU_U03()
        {
            hl7.SSU_U03 msg = new hl7.SSU_U03();
            msg.msh.countryCode = "de";
            msg.uac = new hl7.UAC();
            msg.uac.userAuthenticationCredential.typeOfData = "hello";
            msg.sft.Add(new hl7.SFT());
            msg.sft.Add(new hl7.SFT());
            msg.sft.Add(new hl7.SFT());
            msg.sft[0].softwareBinaryID = "world";
            msg.sac.Add(new hl7.SAC_GROUP());
            msg.sac.Add(new hl7.SAC_GROUP());
            msg.sac.Add(new hl7.SAC_GROUP());
            msg.sac.Add(new hl7.SAC_GROUP());
            msg.sac[0].sac.positionInTray = "hey";
            msg.sac[0].sac.specimenSource = "hh";
            msg.sac[0].sac.carrierId.entityIdentifier = "uu";
            msg.sac[0].sac.carrierId.universalId = "bbb";
            msg.sac[0].obx.Add(new hl7.OBX());
            msg.sac[0].obx.Add(new hl7.OBX());
            msg.sac[0].nte.Add(new hl7.NTE());
            msg.sac[0].spm.Add(new hl7.SPM_GROUP());
            msg.sac[0].spm.Add(new hl7.SPM_GROUP());
            msg.sac[0].spm.Add(new hl7.SPM_GROUP());
            msg.sac[0].spm[0].spm.accessionId = "ggg";
            msg.sac[0].spm[0].spm.containerCondition.alternateText = "tt";
            msg.sac[0].spm[0].spm.containerCondition.nameOfAlternateCodingSystem = "cc";
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


    }
}

