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
        int ParseToken(int level, out string token, out bool isarray);
        int ParseTokenArray(int level, IList<string> array);
        int ParseTillEndOfStruct(int level);
        string GetSegmentId();
        int IsNextFieldFilled(int level, out bool filled);
        int GetCurrentPosition();
    }


    public class Hl7Parser : IHl7Parser
    {
        static readonly char SEGMENT_END = (char)0x0D;   // '\r'

        public Hl7Parser()
        {
        }

        char GetChar(int offset)
        {
            Debug.Assert(m_buffer != null);
            return (offset < m_end) ? (char)m_buffer[offset] : (char)0;
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
                    m_delimiterField[1] = (char)m_buffer[m_offset + 3 + 0];    // |
                    m_delimiterField[2] = (char)m_buffer[m_offset + 3 + 1];    // ^
                    m_delimiterField[3] = (char)m_buffer[m_offset + 3 + 4];    // &
                    m_delimiterRepeat = (char)m_buffer[m_offset + 3 + 2];    // ~
                    m_escape = (char)m_buffer[m_offset + 3 + 3];    // '\\'
                }
                else
                {
                    return false;
                }
            }
            return true;
        }

        public int ParseToken(int level, out string token, out bool isarray)
        {
            token = "";
            isarray = false;
            int l = level;
            if (m_waitForDeleimiterField == 1)
            {
                token = Encoding.ASCII.GetString(m_buffer, m_offset - 1, 1);
            }
            else if (m_waitForDeleimiterField == 2)
            {
                if (m_offset + 4 < m_end)
                {
                    token = Encoding.ASCII.GetString(m_buffer, m_offset, 4);
                    m_offset += 4 + 1;
                }
            }
            else
            {
                int start = m_offset;
                while (true)
                {
                    char c = GetChar(m_offset);
                    if (c == m_delimiterRepeat)
                    {
                        token = DeEscape(start, m_offset);
                        ++m_offset;
                        isarray = true;
                        l = 1;  // array is only on level = 1
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
                    l = ParseTillEndOfStructIntern(level, true, out isarray);
                }
            }

            if (m_waitForDeleimiterField < 3)
            {
                ++m_waitForDeleimiterField;
            }

            Debug.Assert(l <= level);
            return l;
        }

        public int ParseTokenArray(int level, IList<string> array)
        {
            array.Clear();
            int start = m_offset;
            int l = 0;
            bool filled = false;
            while (true)
            {
                char c = GetChar(m_offset);
                if (c == m_delimiterRepeat)
                {
                    array.Add(DeEscape(start, m_offset));
                    ++m_offset;
                    start = m_offset;
                    filled = true;
                }
                else
                {
                    l = IsDelimiter(c);
                    if (l < LAYER_MAX)
                    {
                        if (filled || (l > 1))   // array/repeated (~) is on layer 1 possible
                        {
                            array.Add(DeEscape(start, m_offset));
                        }
                        if (l != -1)
                        {
                            ++m_offset;
                        }
                        if (l <= level)
                        {
                            break;
                        }
                    }

                    if (l > level && l < LAYER_MAX)
                    {
                        bool isarray;
                        l = ParseTillEndOfStructIntern(level, true, out isarray);
                        if (l <= level && !isarray)
                        {
                            break;
                        }
                        start = m_offset;
                    }
                    else
                    {
                        ++m_offset;
                    }
                    filled = true;
                }
            }
            Debug.Assert(l <= level);
            return l;
        }

        public string GetSegmentId()
        {
            StringBuilder token = new StringBuilder();
            int i = 0;
            while (true)
            {
                char c = GetChar(m_offset + i);
                if (IsDelimiter(c) == LAYER_MAX)
                {
                    token.Append(c);
                }
                else
                {
                    break;
                }
                ++i;
            }
            return token.ToString();
        }

        public int IsNextFieldFilled(int level, out bool filled)
        {
            filled = false;
            int l = level;
            char c = GetChar(m_offset);
            if (c == m_delimiterRepeat)
            {
                filled = true;
            }
            else
            {
                l = IsDelimiter(c);
                if (l > level)
                {
                    filled = true;
                    l = level;
                }
            }
            if (!filled)
            {
                ++m_offset;
            }
            Debug.Assert(l <= level);
            return l;
        }

        public int GetCurrentPosition()
        {
            return m_offset;
        }

        public int ParseTillEndOfStruct(int level)
        {
            bool isarrayDummy;
            return ParseTillEndOfStructIntern(level, false, out isarrayDummy);
        }

        public int ParseTillEndOfStructIntern(int level, bool stopOnArray, out bool isarray)
        {
            isarray = false;
            char c;
            while ((c = GetChar(m_offset)) != (char)0)
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
                else if (stopOnArray && c == m_delimiterRepeat)
                {
                    isarray = true;
                    ++m_offset;
                    return 1;
                }
                ++m_offset;
            }
            return -1;
        }

        void SkipControlCharacters()
        {
            while (true)
            {
                char c = GetChar(m_offset);
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

        int IsDelimiter(char c)
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


        static char Hex2char(char c)
        {
            char num = (char)0;
            if ('0' <= c && c <= '9')
            {
                num = (char)(c - '0');
            }
            else if ('a' <= c && c <= 'f')
            {
                num = (char)((c - 'a') + 10);
            }
            else if ('A' <= c && c <= 'F')
            {
                num = (char)((c - 'A') + 10);
            }
            else
            {
                num = (char)0xff;
            }
            return num;
        }


        string DeEscape(int start, int end)
        {
            Debug.Assert(m_buffer != null);
            StringBuilder dest = new StringBuilder();
            int off = start;
            while (off < end)
            {
                char c = (char)m_buffer[off];
                if (c == m_escape)
                {
                    ++off;
                    if (off >= end)
                    {
                        return dest.ToString();
                    }
                    c = (char)m_buffer[off];
                    switch (c)
                    {
                        case 'E':
                            dest.Append(m_escape);
                            ++off;
                            break;
                        case 'F':
                            dest.Append(m_delimiterField[1]);    // |
                            ++off;
                            break;
                        case 'R':
                            dest.Append(m_delimiterRepeat);      // ~
                            ++off;
                            break;
                        case 'S':
                            dest.Append(m_delimiterField[2]);    // ^
                            ++off;
                            break;
                        case 'T':
                            dest.Append(m_delimiterField[3]);    // &
                            ++off;
                            break;
                        case 'X':
                            while (true)
                            {
                                ++off;
                                if (off >= end)
                                {
                                    break;
                                }
                                c = (char)m_buffer[off];
                                if (c == '\\')
                                {
                                    break;
                                }
                                char num = Hex2char(c);
                                if (num == 0xff)
                                {
                                    return dest.ToString();
                                }
                                char d = num;
                                ++off;
                                if (off >= end)
                                {
                                    break;
                                }
                                c = (char)m_buffer[off];
                                num = Hex2char(c);
                                if (num == 0xff)
                                {
                                    return dest.ToString();
                                }
                                d <<= 4;
                                d |= num;
                                dest.Append(d);
                            }
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    dest.Append(c);
                }
                ++off;
            }

            return dest.ToString();
        }

        static readonly int LAYER_MAX = 4;

        byte[] m_buffer = Array.Empty<byte>();
        int m_offset = 0;
        int m_end = 0;

        char[] m_delimiterField = new char[4] { (char)0, (char)0, (char)0, (char)0 };
        char m_delimiterRepeat = (char)0;
        char m_escape = (char)0;

        int m_waitForDeleimiterField = 0;
    }

}   // namespace finalmq
