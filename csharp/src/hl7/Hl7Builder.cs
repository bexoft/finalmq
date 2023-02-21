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

using System.Diagnostics;
using System.Text;

namespace finalmq
{

    public interface IHl7BuilderVisitor
    {
        void EnterNull(int[] levelIndex, int sizeLevelIndex, int index);
        void EnterInt64(int[] levelIndex, int sizeLevelIndex, int index, long value);
        void EnterUInt64(int[] levelIndex, int sizeLevelIndex, int index, ulong value);
        void EnterDouble(int[] levelIndex, int sizeLevelIndex, int index, double value);
        void EnterString(int[] levelIndex, int sizeLevelIndex, int index, string value);
        void Finished();
    };


    public class Hl7Builder : IHl7BuilderVisitor
    {
        static readonly byte SEGMENT_END = 0x0D;   // '\r'

        public Hl7Builder(IZeroCopyBuffer buffer, int maxBlockSize = 512, string delimiters = "|^~\\&")
        {
            m_zeroCopybuffer = buffer;
            m_maxBlockSize = maxBlockSize;
            m_root = new Hl7Node();

            Debug.Assert(delimiters.Length == 5);
            m_delimitersForField = new byte[delimiters.Length + 1];
            Array.Copy(Encoding.ASCII.GetBytes(delimiters), m_delimitersForField, delimiters.Length);
            m_delimitersForField[delimiters.Length] = (byte)m_delimitersForField[0];  // |^~\&|
            m_delimiterField[0] = SEGMENT_END;      // segment delimiter '\r', 0x0D
            m_delimiterField[1] = (byte)delimiters[0];    // |
            m_delimiterField[2] = (byte)delimiters[1];    // ^
            m_delimiterField[3] = (byte)delimiters[4];    // &
            m_delimiterRepeat = (byte)delimiters[2];    // ~
            m_escape = (byte)delimiters[3];    // '\\'
        }

        // IHl7BuilderVisitor
        public void EnterNull(int[] levelIndex, int sizeLevelIndex, int index)
        {
            m_root.EnterString(levelIndex, sizeLevelIndex, index, "\"\"");
        }

        public void EnterInt64(int[] levelIndex, int sizeLevelIndex, int index, long value)
        {
            m_root.EnterString(levelIndex, sizeLevelIndex, index, value.ToString());
        }

        public void EnterUInt64(int[] levelIndex, int sizeLevelIndex, int index, ulong value)
        {
            m_root.EnterString(levelIndex, sizeLevelIndex, index, value.ToString());
        }

        public void EnterDouble(int[] levelIndex, int sizeLevelIndex, int index, double value)
        {
            m_root.EnterString(levelIndex, sizeLevelIndex, index, value.ToString());
        }

        public void EnterString(int[] levelIndex, int sizeLevelIndex, int index, string value)
        {
            m_root.EnterString(levelIndex, sizeLevelIndex, index, value);
        }

        public void Finished()
        {
            ReserveSpace(3 + m_delimitersForField.Length);
            Debug.Assert(m_buffer != null);
            m_buffer[m_offset + 0] = (byte)'M';
            m_buffer[m_offset + 1] = (byte)'S';
            m_buffer[m_offset + 2] = (byte)'H';
            m_offset += 3;
            Array.Copy(m_delimitersForField, 0, m_buffer, m_offset, m_delimitersForField.Length);
            m_offset += m_delimitersForField.Length;

            Serialize(m_root, 0, 0);

            ResizeBuffer();
        }


        void ReserveSpace(int space)
        {
            int sizeRemaining = m_offsetEnd - m_offset;
            if (sizeRemaining < space)
            {
                if (m_bufferStart != null)
                {
                    int size = m_offset - m_bufferStart.Offset;
                    Debug.Assert(size >= 0);
                    m_zeroCopybuffer.DownsizeLastBuffer(size);
                }

                int sizeRemainingZeroCopyBuffer = m_zeroCopybuffer.RemainingSize;
                int sizeNew = m_maxBlockSize;
                if (space <= sizeRemainingZeroCopyBuffer)
                {
                    sizeNew = sizeRemainingZeroCopyBuffer;
                }
                sizeNew = Math.Max(sizeNew, space);
                BufferRef bufferStartNew = m_zeroCopybuffer.AddBuffer(sizeNew);
                m_buffer = bufferStartNew.Buffer;
                m_bufferStart = bufferStartNew;
                m_offsetEnd = m_bufferStart.Offset + sizeNew;
                m_offset = m_bufferStart.Offset;
            }
        }
        void ResizeBuffer()
        {
            if (m_bufferStart != null)
            {
                int size = m_offset - m_bufferStart.Offset;
                Debug.Assert(size >= 0);
                m_zeroCopybuffer.DownsizeLastBuffer(size);
                m_bufferStart = null;
                m_buffer = null;
                m_offsetEnd = 0;
                m_offset = 0;
            }
        }

        void Serialize(Hl7Node node, int index, int iStart)
        {
            IList<Hl7Node> nodes = node.Nodes;

            for (int i = iStart; i < nodes.Count(); ++i)
            {
                Hl7Node subNode = nodes[i];

                Debug.Assert(m_buffer != null);

                if (i != 0 && index == 0 && subNode.SegmentId.Length != 0)
                {
                    byte[] bytesSegmentId = Encoding.UTF8.GetBytes(subNode.SegmentId);
                    ReserveSpace(bytesSegmentId.Length + 1);
                    Array.Copy(bytesSegmentId, 0, m_buffer, m_offset, bytesSegmentId.Length);
                    m_offset += bytesSegmentId.Length;
                    if (subNode.Nodes.Count != 0)
                    {
                        m_buffer[m_offset] = m_delimiterField[index + 1];
                        ++m_offset;
                    }
                }

                if (subNode.Nodes.Count != 0)
                {
                    Serialize(subNode, index + 1, (index == 0 && i == 0) ? 2 : 0);    // skip MSH|^~\&|
                }
                else
                {
                    if (subNode.Strings.Count != 0)
                    {
                        for (int n = 0; n < subNode.Strings.Count; ++n)
                        {
                            string str = subNode.Strings[n];
                            ReserveSpace(str.Length * 6 + 1);
                            EscapeString(str);
                            if (n != subNode.Strings.Count - 1)
                            {
                                m_buffer[m_offset] = m_delimiterRepeat;
                                ++m_offset;
                            }
                        }
                    }
                }
                if ((i < nodes.Count - 1) || (index == 0))
                {
                    ReserveSpace(1);
                    m_buffer[m_offset] = m_delimiterField[index];
                    ++m_offset;
                }
            }
        }

        static readonly byte[] hexDigits = { (byte)'0', (byte)'1', (byte)'2', (byte)'3', (byte)'4', (byte)'5', (byte)'6', (byte)'7', (byte)'8', (byte)'9', (byte)'A', (byte)'B', (byte)'C', (byte)'D', (byte)'E', (byte)'F' };
    
        void EscapeString(string str)
        {
            Debug.Assert(m_buffer != null);

            foreach (var c in str)
            {
                if (c < 0x20)
                {
                    m_buffer[m_offset] = m_escape;
                    m_offset++;
                    m_buffer[m_offset] = (byte)'X';
                    m_offset++;
                    m_buffer[m_offset] = hexDigits[(c >> 4) & 0x0f];
                    m_offset++;
                    m_buffer[m_offset] = hexDigits[(c & 0x0f)];
                    m_offset++;
                    m_buffer[m_offset] = m_escape;
                    m_offset++;
                }
                else
                {
                    if (c == m_delimiterField[1])   // '|'
                    {
                        m_buffer[m_offset] = m_escape;
                        m_offset++;
                        m_buffer[m_offset] = (byte)'F';
                        m_offset++;
                        m_buffer[m_offset] = m_escape;
                        m_offset++;
                    }
                    else if (c == m_delimiterField[2])   // '^'
                    {
                        m_buffer[m_offset] = m_escape;
                        m_offset++;
                        m_buffer[m_offset] = (byte)'S';
                        m_offset++;
                        m_buffer[m_offset] = m_escape;
                        m_offset++;
                    }
                    else if (c == m_delimiterField[3])   // '&'
                    {
                        m_buffer[m_offset] = m_escape;
                        m_offset++;
                        m_buffer[m_offset] = (byte)'T';
                        m_offset++;
                        m_buffer[m_offset] = m_escape;
                        m_offset++;
                    }
                    else if (c == m_escape)   // '\\'
                    {
                        m_buffer[m_offset] = m_escape;
                        m_offset++;
                        m_buffer[m_offset] = (byte)'E';
                        m_offset++;
                        m_buffer[m_offset] = m_escape;
                        m_offset++;
                    }
                    else if (c == m_delimiterRepeat)   // '~'
                    {
                        m_buffer[m_offset] = m_escape;
                        m_offset++;
                        m_buffer[m_offset] = (byte)'R';
                        m_offset++;
                        m_buffer[m_offset] = m_escape;
                        m_offset++;
                    }
                    else
                    {
                        m_buffer[m_offset] = (byte)c;
                        m_offset++;
                    }
                }
            }
        }

        class Hl7Node
        {
            public void EnterString(int[] levelIndex, int sizeLevelIndex, int index, string value, int offsetLevelIndex = 0)
            {
                if (index == -1 && sizeLevelIndex == 0)
                {
                    if (value.Length != 0)
                    {
                        m_segmentId = value;
                    }
                    return;
                }
                int i = index;
                if (sizeLevelIndex > 0)
                {
                    i = levelIndex[offsetLevelIndex + 0];
                }
                int fill = i + 1 - m_nodes.Count;
                for (int n = 0; n < fill; ++n)
                {
                    m_nodes.Add(new Hl7Node());
                }

                if (sizeLevelIndex == 0)
                {
                    m_nodes[i].m_strings.Add(value);
                }
                else
                {
                    m_nodes[i].EnterString(levelIndex, sizeLevelIndex - 1, index, value, offsetLevelIndex + 1);
                }
            }

            public string SegmentId { get { return m_segmentId; } }
            public IList<Hl7Node> Nodes { get { return m_nodes; } }
            public IList<string> Strings { get { return m_strings; } }

            string m_segmentId = string.Empty;
            readonly IList<Hl7Node> m_nodes = new List<Hl7Node>();
            readonly IList<string> m_strings = new List<string>();
        }


        IZeroCopyBuffer m_zeroCopybuffer;
        int m_maxBlockSize = 512;
        BufferRef? m_bufferStart = null;
        byte[]? m_buffer;
        int m_offset = 0;
        int m_offsetEnd = 0;

        static readonly int LAYER_MAX = 4;

        readonly byte[] m_delimitersForField;
        readonly byte[] m_delimiterField = new byte[LAYER_MAX];
        readonly byte   m_delimiterRepeat;
        readonly byte   m_escape;

        readonly Hl7Node m_root;
    };

}   // namespace finalmq
