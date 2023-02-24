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

    public class TestHl7Builder : IDisposable
    {
        static readonly int MAX_BLOCK_SIZE = 1024;

        IHl7BuilderVisitor m_builder;
        byte[] m_data = new byte[MAX_BLOCK_SIZE];
        int m_size = MAX_BLOCK_SIZE;
        BufferRef m_bufferRef;
        Mock<IZeroCopyBuffer>? m_mockBuffer = null;

        public TestHl7Builder()
        {
            m_mockBuffer = new Mock<IZeroCopyBuffer>();
            m_bufferRef = new BufferRef(m_data, 0, m_data.Length);

            m_mockBuffer.Setup(x => x.AddBuffer(MAX_BLOCK_SIZE, It.IsAny<int>())).Returns(m_bufferRef);
            m_mockBuffer.Setup(x => x.DownsizeLastBuffer(It.IsAny<int>())).Callback((int size) => {
                m_size = size;
            });
            m_mockBuffer.Setup(x => x.RemainingSize).Returns(0);

            m_builder = new Hl7Builder(m_mockBuffer.Object, MAX_BLOCK_SIZE);
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
        public void TestTwoEmptySegments()
        {
            m_builder.EnterString(new int[] { 0 }, 1, 0, "MSH");
            m_builder.EnterString(new int[] { 0 }, 1, 2, "name");
            m_builder.EnterString(new int[] { 1 }, 1, 0, "TST");
            m_builder.Finished();
            string str = Encoding.ASCII.GetString(m_data, 0, m_size);
            Debug.Assert(str == "MSH|^~\\&|name\rTST\r");
        }

        [Fact]
        public void TestSubStructs()
        {
            m_builder.EnterString(new int[]{ 0 }, 1, 0, "MSH");
            m_builder.EnterString(new int[]{ 0 }, 1, 2, "a1");
            m_builder.EnterString(new int[]{ 0 }, 1, 3, "a2");

            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 0, "b1");
            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 1, "b2");

            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 0, "c1");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 1, "c2");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 2, "c3");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 3, "c4");

            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 3, "b3");
            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 4, "b4");

            m_builder.EnterString(new int[]{ 0 }, 1, 5, "a3");
            m_builder.EnterString(new int[]{ 0 }, 1, 6, "a4");

            m_builder.EnterString(new int[]{ 1 }, 1, 0, "TST");

            m_builder.Finished();
            string str = Encoding.ASCII.GetString(m_data, 0, m_size);
            Debug.Assert(str == "MSH|^~\\&|a1|a2|b1^b2^c1&c2&c3&c4^b3^b4|a3|a4\rTST\r");
        }

        [Fact]
        public void TestExit3StructsFromInnerLevel()
        {
            m_builder.EnterString(new int[]{ 0 }, 1, 0, "MSH");
            m_builder.EnterString(new int[]{ 0 }, 1, 2, "a1");
            m_builder.EnterString(new int[]{ 0 }, 1, 3, "a2");

            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 0, "b1");
            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 1, "b2");

            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 0, "c1");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 1, "c2");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 2, "c3");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 3, "c4");

            m_builder.EnterString(new int[]{ 1 }, 1, 0, "TST");

            m_builder.Finished();
            string str = Encoding.ASCII.GetString(m_data, 0, m_size);
            Debug.Assert(str == "MSH|^~\\&|a1|a2|b1^b2^c1&c2&c3&c4\rTST\r");
        }

        [Fact]
        public void TestExit2StructsFromInnerLevel()
        {
            m_builder.EnterString(new int[]{ 0 }, 1, 0, "MSH");
            m_builder.EnterString(new int[]{ 0 }, 1, 2, "a1");
            m_builder.EnterString(new int[]{ 0 }, 1, 3, "a2");

            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 0, "b1");
            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 1, "b2");

            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 0, "c1");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 1, "c2");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 2, "c3");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 3, "c4");

            m_builder.EnterString(new int[]{ 0 }, 1, 5, "a3");

            m_builder.EnterString(new int[]{ 1 }, 1, 0, "TST");

            m_builder.Finished();
            string str = Encoding.ASCII.GetString(m_data, 0, m_size);
            Debug.Assert(str == "MSH|^~\\&|a1|a2|b1^b2^c1&c2&c3&c4|a3\rTST\r");
        }


        [Fact]
        public void TestSubStructsRemoveEmpty()
        {
            m_builder.EnterString(new int[]{ 0 }, 1, 0, "MSH");
            m_builder.EnterString(new int[]{ 0 }, 1, 2, "a1");
            m_builder.EnterString(new int[]{ 0 }, 1, 3, "a2");

            m_builder.EnterString(new int[]{ 0, 6, 0 }, 3, 0, "b1");
            m_builder.EnterString(new int[]{ 0, 6, 0 }, 3, 1, "b2");

            m_builder.EnterString(new int[]{ 0, 6, 0, 4}, 4, 0, "c1");
            m_builder.EnterString(new int[]{ 0, 6, 0, 4}, 4, 1, "c2");
            m_builder.EnterString(new int[]{ 0, 6, 0, 4}, 4, 2, "c3");
            m_builder.EnterString(new int[]{ 0, 6, 0, 4}, 4, 3, "c4");

            m_builder.EnterString(new int[]{ 0, 6, 0 }, 3, 7, "b3");
            m_builder.EnterString(new int[]{ 0, 6, 0 }, 3, 8, "b4");

            m_builder.EnterString(new int[]{ 0 }, 1, 9, "a3");
            m_builder.EnterString(new int[]{ 0 }, 1,10, "a4");

            m_builder.EnterString(new int[]{ 1 }, 1, 0, "TST");

            m_builder.Finished();
            string str = Encoding.ASCII.GetString(m_data, 0, m_size);
            Debug.Assert(str == "MSH|^~\\&|a1|a2|||b1^b2^^^c1&c2&c3&c4^^^b3^b4|||a3|a4\rTST\r");
        }


        [Fact]
        public void TestArray()
        {
            m_builder.EnterString(new int[]{ 0 }, 1, 0, "MSH");
            m_builder.EnterString(new int[]{ 0 }, 1, 2, "a1");
            m_builder.EnterString(new int[]{ 0 }, 1, 3, "a2.1");
            m_builder.EnterString(new int[]{ 0 }, 1, 3, "a2.2");
            m_builder.EnterString(new int[]{ 0 }, 1, 3, "a2.3");

            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 0, "b1");
            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 1, "b2.1");
            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 1, "b2.2");
            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 1, "b2.3");
            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 1, "");
            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 1, "");

            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 0, "c1");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 1, "c2.1");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 1, "c2.2");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 1, "c2.3");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 2, "c3");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 3, "c4");

            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 3, "b3");
            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 4, "b4");

            m_builder.EnterString(new int[]{ 0 }, 1, 5, "a3");
            m_builder.EnterString(new int[]{ 0 }, 1, 6, "a4");

            m_builder.EnterString(new int[]{ 1 }, 1, 0, "TST");

            m_builder.Finished();
            string str = Encoding.ASCII.GetString(m_data, 0, m_size);
            Debug.Assert(str == "MSH|^~\\&|a1|a2.1~a2.2~a2.3|b1^b2.1~b2.2~b2.3~~^c1&c2.1~c2.2~c2.3&c3&c4^b3^b4|a3|a4\rTST\r");
        }


        [Fact]
        public void TestTypes()
        {
            m_builder.EnterString(new int[]{ 0 }, 1, 0, "MSH");
            m_builder.EnterString(new int[]{ 0 }, 1, 2, "123");
            m_builder.EnterString(new int[]{ 0 }, 1, 3, "0.123");

            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 0, "-2");
            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 1, "12");

            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 0, "1");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 1, "2");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 2, "-3");
            m_builder.EnterString(new int[]{ 0, 4, 0, 2}, 4, 3, "-4");

            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 3, "1.1");
            m_builder.EnterString(new int[]{ 0, 4, 0 }, 3, 4, "2.1");

            m_builder.EnterString(new int[]{ 0 }, 1, 5, "1");
            m_builder.EnterString(new int[]{ 0 }, 1, 6, "2");

            m_builder.EnterString(new int[]{ 1 }, 1, 0, "TST");

            m_builder.Finished();
            string str = Encoding.ASCII.GetString(m_data, 0, m_size);
            Debug.Assert(str == "MSH|^~\\&|123|0.123|-2^12^1&2&-3&-4^1.1^2.1|1|2\rTST\r");
        }



        [Fact]
        public void TestEscape()
        {
            m_builder.EnterString(new int[]{ 0 }, 1, 0, "MSH");
            m_builder.EnterString(new int[]{ 0 }, 1, 2, "\r\n\t|^&~\\");
            m_builder.EnterString(new int[]{ 0 }, 1, 3, "a");

            m_builder.EnterString(new int[]{ 1 }, 1, 0, "TST");

            m_builder.Finished();

            string str = Encoding.ASCII.GetString(m_data, 0, m_size);
            Debug.Assert(str == "MSH|^~\\&|\\X0D\\\\X0A\\\\X09\\\\F\\\\S\\\\T\\\\R\\\\E\\|a\rTST\r");
        }
    }
}

