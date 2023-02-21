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

    public interface IHl7Parser
    {
        bool StartParse(byte[] buffer, int offset, int size = -1);
        int ParseToken(int level, out byte[] token);
        int ParseTokenArray(int level, IList<byte[]> array);
        int ParseTillEndOfStruct(int level);
        byte[] GetSegmentId();
        int GetCurrentPosition();
    }


    public class Hl7Parser : IHl7Parser
    {
        static readonly byte SEGMENT_END = 0x0D;   // '\r'

        public Hl7Parser()
        {
        }

        byte GetChar(int offset)
        {
            Debug.Assert(m_buffer != null);
            return (offset < m_end) ? m_buffer[offset] : (byte)0;
        }

        public bool StartParse(byte[] buffer, int offset, int size = -1)
        {
            m_buffer = buffer;
            m_offset = offset;
            Debug.Assert(size >= -1);
            if (size >= -1)
            {
                if (size == -1)
                {
                    size = buffer.Length - offset;
                }
                m_end = offset + size;
                SkipControlCharacters();

                if (size < 8)
                {
                    return false;
                }

                if (m_buffer[m_offset + 0] == 'M' && m_buffer[m_offset + 1] == 'S' && m_buffer[m_offset + 2] == 'H')
                {
                    m_delimiterField[0] = SEGMENT_END;      // segment delimiter '\r', 0x0D
                    m_delimiterField[1] = m_buffer[m_offset + 3 + 0];    // |
                    m_delimiterField[2] = m_buffer[m_offset + 3 + 1];    // ^
                    m_delimiterField[3] = m_buffer[m_offset + 3 + 4];    // &
                    m_delimiterRepeat = m_buffer[m_offset + 3 + 2];    // ~
                    m_escape = m_buffer[m_offset + 3 + 3];    // '\\'
                }
                else
                {
                    return false;
                }
            }
            return true;
        }

        public int ParseToken(int level, out byte[] token)
        {
            token = Array.Empty<byte>();
            int l = level;
            if (m_waitForDeleimiterField == 1)
            {
                token = new byte[] { m_buffer[m_offset - 1] };
            }
            else if (m_waitForDeleimiterField == 2)
            {
                if (m_offset + 4 < m_end)
                {
                    token = new byte[4] { m_buffer[m_offset], m_buffer[m_offset + 1], m_buffer[m_offset + 2], m_buffer[m_offset + 3] };
                    m_offset += 4 + 1;
                }
            }
            else
            {
                int start = m_offset;
                while (true)
                {
                    byte c = GetChar(m_offset);
                    if (c == m_delimiterRepeat)
                    {
                        token = DeEscape(start, m_offset);
                        ++m_offset;
                        l = ParseTillEndOfStruct(level);
                        break;
                    }
                    else
                    {
                        l = IsDelimiter(c);
                        if (l < LAYER_MAX)
                        {
                            token = DeEscape(start, m_offset);
                            if (l != -1)
                            {
                                ++m_offset;
                            }
                            break;
                        }
                    }
                    ++m_offset;
                }
                if (l > level)
                {
                    l = ParseTillEndOfStruct(level);
                }
            }

            if (m_waitForDeleimiterField < 3)
            {
                ++m_waitForDeleimiterField;
            }

            Debug.Assert(l <= level);
            return l;
        }

        public int ParseTokenArray(int level, IList<byte[]> array)
        {
            array.Clear();
            int start = m_offset;
            int l = 0;
            while (true)
            {
                byte c = GetChar(m_offset);
                if (c == m_delimiterRepeat)
                {
                    array.Add(DeEscape(start, m_offset));
                    ++m_offset;
                    start = m_offset;
                }
                else
                {
                    l = IsDelimiter(c);
                    if (l < LAYER_MAX)
                    {
                        array.Add(DeEscape(start, m_offset));
                        if (l != -1)
                        {
                            ++m_offset;
                        }
                        break;
                    }
                    ++m_offset;
                }
            }
            if (l > level)
            {
                l = ParseTillEndOfStruct(level);
            }
            Debug.Assert(l <= level);
            return l;
        }

        public byte[] GetSegmentId()
        {
            ArrayBuilder<byte> token = new ArrayBuilder<byte>();
            int i = 0;
            while (true)
            {
                byte c = GetChar(m_offset + i);
                if (IsDelimiter(c) == LAYER_MAX)
                {
                    token.Add(c);
                }
                else
                {
                    break;
                }
                ++i;
            }
            return token.ToArray();
        }

        public int GetCurrentPosition()
        {
            return m_offset;
        }

        public int ParseTillEndOfStruct(int level)
        {
            byte c;
            while ((c = GetChar(m_offset)) != 0)
            {
                int l = IsDelimiter(c);
                if (l <= level)
                {
                    if (l != -1)
                    {
                        ++m_offset;
                    }
                    return l;
                }
                ++m_offset;
            }
            return -1;
        }

        void SkipControlCharacters()
        {
            while (true)
            {
                byte c = GetChar(m_offset);
                if (c < 0x20 && c != 0)
                {
                    ++m_offset;
                }
                else
                {
                    break;
                }
            }
        }

        int IsDelimiter(byte c)
        {
            if (c == 0)
            {
                return -1;
            }
            int i;
            for (i = 0; i < LAYER_MAX; ++i)
            {
                if (c == m_delimiterField[i])
                {
                    break;
                }
            }

            return i;
        }


        static byte Hex2char(byte c)
        {
            byte num = 0;
            if ('0' <= c && c <= '9')
            {
                num = (byte)(c - '0');
            }
            else if ('a' <= c && c <= 'f')
            {
                num = (byte)((c - 'a') + 10);
            }
            else if ('A' <= c && c <= 'F')
            {
                num = (byte)((c - 'A') + 10);
            }
            else
            {
                num = 0xff;
            }
            return num;
        }


        byte[] DeEscape(int start, int end)
        {
            Debug.Assert(m_buffer != null);
            ArrayBuilder<byte> dest = new ArrayBuilder<byte>();
            int off = start;
            while (off < end)
            {
                byte c = m_buffer[off];
                if (c == m_escape)
                {
                    ++off;
                    if (off >= end)
                    {
                        return dest.ToArray();
                    }
                    c = m_buffer[off];
                    switch (c)
                    {
                        case (byte)'E':
                            dest.Add(m_escape);
                            ++off;
                            break;
                        case (byte)'F':
                            dest.Add(m_delimiterField[1]);    // |
                            ++off;
                            break;
                        case (byte)'R':
                            dest.Add(m_delimiterRepeat);      // ~
                            ++off;
                            break;
                        case (byte)'S':
                            dest.Add(m_delimiterField[2]);    // ^
                            ++off;
                            break;
                        case (byte)'T':
                            dest.Add(m_delimiterField[3]);    // &
                            ++off;
                            break;
                        case (byte)'X':
                            while (true)
                            {
                                ++off;
                                if (off >= end)
                                {
                                    break;
                                }
                                c = m_buffer[off];
                                if (c == '\\')
                                {
                                    break;
                                }
                                byte num = Hex2char(c);
                                if (num == 0xff)
                                {
                                    return dest.ToArray();
                                }
                                byte d = num;
                                ++off;
                                if (off >= end)
                                {
                                    break;
                                }
                                c = m_buffer[off];
                                num = Hex2char(c);
                                if (num == 0xff)
                                {
                                    return dest.ToArray();
                                }
                                d <<= 4;
                                d |= num;
                                dest.Add(d);
                            }
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    dest.Add(c);
                }
                ++off;
            }

            return dest.ToArray();
        }

        static readonly int LAYER_MAX = 4;

        byte[] m_buffer = Array.Empty<byte>();
        int m_offset = 0;
        int m_end = 0;

        byte[] m_delimiterField = new byte[4] { 0, 0, 0, 0 };
        byte m_delimiterRepeat = 0;
        byte m_escape = 0;

        int m_waitForDeleimiterField = 0;
    }

}   // namespace finalmq
