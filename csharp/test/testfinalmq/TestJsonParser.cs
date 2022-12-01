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


namespace testfinalmq
{

    public class TestJsonParser : IDisposable
    {
        Mock<IJsonParserVisitor> m_mockJsonParserVisitor;
        JsonParser m_parser;

        public TestJsonParser()
        {
            m_mockJsonParserVisitor = new Mock<IJsonParserVisitor>();
            m_parser = new JsonParser(m_mockJsonParserVisitor.Object);
        }

        public void Dispose()
        {
        }


        [Fact]
        public void TestStringSize()
        {
            string json = "{\"key\":23}";
            int res = m_parser.Parse(Encoding.UTF8.GetBytes(json), 0, 5);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestEmpty()
        {
            string json = "";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestNull()
        {
            string json = "null";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterNull(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestNullWithSpaces()
        {
            string json = "\t\n\r null\t\n\r ";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterNull(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestNullEarlyEnd()
        {
            string json = "nul";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestNullWrongChar()
        {
            string json = "nulo";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("o", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestTrue()
        {
            string json = "true";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterBool(true), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestFalse()
        {
            string json = "false";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterBool(false), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestNoDigit()
        {
            string json = "Hello";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("Hello", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestNegativeNoDigit()
        {
            string json = "-Hello";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("Hello", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestInt32()
        {
            string json = "-1234567";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterInt32(-1234567), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestInt32WithSpaces()
        {
            string json = "\t\n\r -1234567\t\n\r ";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterInt32(-1234567), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestInt32WrongFormat()
        {
            string json = "-123-4567";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("-123-4567", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestUInt32()
        {
            string json = "1234567";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(1234567), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestUInt32WrongFormat()
        {
            string json = "123-4567";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("123-4567", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestInt64()
        {
            string json = "-123456789012";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterInt64(-123456789012), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestInt64WrongFormat()
        {
            string json = "-123-456789012";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("-123-456789012", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestUInt64()
        {
            string json = "123456789012";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterUInt64(123456789012), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestUInt64WrongFormat()
        {
            string json = "123-456789012";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("123-456789012", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestDouble()
        {
            string json = "0.123456789012";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterDouble(0.123456789012), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestDoubleExponent()
        {
            string json = "1e2";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterDouble(100.0), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestDoubleWrongFormat()
        {
            string json = "0.12.34.56789012";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("0.12.34.56789012", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestEmptyString()
        {
            string json = "\"\"";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterString(""), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestEmptyStingEarlyEnd()
        {
            string json = "\"";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

    }
}