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
    public class JsonBuilder : IJsonParserVisitor
    {
        static byte[] m_strNull = { (byte)'n', (byte)'u', (byte)'l', (byte)'l', (byte)',' };
        static byte[] m_strTrue = { (byte)'t', (byte)'r', (byte)'u', (byte)'e', (byte)',' };
        static byte[] m_strFalse = { (byte)'f', (byte)'a', (byte)'l', (byte)'s', (byte)'e', (byte)',' };
        public JsonBuilder(IZeroCopyBuffer buffer, int maxBlockSize = 512)
        {
            m_zeroCopybuffer = buffer;
            m_maxBlockSize = maxBlockSize;
        }

        // IJsonParserVisitor
        public void SyntaxError(string buffer, string message)
        {
            Debug.Assert(false);
        }
        public void EnterNull()
        {
            ReserveSpace(5);
            Debug.Assert(m_buffer != null);
            m_strNull.CopyTo(m_buffer, m_offset);
            m_offset += 5;
        }
        public void EnterBool(bool value)
        {
            ReserveSpace(6);
            Debug.Assert(m_buffer != null);
            if (value)
            {
                m_strTrue.CopyTo(m_buffer, m_offset);
                m_offset += 5;
            }
            else
            {
                m_strFalse.CopyTo(m_buffer, m_offset);
                m_offset += 6;
            }
        }
        public void EnterInt32(int value)
        {
            ReserveSpace(20);   // 11 + 1 is enough: 10 digits + 1 minus + 1 comma
            Debug.Assert(m_buffer != null);
            string s = value.ToString(System.Globalization.CultureInfo.InvariantCulture);
            m_offset += Encoding.UTF8.GetBytes(s, 0, s.Length, m_buffer, m_offset);
            m_buffer[m_offset] = (byte)',';
            ++m_offset;
        }
        public void EnterUInt32(uint value)
        {
            ReserveSpace(20);   // 10 + 1 is enough: 10 digits + 1 comma
            Debug.Assert(m_buffer != null);
            string s = value.ToString(System.Globalization.CultureInfo.InvariantCulture);
            m_offset += Encoding.UTF8.GetBytes(s, 0, s.Length, m_buffer, m_offset);
            m_buffer[m_offset] = (byte)',';
            ++m_offset;
        }
        public void EnterInt64(long value)
        {
            ReserveSpace(30);   // 21 + 1 is enough: 20 digits + 1 minus + 1 comma
            Debug.Assert(m_buffer != null);
            string s = value.ToString(System.Globalization.CultureInfo.InvariantCulture);
            m_offset += Encoding.UTF8.GetBytes(s, 0, s.Length, m_buffer, m_offset);
            m_buffer[m_offset] = (byte)',';
            ++m_offset;
        }
        public void EnterUInt64(ulong value)
        {
            ReserveSpace(30);   // 20 + 1 is enough: 20 digits + 1 comma
            Debug.Assert(m_buffer != null);
            string s = value.ToString(System.Globalization.CultureInfo.InvariantCulture);
            m_offset += Encoding.UTF8.GetBytes(s, 0, s.Length, m_buffer, m_offset);
            m_buffer[m_offset] = (byte)',';
            ++m_offset;
        }
        public void EnterDouble(double value)
        {
            ReserveSpace(50);
            Debug.Assert(m_buffer != null);
            string s = value.ToString(System.Globalization.CultureInfo.InvariantCulture);
            m_offset += Encoding.UTF8.GetBytes(s, 0, s.Length, m_buffer, m_offset);
            m_buffer[m_offset] = (byte)',';
            ++m_offset;
        }
        public void EnterString(string value)
        {
            SerializeStringWithEnding(value, (byte)',');
        }
        public void EnterString(byte[] buffer, int offset, int size)
        {
            SerializeStringWithEnding(buffer, offset, size, (byte)',');
        }
        public void EnterArray()
        {
            ReserveSpace(1);
            Debug.Assert(m_buffer != null);
            m_buffer[m_offset] = (byte)'[';
            ++m_offset;
        }
        public void ExitArray()
        {
            CorrectComma();
            ReserveSpace(2);
            Debug.Assert(m_buffer != null);
            m_buffer[m_offset] = (byte)']';
            ++m_offset;
            m_buffer[m_offset] = (byte)',';
            ++m_offset;
        }
        public void EnterObject()
        {
            ReserveSpace(1);
            Debug.Assert(m_buffer != null);
            m_buffer[m_offset] = (byte)'{';
            ++m_offset;
        }
        public void ExitObject()
        {
            CorrectComma();
            ReserveSpace(2);
            Debug.Assert(m_buffer != null);
            m_buffer[m_offset] = (byte)'}';
            ++m_offset;
            m_buffer[m_offset] = (byte)',';
            ++m_offset;
        }
        public void EnterKey(string key)
        {
            SerializeStringWithEnding(key, (byte)':');
        }
        public void EnterKey(byte[] buffer, int offset, int size)
        {
            SerializeStringWithEnding(buffer, offset, size, (byte)':');
        }
        public void Finished()
        {
            CorrectComma();
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
        void CorrectComma()
        {
            if (m_bufferStart != null)
            {
                Debug.Assert(m_buffer != null);
                int size = m_offset - m_bufferStart.Offset;
                Debug.Assert(size >= 0);
                if (size >= 1)
                {
                    if (m_buffer[m_offset - 1] == ',')
                    {
                        --m_offset;
                    }
                }
            }
        }
        void EscapeString(string str)
        {
            Debug.Assert(m_buffer != null);

            for (int i = 0; i < str.Length; i++)
            {
                char c = str[i];
                if (!EscapeByte(c))
                {
                    m_buffer[m_offset] = (byte)'\\';
                    m_offset++;
                    m_buffer[m_offset] = (byte)'u';
                    m_offset++;
                    m_buffer[m_offset] = hexDigits[((c >> 12) & 0x0f)];
                    m_offset++;
                    m_buffer[m_offset] = hexDigits[((c >> 8) & 0x0f)];
                    m_offset++;
                    m_buffer[m_offset] = hexDigits[((c >> 4) & 0x0f)];
                    m_offset++;
                    m_buffer[m_offset] = hexDigits[(c & 0x0f)];
                    m_offset++;
                }
            }
        }

        bool EscapeByte(char c)
        {
            Debug.Assert(m_buffer != null);
            if (c < 0x20)
            {
                m_buffer[m_offset] = (byte)'\\';
                m_offset++;
                byte e = escape[c];
                m_buffer[m_offset] = e;
                m_offset++;
                if (e == 'u')
                {
                    m_buffer[m_offset] = (byte)'0';
                    m_offset++;
                    m_buffer[m_offset] = (byte)'0';
                    m_offset++;
                    m_buffer[m_offset] = hexDigits[(c >> 4)];
                    m_offset++;
                    m_buffer[m_offset] = hexDigits[(c & 0x0f)];
                    m_offset++;
                }
                return true;
            }
            else if (c < 0x80)
            {
                if (c == '\\' || c == '\"')
                {
                    m_buffer[m_offset] = (byte)'\\';
                    m_offset++;
                }
                m_buffer[m_offset] = (byte)c;
                m_offset++;
                return true;
            }
            return false;
        }
        void SerializeStringWithEnding(string value, byte ending)
        {
            int size = value.Length;
            ReserveSpace(size * 6 + 2 + 1); // string*6 + 2" + ending (',' or ':')
            Debug.Assert(m_buffer != null);
            m_buffer[m_offset] = (byte)'\"';
            ++m_offset;
            EscapeString(value);
            m_buffer[m_offset] = (byte)'\"';
            ++m_offset;
            m_buffer[m_offset] = ending;
            ++m_offset;
        }
        void SerializeStringWithEnding(byte[] buffer, int offset, int size, byte ending)
        {
            ReserveSpace(size * 6 + 2 + 1); // string*6 + 2" + ending (',' or ':')
            Debug.Assert(m_buffer != null);
            m_buffer[m_offset] = (byte)'\"';
            ++m_offset;
            bool escapeNeeded = false;
            int end = offset + size;
            int i = offset;
            for (; i < end; ++i)
            {
                byte c = buffer[i];
                if (!EscapeByte((char)c))
                {
                    escapeNeeded = true;
                    break;
                }
            }
            if (escapeNeeded)
            {
                EscapeString(Encoding.UTF8.GetString(buffer, i, end - i));
            }
            m_buffer[m_offset] = (byte)'\"';
            ++m_offset;
            m_buffer[m_offset] = ending;
            ++m_offset;
        }

        IZeroCopyBuffer m_zeroCopybuffer;
        int m_maxBlockSize = 512;
        BufferRef? m_bufferStart = null;
        byte[]? m_buffer;
        int m_offset = 0;
        int m_offsetEnd = 0;

        static byte[] hexDigits = new byte[] { (byte)'0', (byte)'1', (byte)'2', (byte)'3', (byte)'4', (byte)'5', (byte)'6', (byte)'7', (byte)'8', (byte)'9', (byte)'a', (byte)'b', (byte)'c', (byte)'d', (byte)'e', (byte)'f' };

        static byte[] escape = new byte[0x20]
        {
                //0        1          2          3          4          5          6          7          8          9          A          B          C          D          E          F
                (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'b', (byte)'t', (byte)'n', (byte)'u', (byte)'f', (byte)'r', (byte)'u', (byte)'u', // 00
                (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'u', (byte)'u', // 10
        };
    };

}   // namespace finalmq
