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

    public class TestHl7Parser : IDisposable
    {
        IHl7Parser m_parser;

        public TestHl7Parser()
        {
            m_parser = new Hl7Parser();
        }

        public void Dispose()
        {
        }


        [Fact]
        public void TestStartParseWrongMessageStart()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("WrongMessageStart");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(res == false);
        }

        [Fact]
        public void TestStartParseMessageTooShort()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(false == res);
        }
        [Fact]
        public void TestStartParseMessageTooShortWithSize()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(false == res);
        }

        [Fact]
        public void TestStartParse()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);
        }

        [Fact]
        public void TestParseToken()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|a\x0d");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "a");

            int pos = m_parser.GetCurrentPosition();
            Debug.Assert(11 == pos);

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");

            pos = m_parser.GetCurrentPosition();
            Debug.Assert(11 == pos);
        }


        [Fact]
        public void TestParseTokenWithGivenSize()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|a\x0d");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "a");

            int pos = m_parser.GetCurrentPosition();
            Debug.Assert(11 == pos);

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");

            pos = m_parser.GetCurrentPosition();
            Debug.Assert(11 == pos);
        }

        [Fact]
        public void TestParseTokenWithStartCharacters()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("\x0BMSH|^~\\&|a\x0d");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "a");

            int pos = m_parser.GetCurrentPosition();
            Debug.Assert(12 == pos);

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");

            pos = m_parser.GetCurrentPosition();
            Debug.Assert(12 == pos);
        }

        [Fact]
        public void TestParseTokenWithStartAndEndCharacters()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("\x0BMSH|^~\\&|a\x0d\x1c\x0d");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "a");

            int pos = m_parser.GetCurrentPosition();
            Debug.Assert(12 == pos);

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "\x1c");

            pos = m_parser.GetCurrentPosition();
            Debug.Assert(14 == pos);

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");

            pos = m_parser.GetCurrentPosition();
            Debug.Assert(14 == pos);
        }

        [Fact]
        public void TestParseTokenLevel2()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|a^b|a^b\x0d");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(2 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "b");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(2 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "b");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

        [Fact]
        public void TestParseTokenLevel3()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|a^b1&b2|\x0d");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(2 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(3, out token);
            Debug.Assert(3 == level);
            Debug.Assert(token == "b1");

            level = m_parser.ParseToken(3, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "b2");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

        [Fact]
        public void TestParseTokenLevel3to2()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|a^b1&b2^c|\x0d");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(2 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(3, out token);
            Debug.Assert(3 == level);
            Debug.Assert(token == "b1");

            level = m_parser.ParseToken(3, out token);
            Debug.Assert(2 == level);
            Debug.Assert(token == "b2");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "c");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

        [Fact]
        public void TestParseTokenLevelSkipStructLevel3to2()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|a^b1&b2^c|\x0d");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(2 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(3, out token);
            Debug.Assert(3 == level);
            Debug.Assert(token == "b1");

            level = m_parser.ParseTillEndOfStruct(2);
            Debug.Assert(2 == level);

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "c");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

        [Fact]
        public void TestParseTokenLevelSkipStructLevel3to1()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|a^b1&b2^c|d\x0d");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(2 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(3, out token);
            Debug.Assert(3 == level);
            Debug.Assert(token == "b1");

            level = m_parser.ParseTillEndOfStruct(1);
            Debug.Assert(1 == level);

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "d");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

        [Fact]
        public void TestParseTokenLevelSkipStructLevel3to1Last()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|a^b1&b2^c\x0d");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(2 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(3, out token);
            Debug.Assert(3 == level);
            Debug.Assert(token == "b1");

            level = m_parser.ParseTillEndOfStruct(1);
            Debug.Assert(0 == level);

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

        [Fact]
        public void TestParseTokenLevelSkipStructLevel3to1LastEnd()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|a^b1&b2^c");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(2 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(3, out token);
            Debug.Assert(3 == level);
            Debug.Assert(token == "b1");

            level = m_parser.ParseTillEndOfStruct(1);
            Debug.Assert(-1 == level);
        }


        [Fact]
        public void TestParseTokenWrongLevel()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|a^b1&b2^c|d\x0d");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(2 == level);
            Debug.Assert(token == "a");

            // wrong level
            level = m_parser.ParseToken(2, out token);
            Debug.Assert(2 == level);
            Debug.Assert(token == "b1");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "c");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "d");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

        [Fact]
        public void TestParseTokenArray()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|a^b1~b2~~^c|d\x0d");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(2 == level);
            Debug.Assert(token == "a");

            IList<string> arr = new List<string>();
            level = m_parser.ParseTokenArray(2, arr);
            Debug.Assert(2 == level);
            Debug.Assert(4 == arr.Count);
            Debug.Assert(arr[0] == "b1");
            Debug.Assert(arr[1] == "b2");
            Debug.Assert(arr[2] == "");
            Debug.Assert(arr[3] == "");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "c");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "d");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

        [Fact]
        public void TestParseTokenArrayWrongLevel()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|a^b1~b2~~&b4^c|d\x0d");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(2 == level);
            Debug.Assert(token == "a");

            // wrong level
            IList<string> arr = new List<string>();
            level = m_parser.ParseTokenArray(2, arr);
            Debug.Assert(2 == level);
            Debug.Assert(2 == level);
            Debug.Assert(4 == arr.Count);
            Debug.Assert(arr[0] == "b1");
            Debug.Assert(arr[1] == "b2");
            Debug.Assert(arr[2] == "");
            Debug.Assert(arr[3] == "");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "c");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "d");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

        [Fact]
        public void TestParseTokenWrongArray()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|a^b1~b2~~^c|d\x0d");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(2 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(2 == level);
            Debug.Assert(token == "b1");

            level = m_parser.ParseToken(2, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "c");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "d");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }


        [Fact]
        public void TestParseTokenEscape()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|\\X0D\\\\X0A\\\\X09\\\\F\\\\S\\\\T\\\\R\\\\E\\|a\rTST\r");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "\r\n\t|^&~\\");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "TST");

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

        [Fact]
        public void TestParseTokenEscapeSmallHexCode()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|\\X0d\\|a\rTST\r");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "\r");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "TST");

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

        [Fact]
        public void TestParseTokenEscapeWrong1()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|\\X0D|a\rTST\r");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "\r");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "TST");

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

        [Fact]
        public void TestParseTokenEscapeWrong2()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|\\|a\rTST\r");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "TST");

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

        [Fact]
        public void TestParseTokenEscapeWrong3()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|\\F|a\rTST\r");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "TST");

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

        [Fact]
        public void TestParseTokenEscapeWrong4()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|\\Xwp\\|a\rTST\r");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "TST");

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

        [Fact]
        public void TestParseTokenEscapeWrong5()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|\\X0p\\|a\rTST\r");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "TST");

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

        [Fact]
        public void TestParseTokenEscapeWrong6()
        {
            byte[] hl7 = Encoding.ASCII.GetBytes("MSH|^~\\&|\\X0|a\rTST\r");
            bool res = m_parser.StartParse(hl7, 0);
            Debug.Assert(true == res);

            string token;
            int level;
            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "MSH");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "|");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "^~\\&");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(1 == level);
            Debug.Assert(token == "");

            level = m_parser.ParseToken(1, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "a");

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(0 == level);
            Debug.Assert(token == "TST");

            level = m_parser.ParseToken(0, out token);
            Debug.Assert(-1 == level);
            Debug.Assert(token == "");
        }

    }
}

