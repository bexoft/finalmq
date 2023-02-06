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

    public interface IJsonParserVisitor
    {
        void SyntaxError(string buffer, string message);
        void EnterNull();
        void EnterBool(bool value);
        void EnterInt32(int value);
        void EnterUInt32(uint value);
        void EnterInt64(long value);
        void EnterUInt64(ulong value);
        void EnterDouble(double value);
        void EnterString(string value);
        void EnterString(byte[] buffer, int offset, int size);
        void EnterArray();
        void ExitArray();
        void EnterObject();
        void ExitObject();
        void EnterKey(string key);
        void EnterKey(byte[] buffer, int offset, int size);
        void Finished();
    };


    public class JsonParser
    {
        public JsonParser(IJsonParserVisitor visitor)
        {
            m_visitor = visitor;
        }

        public int Parse(byte[] buffer, int offset = 0, int size = -1)
        {
            if (size == -1)
            {
                size = buffer.Length - offset;
            }
            m_end = offset + size;
            Debug.Assert(m_end <= buffer.Length);
            m_buffer = buffer;
            int ret = ParseValue(offset);
            m_visitor.Finished();
            return ret;
        }

        public int Parse(string json)
        {
            byte[] buffer = Encoding.UTF8.GetBytes(json);
            return Parse(buffer);
        }

        string GetCurrentText(int offset)
        {
            int cnt = 20;
            int remaining = m_end - offset;
            if (cnt > remaining)
            {
                cnt = remaining;
            }
            return Encoding.UTF8.GetString(m_buffer, offset, cnt);
        }

        int ParseValue(int offset)
        {
            offset = ParseWhiteSpace(offset);
            char c = GetChar(offset);
            if (c == 0)
            {
                m_visitor.SyntaxError(GetCurrentText(offset), "value expected");
                return -1;
            }
            switch (c)
            {
                // string
                case '\"':
                    offset = ParseString(offset, false);
                    break;
                // object
                case '{':
                    offset = ParseObject(offset);
                    break;
                case '[':
                    offset = ParseArray(offset);
                    break;
                case 'n':
                    offset = ParseNull(offset);
                    break;
                case 't':
                    offset = ParseTrue(offset);
                    break;
                case 'f':
                    offset = ParseFalse(offset);
                    break;
                default:
                    offset = ParseNumber(offset);
                    break;
            }
            if (offset != -1)
            {
                offset = ParseWhiteSpace(offset);
                return offset;
            }
            return -1;
        }

        int ParseWhiteSpace(int offset)
        {
            char c;
            while ((c = GetChar(offset)) != 0)
            {
                switch (c)
                {
                    case ' ':
                    case '\t':
                    case '\n':
                    case '\r':
                        offset++;
                        break;
                    default:
                        return offset;
                }
            }
            return offset;
        }
        int ParseObject(int offset)
        {
            m_visitor.EnterObject();
            // skip '{'
            offset++;
            offset = ParseWhiteSpace(offset);

            while (GetChar(offset) != 0)
            {
                offset = ParseWhiteSpace(offset);
                char c = GetChar(offset);
                if (c == '}')
                {
                    offset++;
                    m_visitor.ExitObject();
                    return offset;
                }
                if (c != '\"')
                {
                    m_visitor.SyntaxError(GetCurrentText(offset), "'\"' for key expected");
                    return -1;
                }
                offset = ParseString(offset, true);
                if (offset == -1)
                {
                    return -1;
                }
                offset = ParseWhiteSpace(offset);

                if (GetChar(offset) != ':')
                {
                    m_visitor.SyntaxError(GetCurrentText(offset), "':' expected");
                    return -1;
                }

                offset++;
                offset = ParseValue(offset);
                if (offset == -1)
                {
                    return -1;
                }
                c = GetChar(offset);
                if (c != ',' && c != '}')
                {
                    m_visitor.SyntaxError(GetCurrentText(offset), "',' or '}' expected");
                    return -1;
                }
                if (c == ',')
                {
                    offset++;
                }
            }
            m_visitor.SyntaxError(GetCurrentText(offset), "',' or '}' expected");
            return -1;
        }
        int ParseArray(int offset)
        {
            m_visitor.EnterArray();

            // skip '['
            offset++;

            while (GetChar(offset) != 0)
            {
                offset = ParseWhiteSpace(offset);
                if (GetChar(offset) == ']')
                {
                    offset++;
                    m_visitor.ExitArray();
                    return offset;
                }
                offset = ParseValue(offset);
                if (offset == -1)
                {
                    return -1;
                }
                char c = GetChar(offset);
                if (c != ',' && c != ']')
                {
                    m_visitor.SyntaxError(GetCurrentText(offset), "',' or ']' expected");
                    return -1;
                }
                if (c == ',')
                {
                    offset++;
                }
            }
            m_visitor.SyntaxError(GetCurrentText(offset), "',' or ']' expected");
            return -1;
        }
        int ParseNumber(int offset)
        {
            int first = offset;
            char c = GetChar(offset);
            bool isNegative = false;
            if (c == '-')
            {
                isNegative = true;
                offset++;
                c = GetChar(offset);
                if (c >= '0' && c <= '9')
                {
                    offset++;
                }
                else
                {
                    m_visitor.SyntaxError(GetCurrentText(offset), "digit expected");
                    return -1;
                }
            }
            else if ((c >= '0' && c <= '9') || (c == '-'))
            {
                offset++;
            }
            else
            {
                m_visitor.SyntaxError(GetCurrentText(offset), "digit expected");
                return -1;
            }

            bool isFloat = false;
            while ((c = GetChar(offset)) != 0)
            {
                if ((c >= '0' && c <= '9') || (c == '+') || (c == '-'))
                {
                    offset++;
                }
                else if ((c == '.') || (c == 'e') || (c == 'E'))
                {
                    offset++;
                    isFloat = true;
                }
                else
                {
                    break;
                }
            }

            if (isFloat)
            {
                string strNumber = Encoding.ASCII.GetString(m_buffer, first, offset - first);
                double value;
                if (!Double.TryParse(strNumber, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out value))
                {
                    m_visitor.SyntaxError(GetCurrentText(first), "wrong number format");
                    return -1;
                }
                m_visitor.EnterDouble(value);
            }
            else if (isNegative)
            {
                bool ok;
                long value = Convertion.ConvertByteStringToInt64(m_buffer, first, offset - first, out ok);
                if (!ok)
                {
                    m_visitor.SyntaxError(GetCurrentText(first), "wrong number format");
                    return -1;
                }
                Debug.Assert(value < 0);
                if (value >= Int32.MinValue)
                {
                    m_visitor.EnterInt32((int)value);
                }
                else
                {
                    m_visitor.EnterInt64(value);
                }
            }
            else
            {
                bool ok;
                ulong value = Convertion.ConvertByteStringToUInt64(m_buffer, first, offset - first, out ok);
                if (!ok)
                {
                    m_visitor.SyntaxError(GetCurrentText(first), "wrong number format");
                    return -1;
                }
                if (value <= UInt32.MaxValue)
                {
                    m_visitor.EnterUInt32((uint)value);
                }
                else
                {
                    m_visitor.EnterUInt64(value);
                }
            }

            return offset;
        }
        int ParseString(int offset, bool key)
        {
            // skip '"'
            offset++;

            int offsetBegin = offset;

            // try fast parse. it is fast when there is no escape character in the string
            char c;
            while ((c = GetChar(offset)) != 0)
            {
                if (c == '\"')
                {
                    int size = offset - offsetBegin;
                    if (key)
                    {
                        m_visitor.EnterKey(m_buffer, offsetBegin, size);
                    }
                    else
                    {
                        m_visitor.EnterString(m_buffer, offsetBegin, size);
                    }
                    offset++;
                    return offset;
                }
                else if (c == '\\')
                {
                    break;
                }
                offset++;
            }

            if (c == 0)
            {
                m_visitor.SyntaxError(GetCurrentText(offset), "'\"' expected");
                return -1;
            }

            // fast parse was not possible, go ahead with escaping
            if (m_dest == null)
            {
                m_dest = new StringBuilder(2048);
            }
            else
            {
                m_dest.Clear();
            }
            m_dest.Append(Encoding.UTF8.GetString(m_buffer, offsetBegin, offset - offsetBegin));
            while ((c = GetChar(offset)) != 0)
            {
                if (c == '\"')
                {
                    string value = m_dest.ToString();
                    if (key)
                    {
                        m_visitor.EnterKey(value);
                    }
                    else
                    {
                        m_visitor.EnterString(value);
                    }
                    offset++;
                    return offset;
                }
                else if (c == '\\')
                {
                    offset++;
                    if ((c = GetChar(offset)) != 0)
                    {
                        switch (c)
                        {
                            case '\"':
                            case '\\':
                            case '/':
                                m_dest.Append(c);
                                break;
                            case 'b':
                                m_dest.Append('\b');
                                break;
                            case 'f':
                                m_dest.Append('\f');
                                break;
                            case 'n':
                                m_dest.Append('\n');
                                break;
                            case 'r':
                                m_dest.Append('\r');
                                break;
                            case 't':
                                m_dest.Append('\t');
                                break;
                            case 'u':
                                {
                                    offset++;
                                    int num = 0;
                                    offset = ParseUEscape(offset, out num);
                                    if (offset == -1)
                                    {
                                        return -1;
                                    }
                                    if (num >= 0xD800 && num <= 0xDBFF)
                                    {
                                        if (GetChar(offset) != '\\')
                                        {
                                            m_visitor.SyntaxError(GetCurrentText(offset), "'\\' expected");
                                            return -1;
                                        }
                                        offset++;
                                        if (GetChar(offset) != 'u')
                                        {
                                            m_visitor.SyntaxError(GetCurrentText(offset), "'u' expected");
                                            return -1;
                                        }
                                        offset++;
                                        int num2 = 0;
                                        offset = ParseUEscape(offset, out num2);
                                        if (offset == -1)
                                        {
                                            return -1;
                                        }
                                        if (num2 < 0xDC00 || num2 > 0xDFFF)
                                        {
                                            m_visitor.SyntaxError(GetCurrentText(offset), "wrong utf16 value");
                                            return -1;
                                        }
                                        //num += num2 << 16;
                                        num = (((num - 0xD800) << 10) | (num2 - 0xDC00)) + 0x10000;
                                    }
                                    else if (num > 0xDBFF && num <= 0xDFFF)
                                    {
                                        m_visitor.SyntaxError(GetCurrentText(offset), "wrong utf16 valueh");
                                        return -1;
                                    }
                                    offset--;

                                    if (num <= 0x7F)
                                    {
                                        m_dest.Append((char)(num & 0xff));
                                    }
                                    else
                                    {
                                        Debug.Assert(num <= 0x10FFFF);
                                        m_dest.Append(Char.ConvertFromUtf32(num));
                                    }
                                }
                                break;
                            default:
                                m_dest.Append('\\');
                                m_dest.Append(c);
                                break;
                        }
                    }
                }
                else
                {
                    m_dest.Append(c);
                }
                offset++;
            }
            m_visitor.SyntaxError(GetCurrentText(offset), "'\"' expected");
            return -1;
        }
        int ParseNull(int offset)
        {
            offset = CmpString(offset, "null");
            if (offset >= 0)
            {
                m_visitor.EnterNull();
            }
            return offset;
        }
        int ParseTrue(int offset)
        {
            offset = CmpString(offset, "true");
            if (offset >= 0)
            {
                m_visitor.EnterBool(true);
            }
            return offset;
        }
        int ParseFalse(int offset)
        {
            offset = CmpString(offset, "false");
            if (offset >= 0)
            {
                m_visitor.EnterBool(false);
            }
            return offset;
        }
        int CmpString(int offset, string strCmp)
        {
            for (int i = 0; i < strCmp.Length; ++i)
            {
                char c = strCmp[i];
                if (GetChar(offset) != c)
                {
                    string message = "";
                    message += c;
                    message += " expected";
                    m_visitor.SyntaxError(GetCurrentText(offset), message);
                    return -1;
                }
                offset++;
            }

            return offset;
        }

        static int GetHexDigit(char c)
        {
            if (c >= '0' && c <= '9')
            {
                return c - '0';
            }
            else if (c >= 'a' && c <= 'f')
            {
                return 10 + c - 'a';
            }
            else if (c >= 'A' && c <= 'F')
            {
                return 10 + c - 'A';
            }
            return -1;
        }

        int ParseUEscape(int offset, out int value)
        {
            value = 0;
            for (int i = 0; i < 4; ++i)
            {
                int v = GetHexDigit(GetChar(offset));
                if (v == -1)
                {
                    m_visitor.SyntaxError(GetCurrentText(offset), "invalid u escape");
                    return -1;
                }
                v <<= 12 - 4 * i;
                value += v;
                offset++;
            }
            return offset;
        }

        /*
        byte GetByte(int offset)
        {
            return ((offset < m_end) ? m_buffer[offset] : (byte)0);
        }
        */
        char GetChar(int offset)
        {
            return ((offset < m_end) ? (char)m_buffer[offset] : '\0');
        }

        IJsonParserVisitor m_visitor;
        byte[] m_buffer = Array.Empty<byte>();
        int m_end = 0;
        StringBuilder? m_dest = null;
    };

}   // namespace finalmq
