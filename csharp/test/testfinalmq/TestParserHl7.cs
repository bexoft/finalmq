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
            hl7.SSU_U03 cmp = new hl7.SSU_U03();
            cmp.msh.fieldSeparator = "|";
            cmp.msh.encodingCharacters = "^~\\&";
            cmp.msh.messageType.messageCode = "SSU";
            cmp.msh.messageType.triggerEvent = "U03";
            cmp.msh.messageType.messageStructure = "SSU_U03";
            cmp.msh.countryCode = "de";
            cmp.uac = new hl7.UAC();
            cmp.uac.userAuthenticationCredential.typeOfData = "hello";
            cmp.sft.Add(new hl7.SFT());
            cmp.sft.Add(new hl7.SFT());
            cmp.sft.Add(new hl7.SFT());
            cmp.sft[0].softwareBinaryID = "world";
            cmp.sac.Add(new hl7.SAC_GROUP());
            cmp.sac.Add(new hl7.SAC_GROUP());
            cmp.sac.Add(new hl7.SAC_GROUP());
            cmp.sac.Add(new hl7.SAC_GROUP());
            cmp.sac[0].sac.positionInTray = "hey";
            cmp.sac[0].sac.specimenSource = "hh";
            cmp.sac[0].sac.carrierId.entityIdentifier = "uu";
            cmp.sac[0].sac.carrierId.universalId = "bbb";
            cmp.sac[0].obx.Add(new hl7.OBX());
            cmp.sac[0].obx.Add(new hl7.OBX());
            cmp.sac[0].nte.Add(new hl7.NTE());
            cmp.sac[0].spm.Add(new hl7.SPM_GROUP());
            cmp.sac[0].spm.Add(new hl7.SPM_GROUP());
            cmp.sac[0].spm.Add(new hl7.SPM_GROUP());
            cmp.sac[0].spm[0].spm.accessionId = "ggg";
            cmp.sac[0].spm[0].spm.containerCondition.alternateText = "tt";
            cmp.sac[0].spm[0].spm.containerCondition.nameOfAlternateCodingSystem = "cc";
            cmp.sac[0].spm[0].obx.Add(new hl7.OBX());
            cmp.sac[0].spm[0].obx.Add(new hl7.OBX());
            cmp.sac[0].spm[0].obx.Add(new hl7.OBX());
            cmp.sac[0].spm[0].obx.Add(new hl7.OBX());
            cmp.sac[0].spm[0].obx.Add(new hl7.OBX());

            hl7.SSU_U03 msg = new hl7.SSU_U03();
            string data = "MSH|^~\\&|||||||SSU^U03^SSU_U03||||||||de\rSFT|||world\rSFT\rSFT\rUAC||^hello\rEQU\rSAC||||||hh||||uu^^bbb||||hey\rOBX\rOBX\rNTE\rSPM||||||||||||||||||||||||||||^^^^tt^cc||ggg\rOBX\rOBX\rOBX\rOBX\rOBX\rSPM\rSPM\rSAC\rSAC\rSAC\r";

            SerializerStruct serializer = new SerializerStruct(msg);
            ParserHl7 parser = new ParserHl7(serializer, data);
            parser.ParseStruct("hl7.SSU_U03");

            Debug.Assert(cmp.Equals(msg));
        }


    }
}

