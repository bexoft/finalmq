using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace finalmq
{
    
    class ParserJson : IJsonParserVisitor
    {

        public ParserJson(IParserVisitor visitor, byte[] buffer, int offset = 0, int size = -1)
        {
            m_parser = new JsonParser(this);
            m_buffer = buffer;
            m_offset = offset;
            m_size = size;
            m_visitor = visitor;
        }

        public int ParseStruct(string typeName)
        {
            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct(typeName);
            if (stru == null)
            {
                m_visitor.NotifyError(Encoding.UTF8.GetString(m_buffer, m_offset, 20), "typename not found");
                m_visitor.Finished();
                return -1;
            }

            MetaField field = new MetaField(MetaTypeId.TYPE_STRUCT, typeName, "", "", 0, -1);
            field.MetaStruct = stru;
            m_fieldCurrent = field;

            m_visitor.StartStruct(stru);
            int res = m_parser.Parse(m_buffer, m_offset, m_size);
            m_visitor.Finished();

            return res;
        }

        // IJsonParserVisitor
        public void SyntaxError(string buffer, string message)
        {

        }
        public void EnterNull()
        {

        }
        public void EnterBool(bool value)
        {
            EnterNumber(value);
        }
        public void EnterInt32(int value)
        {
            EnterNumber(value);
        }
        public void EnterUInt32(uint value)
        {
            EnterNumber(value);
        }
        public void EnterInt64(long value)
        {
            EnterNumber(value);
        }
        public void EnterUInt64(ulong value)
        {
            EnterNumber(value);
        }
        public void EnterDouble(double value)
        {
            EnterNumber(value);
        }
        public void EnterString(string value)
        {
            if (m_fieldCurrent == null)
            {
                // unknown key
                return;
            }
            switch (m_fieldCurrent.TypeId)
            {
                case MetaTypeId.TYPE_BOOL:
                    {
                        bool v = (value == "true");
                        m_visitor.EnterBool(m_fieldCurrent, v);
                    }
                    break;
                case MetaTypeId.TYPE_INT32:
                    {
                        int v = Convertion.Convert<int>(value);
                        m_visitor.EnterInt32(m_fieldCurrent, v);
                    }
                    break;
                case MetaTypeId.TYPE_UINT32:
                    {
                        uint v = Convertion.Convert<uint>(value);
                        m_visitor.EnterUInt32(m_fieldCurrent, v);
                    }
                    break;
                case MetaTypeId.TYPE_INT64:
                    {
                        long v = Convertion.Convert<long>(value);
                        m_visitor.EnterInt64(m_fieldCurrent, v);
                    }
                    break;
                case MetaTypeId.TYPE_UINT64:
                    {
                        ulong v = Convertion.Convert<ulong>(value);
                        m_visitor.EnterUInt64(m_fieldCurrent, v);
                    }
                    break;
                case MetaTypeId.TYPE_FLOAT:
                    {
                        float v = Convertion.Convert<float>(value);
                        m_visitor.EnterFloat(m_fieldCurrent, v);
                    }
                    break;
                case MetaTypeId.TYPE_DOUBLE:
                    {
                        double v = Convertion.Convert<double>(value);
                        m_visitor.EnterDouble(m_fieldCurrent, v);
                    }
                    break;
                case MetaTypeId.TYPE_STRING:
                    {
                        m_visitor.EnterString(m_fieldCurrent, value);
                    }
                    break;
                case MetaTypeId.TYPE_BYTES:
                    {
                        // convert from base64
                        byte[] v = System.Convert.FromBase64String(value);
                        m_visitor.EnterBytes(m_fieldCurrent, v);
                    }
                    break;
                case MetaTypeId.TYPE_ENUM:
                    {
                        m_visitor.EnterEnum(m_fieldCurrent, value);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_BOOL:
                    {
                        bool v = (value == "true");
                        if (m_arrayBool == null)
                        {
                            m_arrayBool = new ArrayBuilder<bool>();
                        }
                        m_arrayBool.Add(v);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_INT32:
                    {
                        if (m_arrayInt32 == null)
                        {
                            m_arrayInt32 = new ArrayBuilder<int>();
                        }
                        int v = Convertion.Convert<int>(value);
                        m_arrayInt32.Add(v);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_UINT32:
                    {
                        if (m_arrayUInt32 == null)
                        {
                            m_arrayUInt32 = new ArrayBuilder<uint>();
                        }
                        uint v = Convertion.Convert<uint>(value);
                        m_arrayUInt32.Add(v);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_INT64:
                    {
                        if (m_arrayInt64 == null)
                        {
                            m_arrayInt64 = new ArrayBuilder<long>();
                        }
                        long v = Convertion.Convert<long>(value);
                        m_arrayInt64.Add(v);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_UINT64:
                    {
                        if (m_arrayUInt64 == null)
                        {
                            m_arrayUInt64 = new ArrayBuilder<ulong>();
                        }
                        ulong v = Convertion.Convert<ulong>(value);
                        m_arrayUInt64.Add(v);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_FLOAT:
                    {
                        if (m_arrayFloat == null)
                        {
                            m_arrayFloat = new ArrayBuilder<float>();
                        }
                        float v = Convertion.Convert<float>(value);
                        m_arrayFloat.Add(v);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_DOUBLE:
                    {
                        if (m_arrayDouble == null)
                        {
                            m_arrayDouble = new ArrayBuilder<double>();
                        }
                        double v = Convertion.Convert<double>(value);
                        m_arrayDouble.Add(v);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_STRING:
                    {
                        if (m_arrayString == null)
                        {
                            m_arrayString = new List<string>();
                        }
                        m_arrayString.Add(value);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_BYTES:
                    {
                        if (m_arrayBytes == null)
                        {
                            m_arrayBytes = new List<byte[]>();
                        }
                        // convert from base64
                        byte[] v = System.Convert.FromBase64String(value);
                        m_arrayBytes.Add(v);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_ENUM:
                    if (((m_arrayString != null) && (m_arrayString.Count != 0)) ||
                        ((m_arrayInt32 == null) || (m_arrayInt32.Length == 0)))
                    {

                        if (m_arrayString == null)
                        {
                            m_arrayString = new List<string>();
                        }
                        m_arrayString.Add(value);
                    }
                    else
                    {
                        if (m_arrayInt32 == null)
                        {
                            m_arrayInt32 = new ArrayBuilder<int>();
                        }
                        int valueInt = MetaDataGlobal.Instance.GetEnumValueByName(m_fieldCurrent, value);
                        m_arrayInt32.Add(valueInt);
                    }
                    break;
                default:
                    //todo                    streamError << "string not expected";
                    break;
            }
        }
        public void EnterString(byte[] buffer, int offset, int size)
        {
            if (m_fieldCurrent == null)
            {
                // unknown key
                return;
            }
            switch (m_fieldCurrent.TypeId)
            {
                case MetaTypeId.TYPE_BOOL:
                    {
                        bool v = IsTrue(buffer, offset, size);
                        m_visitor.EnterBool(m_fieldCurrent, v);
                    }
                    break;
                case MetaTypeId.TYPE_INT32:
                    {
                        bool ok;
                        int v = Convertion.ConvertByteStringToInt32(buffer, offset, size, out ok);
                        m_visitor.EnterInt32(m_fieldCurrent, v);
                    }
                    break;
                case MetaTypeId.TYPE_UINT32:
                    {
                        bool ok;
                        uint v = Convertion.ConvertByteStringToUInt32(buffer, offset, size, out ok);
                        m_visitor.EnterUInt32(m_fieldCurrent, v);
                    }
                    break;
                case MetaTypeId.TYPE_INT64:
                    {
                        bool ok;
                        long v = Convertion.ConvertByteStringToInt64(buffer, offset, size, out ok);
                        m_visitor.EnterInt64(m_fieldCurrent, v);
                    }
                    break;
                case MetaTypeId.TYPE_UINT64:
                    {
                        bool ok;
                        ulong v = Convertion.ConvertByteStringToUInt64(buffer, offset, size, out ok);
                        m_visitor.EnterUInt64(m_fieldCurrent, v);
                    }
                    break;
                case MetaTypeId.TYPE_FLOAT:
                    {
                        string s = Encoding.UTF8.GetString(buffer, offset, size);
                        float v = Convertion.Convert<float>(s);
                        m_visitor.EnterFloat(m_fieldCurrent, v);
                    }
                    break;
                case MetaTypeId.TYPE_DOUBLE:
                    {
                        string s = Encoding.UTF8.GetString(buffer, offset, size);
                        double v = Convertion.Convert<double>(s);
                        m_visitor.EnterDouble(m_fieldCurrent, v);
                    }
                    break;
                case MetaTypeId.TYPE_STRING:
                    {
                        m_visitor.EnterString(m_fieldCurrent, buffer, offset, size);
                    }
                    break;
                case MetaTypeId.TYPE_BYTES:
                    {
                        // convert from base64
                        string s = Encoding.UTF8.GetString(buffer, offset, size);
                        byte[] v = System.Convert.FromBase64String(s);
                        m_visitor.EnterBytes(m_fieldCurrent, v);
                    }
                    break;
                case MetaTypeId.TYPE_ENUM:
                    {
                        string s = Encoding.UTF8.GetString(buffer, offset, size);
                        m_visitor.EnterEnum(m_fieldCurrent, s);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_BOOL:
                    {
                        bool v = IsTrue(buffer, offset, size);
                        if (m_arrayBool == null)
                        {
                            m_arrayBool = new ArrayBuilder<bool>();
                        }
                        m_arrayBool.Add(v);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_INT32:
                    {
                        if (m_arrayInt32 == null)
                        {
                            m_arrayInt32 = new ArrayBuilder<int>();
                        }
                        bool ok;
                        int v = Convertion.ConvertByteStringToInt32(buffer, offset, size, out ok);
                        m_arrayInt32.Add(v);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_UINT32:
                    {
                        if (m_arrayUInt32 == null)
                        {
                            m_arrayUInt32 = new ArrayBuilder<uint>();
                        }
                        bool ok;
                        uint v = Convertion.ConvertByteStringToUInt32(buffer, offset, size, out ok);
                        m_arrayUInt32.Add(v);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_INT64:
                    {
                        if (m_arrayInt64 == null)
                        {
                            m_arrayInt64 = new ArrayBuilder<long>();
                        }
                        bool ok;
                        long v = Convertion.ConvertByteStringToInt64(buffer, offset, size, out ok);
                        m_arrayInt64.Add(v);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_UINT64:
                    {
                        if (m_arrayUInt64 == null)
                        {
                            m_arrayUInt64 = new ArrayBuilder<ulong>();
                        }
                        bool ok;
                        ulong v = Convertion.ConvertByteStringToUInt64(buffer, offset, size, out ok);
                        m_arrayUInt64.Add(v);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_FLOAT:
                    {
                        if (m_arrayFloat == null)
                        {
                            m_arrayFloat = new ArrayBuilder<float>();
                        }
                        string s = Encoding.UTF8.GetString(buffer, offset, size);
                        float v = Convertion.Convert<float>(s);
                        m_arrayFloat.Add(v);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_DOUBLE:
                    {
                        if (m_arrayDouble == null)
                        {
                            m_arrayDouble = new ArrayBuilder<double>();
                        }
                        string s = Encoding.UTF8.GetString(buffer, offset, size);
                        double v = Convertion.Convert<double>(s);
                        m_arrayDouble.Add(v);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_STRING:
                    {
                        if (m_arrayString == null)
                        {
                            m_arrayString = new List<string>();
                        }
                        string s = Encoding.UTF8.GetString(buffer, offset, size);
                        m_arrayString.Add(s);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_BYTES:
                    {
                        if (m_arrayBytes == null)
                        {
                            m_arrayBytes = new List<byte[]>();
                        }
                        // convert from base64
                        string s = Encoding.UTF8.GetString(buffer, offset, size);
                        byte[] v = System.Convert.FromBase64String(s);
                        m_arrayBytes.Add(v);
                    }
                    break;
                case MetaTypeId.TYPE_ARRAY_ENUM:
                    if (((m_arrayString != null) && (m_arrayString.Count != 0)) || 
                        ((m_arrayInt32 == null) || (m_arrayInt32.Length == 0)))
                    {

                        if (m_arrayString == null)
                        {
                            m_arrayString = new List<string>();
                        }
                        string s = Encoding.UTF8.GetString(buffer, offset, size);
                        m_arrayString.Add(s);
                    }
                    else
                    {
                        if (m_arrayInt32 == null)
                        {
                            m_arrayInt32 = new ArrayBuilder<int>();
                        }
                        string s = Encoding.UTF8.GetString(buffer, offset, size);
                        int valueInt = MetaDataGlobal.Instance.GetEnumValueByName(m_fieldCurrent, s);
                        m_arrayInt32.Add(valueInt);
                    }
                    break;
                default:
//todo                    streamError << "string not expected";
                    break;
            }
        }
        public void EnterArray()
        {

        }
        public void ExitArray()
        {

        }
        public void EnterObject()
        {

        }
        public void ExitObject()
        {

        }
        public void EnterKey(string key)
        {

        }
        public void EnterKey(byte[] buffer, int offset, int size)
        {

        }
        public void Finished()
        {

        }


        void EnterNumber<T>(T value)
        {
            Debug.Assert(value != null);
            if (m_fieldCurrent == null)
            {
                // unknown key
                return;
            }
            dynamic v = value;
            switch (m_fieldCurrent.TypeId)
            {
                case MetaTypeId.TYPE_BOOL:
                    m_visitor.EnterBool(m_fieldCurrent, v);
                    break;
                case MetaTypeId.TYPE_INT32:
                    m_visitor.EnterInt32(m_fieldCurrent, v);
                    break;
                case MetaTypeId.TYPE_UINT32:
                    m_visitor.EnterUInt32(m_fieldCurrent, v);
                    break;
                case MetaTypeId.TYPE_INT64:
                    m_visitor.EnterInt64(m_fieldCurrent, v);
                    break;
                case MetaTypeId.TYPE_UINT64:
                    m_visitor.EnterUInt64(m_fieldCurrent, v);
                    break;
                case MetaTypeId.TYPE_FLOAT:
                    m_visitor.EnterFloat(m_fieldCurrent, v);
                    break;
                case MetaTypeId.TYPE_DOUBLE:
                    m_visitor.EnterDouble(m_fieldCurrent, v);
                    break;
                case MetaTypeId.TYPE_STRING:
                    m_visitor.EnterString(m_fieldCurrent, v);
                    break;
                case MetaTypeId.TYPE_ENUM:
                    m_visitor.EnterEnum(m_fieldCurrent, v);
                    break;
                case MetaTypeId.TYPE_ARRAY_BOOL:
                    if (m_arrayBool == null)
                    {
                        m_arrayBool = new ArrayBuilder<bool>();
                    }
                    m_arrayBool.Add(v);
                    break;
                case MetaTypeId.TYPE_ARRAY_INT32:
                    if (m_arrayInt32 == null)
                    {
                        m_arrayInt32 = new ArrayBuilder<int>();
                    }
                    m_arrayInt32.Add(v);
                    break;
                case MetaTypeId.TYPE_ARRAY_UINT32:
                    if (m_arrayUInt32 == null)
                    {
                        m_arrayUInt32 = new ArrayBuilder<uint>();
                    }
                    m_arrayUInt32.Add(v);
                    break;
                case MetaTypeId.TYPE_ARRAY_INT64:
                    if (m_arrayInt64 == null)
                    {
                        m_arrayInt64 = new ArrayBuilder<long>();
                    }
                    m_arrayInt64.Add(v);
                    break;
                case MetaTypeId.TYPE_ARRAY_UINT64:
                    if (m_arrayUInt64 == null)
                    {
                        m_arrayUInt64 = new ArrayBuilder<ulong>();
                    }
                    m_arrayUInt64.Add(v);
                    break;
                case MetaTypeId.TYPE_ARRAY_FLOAT:
                    if (m_arrayFloat == null)
                    {
                        m_arrayFloat = new ArrayBuilder<float>();
                    }
                    m_arrayFloat.Add(v);
                    break;
                case MetaTypeId.TYPE_ARRAY_DOUBLE:
                    if (m_arrayDouble == null)
                    {
                        m_arrayDouble = new ArrayBuilder<double>();
                    }
                    m_arrayDouble.Add(v);
                    break;
                case MetaTypeId.TYPE_ARRAY_STRING:
                    if (m_arrayString == null)
                    {
                        m_arrayString = new List<string>();
                    }
                    m_arrayString.Add(v);
                    break;
                case MetaTypeId.TYPE_ARRAY_BYTES:
                    if (m_arrayBytes == null)
                    {
                        m_arrayBytes = new List<byte[]>();
                    }
                    m_arrayBytes.Add(v);
                    break;
                case MetaTypeId.TYPE_ARRAY_ENUM:
                    if ((m_arrayString == null || m_arrayString.Count == 0) || 
                        (m_arrayInt32 != null && m_arrayInt32.Length != 0))
                    {
                        if (m_arrayInt32 == null)
                        {
                            m_arrayInt32 = new ArrayBuilder<int>();
                        }
                        m_arrayInt32.Add(v);
                    }
                    else
                    {
                        string s = MetaDataGlobal.Instance.GetEnumAliasByValue(m_fieldCurrent, v);
                        m_arrayString.Add(s);
                    }
                    break;
                default:
//todo                    streamError << "number not expected";
                    break;
            }
        }

        static bool IsTrue(byte[] value, int offset, int size)
        {
            if (size == 4)
            {
                if ((value[offset + 0] == 't') &&
                    (value[offset + 1] == 'r') &&
                    (value[offset + 2] == 'u') &&
                    (value[offset + 3] == 'e'))
                {
                    return true;
                }
            }
            return false;
        }

        static bool IsNaN(byte[] value, int offset, int size)
        {
            if (size == 3)
            {
                if ((value[offset + 0] == 'N') &&
                    (value[offset + 1] == 'a') &&
                    (value[offset + 2] == 'N'))
                {
                    return true;
                }
            }
            return false;
        }

        static bool IsInfinity(byte[] value, int offset, int size)
        {
            if (size == 8)
            {
                if ((value[offset + 0] == 'I') &&
                    (value[offset + 1] == 'n') &&
                    (value[offset + 2] == 'f') &&
                    (value[offset + 3] == 'i') &&
                    (value[offset + 4] == 'n') &&
                    (value[offset + 5] == 'i') &&
                    (value[offset + 6] == 't') &&
                    (value[offset + 7] == 'y'))
                {
                    return true;
                }
            }
            return false;
        }

        static bool IsNInfinity(byte[] value, int offset, int size)
        {
            if (size == 9)
            {
                if ((value[offset + 0] == '-') && 
                    (value[offset + 1] == 'I') &&
                    (value[offset + 2] == 'n') &&
                    (value[offset + 3] == 'f') &&
                    (value[offset + 4] == 'i') &&
                    (value[offset + 5] == 'n') &&
                    (value[offset + 6] == 'i') &&
                    (value[offset + 7] == 't') &&
                    (value[offset + 8] == 'y'))
                {
                    return true;
                }
            }
            return false;
        }

        static float convertToFloat(byte[] value, int offset, int size)
        {
            float v = 0;
            if (IsNaN(value, offset, size))
            {
                v = System.Single.NaN;
            }
            else if (IsInfinity(value, offset, size))
            {
                v = System.Single.PositiveInfinity;
            }
            else if (IsNInfinity(value, offset, size))
            {
                v = System.Single.NegativeInfinity;
            }
            else
            {
                string s = Encoding.UTF8.GetString(value, offset, size);
                v = Convertion.Convert<float>(s);
            }
            return v;
        }

        static double convertToDouble(byte[] value, int offset, int size)
        {
            double v = 0;
            if (IsNaN(value, offset, size))
            {
                v = System.Double.NaN;
            }
            else if (IsInfinity(value, offset, size))
            {
                v = System.Double.PositiveInfinity;
            }
            else if (IsNInfinity(value, offset, size))
            {
                v = System.Double.NegativeInfinity;
            }
            else
            {
                string s = Encoding.UTF8.GetString(value, offset, size);
                v = Convertion.Convert<double>(s);
            }
            return v;
        }

        readonly byte[] m_buffer;
        readonly int m_offset;
        readonly int m_size;
        readonly IParserVisitor m_visitor;
        readonly JsonParser m_parser;

        IList<MetaField> m_stack = new List<MetaField>();
        MetaStruct? m_structCurrent = null;
        MetaField? m_fieldCurrent = null;

        ArrayBuilder<bool>? m_arrayBool;
        ArrayBuilder<int>? m_arrayInt32;
        ArrayBuilder<uint>? m_arrayUInt32;
        ArrayBuilder<long>? m_arrayInt64;
        ArrayBuilder<ulong>? m_arrayUInt64;
        ArrayBuilder<float>? m_arrayFloat;
        ArrayBuilder<double>? m_arrayDouble;
        IList<string>? m_arrayString;
        IList<byte[]>? m_arrayBytes;
    }
    
}
