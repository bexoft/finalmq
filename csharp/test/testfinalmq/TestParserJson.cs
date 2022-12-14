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

    [MetaEnum("Hello")]
    public enum MyEnum
    {
        [MetaEnumEntry("Hello", alias:"entry1")] 
        MY_ENTRY1 = 0,
        [MetaEnumEntry("Hello", alias:"entry2")] 
        MY_ENTRY2 = 1,
    }

    [MetaStruct("Hello")]
    public class MyTestClass
    {
        [MetaField("Hello", flags : MetaFieldFlags.METAFLAG_PROTO_VARINT | MetaFieldFlags.METAFLAG_PROTO_ZIGZAG)]
        public int value
        {
            get { return m_a; }
            set { m_a = value; }
        }

        private int m_a = 0;
    }

    public class TestParserJson : IDisposable
    {
        public TestParserJson()
        {
        }

        public void Dispose()
        {
        }

        [Fact]
        public void TestUnknownStruct()
        {
            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();
            string data = "{}";
            ParserJson parser = new ParserJson(mockVisitor.Object, data);
            int res = parser.ParseStruct("test.BlaBla");
            Debug.Assert(res == -1);

            mockVisitor.Verify(x => x.NotifyError(data, It.IsAny<string>()), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestBool()
        {
            bool VALUE = true;

            MetaField? fieldValue = MetaDataGlobal.Instance.GetField("test.TestBool", "value");
            Debug.Assert(fieldValue != null);

            Mock<IParserVisitor> mockVisitor = new Mock<IParserVisitor>();

            string data = "{\"value\":true}";
            ParserJson parser = new ParserJson(mockVisitor.Object, data);
            int res = parser.ParseStruct("test.TestBool");
            Debug.Assert(res != -1);

            mockVisitor.Verify(x => x.StartStruct(It.IsAny<MetaStruct>()), Times.Once);
            mockVisitor.Verify(x => x.EnterBool(fieldValue, VALUE), Times.Once);
            mockVisitor.Verify(x => x.Finished(), Times.Once);
        }

    }
}

