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

            m_mockJsonParserVisitor.Verify(x => x.EnterString(It.IsAny<byte[]>(), 1, 0), Times.Once);
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

        [Fact]
        public void TestEmptyStringWithSpaces()
        {
            string json = "\t\n\r \"\"\t\n\r ";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterString(It.IsAny<byte[]>(), 5, 0), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestString()
        {
            string json = "\"Hello World\"";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterString(It.IsAny<byte[]>(), 1, 11), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestStringSimpleEscape()
        {
            string json = "\" \\\" \\\\ \\/ \\b \\f \\n \\r \\t \"";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterString(" \" \\ / \b \f \n \r \t "), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestStringSimpleEscapeEarlyEnd()
        {
            string json = "\"\\t";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestStringEscapeU16()
        {
            string json = "\"\\u00E4\"";
            string cmp = Encoding.UTF8.GetString(new byte[] { 0xc3, 0xa4 });
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterString(cmp), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestStringEscapeU16Invalid()
        {
            string json = "\"\\u00H4\"";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("H4\"", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestStringEscapeU16EarlyEnd()
        {
            string json = "\"\\u00E";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestStringEscapeU32()
        {
            string json = "\"\\uD802\\uDDAA\""; // codepoint=68010=0x109aa
            string cmp = Encoding.UTF8.GetString(new byte[] { 0xf0, 0x90, 0xa6, 0xaa });
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterString(cmp), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestStringEscapeAscii()
        {
            string json = "\"\\u0032\""; // codepoint=0x00000032
            string cmp = "2";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterString(cmp), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestStringEscapeU32Invalid()
        {
            string json = "\"\\uD801\\uDEH1\"";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("H1\"", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestStringEscapeU32EarlyEnd()
        {
            string json = "\"\\uD801\\uDE01";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestStringEscapeU32UnknownEscape()
        {
            string json = "\"\\z\"";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterString("\\z"), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestEmptyArray()
        {
            string json = "[]";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestEmptyArrayWithSpaces()
        {
            string json = "\t\n\r [\t\n\r ]\t\n\r ";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayWithOneValue()
        {
            string json = "[123]";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayWithOneValueWithSpaces()
        {
            string json = "\t\n\r [\t\n\r 123\t\n\r ]\t\n\r ";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayWithTwoValues()
        {
            string json = "[123,\"Hello\"]";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterString(It.IsAny<byte[]>(), 6, 5), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayWithTwoValuesWithSpaces()
        {
            string json = " [ 123 , \"Hello\" ] ";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterString(It.IsAny<byte[]>(), 10, 5), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayTolerateLastCommaTwoValues()
        {
            string json = "[123,\"Hello\",]";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterString(It.IsAny<byte[]>(), 6, 5), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayTolerateLastCommaWithSpacesTwoValues()
        {
            string json = " [ 123 , \"Hello\" , ] ";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterString(It.IsAny<byte[]>(), 10, 5), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayTolerateLastCommaOneValue()
        {
            string json = "[123,]";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayTolerateLastCommaOneValueWithSpaces()
        {
            string json = " [ 123 , ] ";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestEmptyArrayOneComma()
        {
            string json = "[,]";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.EnterArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.SyntaxError(",]", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void testEmptyArrayOneCommaWithSpaces()
        {
            string json = " [ , ] ";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.EnterArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.SyntaxError(", ] ", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayTwoCommas()
        {
            string json = "[123,,]";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.EnterArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.SyntaxError(",]", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayTwoCommasWithSpaces()
        {
            string json = " [ 123 , , ] ";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.EnterArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.SyntaxError(", ] ", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayMissingComma()
        {
            string json = "[123 12]";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.EnterArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("12]", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestArrayEarlyEnd()
        {
            string json = "[123,";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.EnterArray(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestEmptyObjectNoEnd()
        {
            string json = "{";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestEmptyObject()
        {
            string json = "{}";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestEmptyObjectWithSpaces()
        {
            string json = "\t\n\r {\t\n\r }\t\n\r ";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestObjectWithOneEntry()
        {
            string json = "{\"key\":123}";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterKey(It.IsAny<byte[]>(), 2, 3), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void testObjectWithOneEntryWithSpaces()
        {
            string json = " { \"key\" : 123 } ";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterKey(It.IsAny<byte[]>(), 4, 3), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestObjectMissingColen()
        {
            string json = "{\"key\" 123}";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterKey(It.IsAny<byte[]>(), 2, 3), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("123}", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestObjectMissingKeyString()
        {
            string json = "{key:123}";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("key:123}", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestObjectMissingKeyStringWithSpaces()
        {
            string json = " { key : 123 } ";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("key : 123 } ", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestObjectMissingComma()
        {
            string json = "{\"key\":123 \"next\":123}";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterKey(It.IsAny<byte[]>(), 2, 3), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("\"next\":123}", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestObjectWithOneEntryTolerateLastComma()
        {
            string json = "{\"key\":123,}";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterKey(It.IsAny<byte[]>(), 2, 3), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestObjectTwoCommas()
        {
            string json = "{\"key\":123,,\"next\":123}";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterKey(It.IsAny<byte[]>(), 2, 3), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.SyntaxError(",\"next\":123}", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestObjectTwoCommasWithSpaces()
        {
            string json = "{\"key\":123 , ,\"next\":123}";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterKey(It.IsAny<byte[]>(), 2, 3), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.SyntaxError(",\"next\":123}", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestObjectEarlyEndInKey()
        {
            string json = "{\"key";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void TestObjectInvalidValue()
        {
            string json = "{\"key\":12-12";
            int res = m_parser.Parse(json);
            Debug.Assert(res == -1);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterKey(It.IsAny<byte[]>(), 2, 3), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.SyntaxError("12-12", It.IsAny<string>()), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void testObjectKeyWithEscape()
        {
            string json = "{\"ke\\ty\":123}";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterKey("ke\ty"), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

        [Fact]
        public void testObjectKeysWithEscape()
        {
            string json = "{\"ke\\ty\":123, \"ke\\ny\":456}";
            int res = m_parser.Parse(json);
            Debug.Assert(res == json.Length);

            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterKey("ke\ty"), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(123), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterKey("ke\ny"), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.EnterUInt32(456), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.ExitObject(), Times.Once);
            m_mockJsonParserVisitor.Verify(x => x.Finished(), Times.Once);
        }

    }
}

