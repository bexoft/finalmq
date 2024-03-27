using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Reflection;
using System.Text;

namespace finalmq
{

    public class ParserProto
    {
        private static int INDEX2ID = 1;

        public ParserProto(IParserVisitor visitor, byte[] buffer, int offset = 0, int size = -1)
        {
            m_buffer = buffer;
            m_offset = offset;
            m_size = size;
            m_visitor = visitor;
            if (m_size < 0)
            {
                m_size = m_buffer.Length - offset;
            }
            Debug.Assert(m_offset >= 0);
            Debug.Assert(m_offset + m_size <= buffer.Length);
        }

        public bool ParseStruct(string typeName)
        {
            if (m_offset < 0 || m_size < 0)
            {
                // end of data
                return false;
            }

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct(typeName);
            if (stru == null)
            {
                m_visitor.NotifyError("", "typename not found");
                return false;
            }

            m_visitor.StartStruct(stru);
            bool res = ParseStructIntern(stru);
            m_visitor.Finished();
            return res;
        }
        private enum WireType
        {
            WIRETYPE_VARINT = 0,
            WIRETYPE_FIXED64 = 1,
            WIRETYPE_LENGTH_DELIMITED = 2,
            WIRETYPE_START_GROUP = 3,
            WIRETYPE_END_GROUP = 4,
            WIRETYPE_FIXED32 = 5,
        };

        bool ParseString(out int offset, out int size)
        {
            bool ok = true;
            offset = -1;
            size = 0;
            WireType wireType = (WireType)(m_tag & 0x7);
            m_tag = 0;
            if (wireType == WireType.WIRETYPE_LENGTH_DELIMITED)
            {
                int sizeBuffer = (int)ParseVarint();
                if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && (m_offset != -1))
                {
                    offset = m_offset;
                    size = sizeBuffer;
                    m_offset += sizeBuffer;
                    m_size -= sizeBuffer;
                }
                else
                {
                    m_offset = -1;
                    m_size = 0;
                }
            }
            else
            {
                Skip(wireType);
                ok = false;
            }
            return ok;
        }
        void ParseStructWire(MetaField field)
        {
            WireType wireType = (WireType)(m_tag & 0x7);

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct(field);
            if (stru == null)
            {
                m_visitor.NotifyError("", "typename not found");
                Skip(wireType);
                return;
            }

            m_tag = 0;
            if (wireType == WireType.WIRETYPE_LENGTH_DELIMITED)
            {
                int sizeBuffer = (int)ParseVarint();
                if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && (m_offset != -1))
                {
                    m_visitor.EnterStruct(field);
                    ParserProto parser = new ParserProto(m_visitor, m_buffer, m_offset, sizeBuffer);
                    bool res = parser.ParseStructIntern(stru);
                    m_visitor.ExitStruct(field);
                    if (res)
                    {
                        m_offset += sizeBuffer;
                        m_size -= sizeBuffer;
                    }
                    else
                    {
                        m_offset = -1;
                        m_size = 0;
                    }
                }
                else
                {
                    m_offset = -1;
                    m_size = 0;
                }
            }
            else
            {
                Skip(wireType);
            }
        }
        bool ParseStructIntern(MetaStruct stru)
        {
            if (m_offset == -1 || m_size < 0)
            {
                // end of data
                return false;
            }

            while (m_size > 0)
            {
                if (m_tag == 0)
                {
                    m_tag = (uint)ParseVarint();
                }
                if (m_offset != -1)
                {
                    int id = (int)(m_tag >> 3);
                    int index = id - INDEX2ID;
                    MetaField? field = stru.GetFieldByIndex(index);
                    if (field != null)
                    {
                        switch (field.TypeId)
                        {
                            case MetaTypeId.TYPE_NONE:
                                break;
                            case MetaTypeId.TYPE_BOOL:
                                {
                                    bool ok = false;
                                    ulong value = ParseValueUInt64(out ok, false);
                                    if (ok)
                                    {
                                        m_visitor.EnterBool(field, (value != 0));
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_INT8:
                                {
                                    int value = 0;
                                    bool zz = ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_ZIGZAG) != 0);
                                    bool ok = false;
                                    value = (int)ParseValueUInt32(out ok, zz);
                                    if (ok)
                                    {
                                        m_visitor.EnterInt8(field, (sbyte)value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_UINT8:
                                {
                                    uint value = 0;
                                    bool ok = false;
                                    value = ParseValueUInt32(out ok, false);
                                    if (ok)
                                    {
                                        m_visitor.EnterUInt8(field, (byte)value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_INT16:
                                {
                                    int value = 0;
                                    bool zz = ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_ZIGZAG) != 0);
                                    bool ok = false;
                                    value = (int)ParseValueUInt32(out ok, zz);
                                    if (ok)
                                    {
                                        m_visitor.EnterInt16(field, (short)value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_UINT16:
                                {
                                    uint value = 0;
                                    bool ok = false;
                                    value = ParseValueUInt32(out ok, false);
                                    if (ok)
                                    {
                                        m_visitor.EnterUInt16(field, (ushort)value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_INT32:
                                {
                                    int value = 0;
                                    bool zz = ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_ZIGZAG) != 0);
                                    bool ok = false;
                                    value = (int)ParseValueUInt32(out ok, zz);
                                    if (ok)
                                    {
                                        m_visitor.EnterInt32(field, value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_UINT32:
                                {
                                    uint value = 0;
                                    bool ok = false;
                                    value = ParseValueUInt32(out ok, false);
                                    if (ok)
                                    {
                                        m_visitor.EnterUInt32(field, value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_INT64:
                                {
                                    long value = 0;
                                    bool zz = ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_ZIGZAG) != 0);
                                    bool ok = false;
                                    value = (long)ParseValueUInt64(out ok, zz);
                                    if (ok)
                                    {
                                        m_visitor.EnterInt64(field, value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_UINT64:
                                {
                                    ulong value = 0;
                                    bool ok = false;
                                    value = (ulong)ParseValueUInt64(out ok, false);
                                    if (ok)
                                    {
                                        m_visitor.EnterUInt64(field, value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_FLOAT:
                                {
                                    bool ok = false;
                                    uint value = ParseFixedValueUInt32(out ok);
                                    if (ok)
                                    {
                                        float v = BitConverter.UInt32BitsToSingle(value);
                                        m_visitor.EnterFloat(field, v);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_DOUBLE:
                                {
                                    bool ok = false;
                                    ulong value = ParseFixedValueUInt64(out ok);
                                    if (ok)
                                    {
                                        double v = BitConverter.UInt64BitsToDouble(value);
                                        m_visitor.EnterDouble(field, v);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_STRING:
                                {
                                    int offset;
                                    int size = 0;
                                    bool ok = ParseString(out offset, out size);
                                    if (ok)
                                    {
                                        m_visitor.EnterString(field, m_buffer, offset, size);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_BYTES:
                                {
                                    int offset;
                                    int size = 0;
                                    bool ok = ParseString(out offset, out size);
                                    if (ok)
                                    {
                                        m_visitor.EnterBytes(field, m_buffer, offset, size);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_STRUCT:
                                ParseStructWire(field);
                                break;
                            case MetaTypeId.TYPE_ENUM:
                                {
                                    bool ok = false;
                                    int value = (int)ParseValueUInt32(out ok, false);
                                    if (ok)
                                    {
                                        m_visitor.EnterEnum(field, value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_ARRAY_BOOL:
                                {
                                    bool[]? array = ParseArrayVarintBool();
                                    if (array != null)
                                    {
                                        m_visitor.EnterArrayBool(field, array);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_ARRAY_INT8:
                                {
                                    int[]? array = null;
                                    if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                                    {
                                        array = ParseArrayVarint<int>();
                                    }
                                    else if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_ZIGZAG) != 0)
                                    {
                                        array = ParseArrayVarint<int>(true);
                                    }
                                    else
                                    {
                                        array = ParseArrayFixedUInt32<int>();
                                    }
                                    if (array != null)
                                    {
                                        sbyte[] arrayInt8 = new sbyte[array.Length];
                                        for (int i = 0; i < array.Length; ++i)
                                        {
                                            arrayInt8[i] = (sbyte)array[i]; 
                                        }
                                        m_visitor.EnterArrayInt8(field, arrayInt8);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_ARRAY_INT16:
                                {
                                    int[]? array = null;
                                    if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                                    {
                                        array = ParseArrayVarint<int>();
                                    }
                                    else if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_ZIGZAG) != 0)
                                    {
                                        array = ParseArrayVarint<int>(true);
                                    }
                                    else
                                    {
                                        array = ParseArrayFixedUInt32<int>();
                                    }
                                    if (array != null)
                                    {
                                        short[] arrayInt16 = new short[array.Length];
                                        for (int i = 0; i < array.Length; ++i)
                                        {
                                            arrayInt16[i] = (short)array[i];
                                        }
                                        m_visitor.EnterArrayInt16(field, arrayInt16);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_ARRAY_UINT16:
                                {
                                    uint[]? array = null;
                                    if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                                    {
                                        array = ParseArrayVarint<uint>();
                                    }
                                    else
                                    {
                                        array = ParseArrayFixedUInt32<uint>();
                                    }
                                    if (array != null)
                                    {
                                        ushort[] arrayUInt16 = new ushort[array.Length];
                                        for (int i = 0; i < array.Length; ++i)
                                        {
                                            arrayUInt16[i] = (ushort)array[i];
                                        }
                                        m_visitor.EnterArrayUInt16(field, arrayUInt16);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_ARRAY_INT32:
                                {
                                    int[]? array = null;
                                    if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                                    {
                                        array = ParseArrayVarint<int>();
                                    }
                                    else if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_ZIGZAG) != 0)
                                    {
                                        array = ParseArrayVarint<int>(true);
                                    }
                                    else
                                    {
                                        array = ParseArrayFixedUInt32<int>();
                                    }
                                    if (array != null)
                                    {
                                        m_visitor.EnterArrayInt32(field, array);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_ARRAY_UINT32:
                                {
                                    uint[]? array = null;
                                    if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                                    {
                                        array = ParseArrayVarint<uint>();
                                    }
                                    else
                                    {
                                        array = ParseArrayFixedUInt32<uint>();
                                    }
                                    if (array != null)
                                    {
                                        m_visitor.EnterArrayUInt32(field, array);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_ARRAY_INT64:
                                {
                                    long[]? array = null;
                                    if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                                    {
                                        array = ParseArrayVarint<long>();
                                    }
                                    else if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_ZIGZAG) != 0)
                                    {
                                        array = ParseArrayVarint<long>(true);
                                    }
                                    else
                                    {
                                        array = ParseArrayFixedUInt64<long>();
                                    }
                                    if (array != null)
                                    {
                                        m_visitor.EnterArrayInt64(field, array);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_ARRAY_UINT64:
                                {
                                    ulong[]? array = null;
                                    if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                                    {
                                        array = ParseArrayVarint<ulong>();
                                    }
                                    else
                                    {
                                        array = ParseArrayFixedUInt64<ulong>();
                                    }
                                    if (array != null)
                                    {
                                        m_visitor.EnterArrayUInt64(field, array);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_ARRAY_FLOAT:
                                {
                                    float[]? array = ParseArrayFixedFloat();
                                    if (array != null)
                                    {
                                        m_visitor.EnterArrayFloat(field, array);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_ARRAY_DOUBLE:
                                {
                                    double[]? array = ParseArrayFixedDouble();
                                    if (array != null)
                                    {
                                        m_visitor.EnterArrayDouble(field, array);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_ARRAY_STRING:
                                {
                                    IList<string>? array = ParseArrayString();
                                    if (array != null)
                                    {
                                        m_visitor.EnterArrayString(field, array);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_ARRAY_BYTES:
                                {
                                    IList<byte[]>? array = ParseArrayBytes();
                                    if (array != null)
                                    {
                                        m_visitor.EnterArrayBytes(field, array);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_ARRAY_STRUCT:
                                ParseArrayStruct(field);
                                break;
                            case MetaTypeId.TYPE_ARRAY_ENUM:
                                {
                                    int[]? array = ParseArrayVarint<int>();
                                    if (array != null)
                                    {
                                        m_visitor.EnterArrayEnum(field, array);
                                    }
                                }
                                break;
                            case MetaTypeId.OFFSET_ARRAY_FLAG:
                                Debug.Assert(false);
                                break;
                            default:
                                Debug.Assert(false);
                                break;
                        }
                    }
                    else
                    {
                        WireType wireType = (WireType)(m_tag & 0x7);
                        Skip(wireType);
                        m_tag = 0;
                    }
                }
            }

            return (m_offset != -1);
        }


        void ParseArrayStruct(MetaField field)
        {
            if (m_offset == -1)
            {
                return;
            }

            WireType wireType = (WireType)(m_tag & 0x7);

            MetaStruct? stru = MetaDataGlobal.Instance.GetStruct(field);
            if (stru == null)
            {
                m_visitor.NotifyError("", "typename not found");
                Skip(wireType);
                return;
            }

            uint tag = m_tag;
            m_tag = 0;
            if (wireType == WireType.WIRETYPE_LENGTH_DELIMITED)
            {

                MetaField? fieldWithoutArray = field.FieldWithoutArray;
                Debug.Assert(fieldWithoutArray != null);
                m_visitor.EnterArrayStruct(field);
                do
                {
                    int sizeBuffer = (int)ParseVarint();
                    if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && (m_offset != -1))
                    {
                        m_visitor.EnterStruct(fieldWithoutArray);
                        ParserProto parser = new ParserProto(m_visitor, m_buffer, m_offset, sizeBuffer);
                        bool res = parser.ParseStructIntern(stru);
                        m_visitor.ExitStruct(fieldWithoutArray);
                        if (res)
                        {
                            m_offset += sizeBuffer;
                            m_size -= sizeBuffer;
                        }
                        else
                        {
                            m_offset = -1;
                            m_size = 0;
                            break;
                        }

                        if (m_size > 0)
                        {
                            m_tag = (uint)ParseVarint();
                        }
                        else
                        {
                            m_tag = 0;
                            break;
                        }
                    }
                    else
                    {
                        m_offset = -1;
                        m_size = 0;
                        break;
                    }
                } while ((m_tag == tag) && (m_offset != -1));
                m_visitor.ExitArrayStruct(field);
            }
            else
            {
                Skip(wireType);
            }
        }

        IList<string>? ParseArrayString()
        {
            if (m_offset == -1)
            {
                return null;
            }
            IList<string>? arr = null;
            WireType wireType = (WireType)(m_tag & 0x7);
            uint tag = m_tag;
            m_tag = 0;
            if (wireType == WireType.WIRETYPE_LENGTH_DELIMITED)
            {
                arr = new List<string>();
                do
                {
                    int sizeBuffer = (int)ParseVarint();
                    if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && (m_offset != -1))
                    {
                        string? v = null;
                        try
                        {
                            v = Encoding.UTF8.GetString(m_buffer, m_offset, sizeBuffer);
                        }
                        catch (Exception)
                        {
                            v = Encoding.ASCII.GetString(m_buffer, m_offset, sizeBuffer);
                        }
                        arr.Add(v);
                        m_offset += sizeBuffer;
                        m_size -= sizeBuffer;
                        if (m_size > 0)
                        {
                            m_tag = (uint)ParseVarint();
                        }
                        else
                        {
                            m_tag = 0;
                            break;
                        }
                    }
                    else
                    {
                        m_offset = -1;
                        m_size = 0;
                        break;
                    }
                } while ((m_tag == tag) && (m_offset != -1));
            }
            else
            {
                Skip(wireType);
            }

            return arr;
        }

        IList<byte[]>? ParseArrayBytes()
        {
            if (m_offset == -1)
            {
                return null;
            }
            IList<byte[]>? arr = null;
            WireType wireType = (WireType)(m_tag & 0x7);
            uint tag = m_tag;
            m_tag = 0;
            if (wireType == WireType.WIRETYPE_LENGTH_DELIMITED)
            {
                arr = new List<byte[]>();
                do
                {
                    int sizeBuffer = (int)ParseVarint();
                    if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && (m_offset != -1))
                    {
                        byte[] v = new byte[sizeBuffer];
                        System.Array.Copy(m_buffer, m_offset, v, 0, sizeBuffer);
                        arr.Add(v);
                        m_offset += sizeBuffer;
                        m_size -= sizeBuffer;
                        if (m_size > 0)
                        {
                            m_tag = (uint)ParseVarint();
                        }
                        else
                        {
                            m_tag = 0;
                            break;
                        }
                    }
                    else
                    {
                        m_offset = -1;
                        m_size = 0;
                        break;
                    }
                } while ((m_tag == tag) && (m_offset != -1));
            }
            else
            {
                Skip(wireType);
            }

            return arr;
        }

        uint ParseFixedValueUInt32(out bool ok)
        {
            WireType wireType = (WireType)(m_tag & 0x7);
            m_tag = 0;
            if (wireType == WireType.WIRETYPE_FIXED32)
            {
                ok = true;
                return ParseFixedUInt32();
            }
            Skip(wireType);
            ok = false;
            return 0;
        }

        ulong ParseFixedValueUInt64(out bool ok)
        {
            WireType wireType = (WireType)(m_tag & 0x7);
            m_tag = 0;
            if (wireType == WireType.WIRETYPE_FIXED64)
            {
                ok = true;
                return ParseFixedUInt64();
            }
            Skip(wireType);
            ok = false;
            return 0;
        }

        uint ParseFixedUInt32()
        {
            uint value = 0;
            if (m_size >= sizeof(uint))
            {
                value |= m_buffer[m_offset];
                ++m_offset;
                value |= (uint)m_buffer[m_offset] << 8;
                ++m_offset;
                value |= (uint)m_buffer[m_offset] << 16;
                ++m_offset;
                value |= (uint)m_buffer[m_offset] << 24;
                ++m_offset;
                m_size -= sizeof(uint);
                return value;
            }
            m_offset = -1;
            m_size = 0;
            return 0;
        }

        ulong ParseFixedUInt64()
        {
            ulong value = 0;
            if (m_size >= sizeof(ulong))
            {
                value |= m_buffer[m_offset];
                ++m_offset;
                value |= (ulong)m_buffer[m_offset] << 8;
                ++m_offset;
                value |= (ulong)m_buffer[m_offset] << 16;
                ++m_offset;
                value |= (ulong)m_buffer[m_offset] << 24;
                ++m_offset;
                value |= (ulong)m_buffer[m_offset] << 32;
                ++m_offset;
                value |= (ulong)m_buffer[m_offset] << 40;
                ++m_offset;
                value |= (ulong)m_buffer[m_offset] << 48;
                ++m_offset;
                value |= (ulong)m_buffer[m_offset] << 56;
                ++m_offset;
                m_size -= sizeof(ulong);
                return value;
            }
            m_offset = -1;
            m_size = 0;
            return 0;
        }
        uint ParseValueUInt32(out bool ok, bool zz)
        {
            WireType wireType = (WireType)(m_tag & 0x7);
            m_tag = 0;
            if (wireType == WireType.WIRETYPE_VARINT)
            {
                ok = true;
                if (!zz)
                {
                    return (uint)ParseVarint();
                }
                else
                {
                    ulong v = ParseVarint();
                    return (uint)ZigZagUInt64(v);
                }
            }
            else if (wireType == WireType.WIRETYPE_FIXED32)
            {
                ok = true;
                return ParseFixedUInt32();
            }
            else if (wireType == WireType.WIRETYPE_FIXED64)
            {
                ok = true;
                return (uint)ParseFixedUInt64();
            }
            Skip(wireType);
            ok = false;
            return 0;
        }
        ulong ParseValueUInt64(out bool ok, bool zz)
        {
            WireType wireType = (WireType)(m_tag & 0x7);
            m_tag = 0;
            if (wireType == WireType.WIRETYPE_VARINT)
            {
                ok = true;
                if (!zz)
                {
                    return ParseVarint();
                }
                else
                {
                    ulong v = ParseVarint();
                    return ZigZagUInt64(v);
                }
            }
            else if (wireType == WireType.WIRETYPE_FIXED32)
            {
                ok = true;
                return ParseFixedUInt32();
            }
            else if (wireType == WireType.WIRETYPE_FIXED64)
            {
                ok = true;
                return ParseFixedUInt64();
            }
            Skip(wireType);
            ok = false;
            return 0;
        }
        ulong ParseVarint()
        {
            ulong res = 0;
            if (m_size <= 0)
            {
                m_offset = -1;
                m_size = 0;
                return 0;
            }
            ulong c = m_buffer[m_offset];
            res = c;
            ++m_offset;
            --m_size;
            if (c < 128)
            {
                return res;
            }
            for (int shift = 7; shift < 70; shift += 7)
            {
                if (m_size <= 0)
                {
                    break;
                }
                c = m_buffer[m_offset];
                res += (c - 1) << shift;
                ++m_offset;
                --m_size;
                if (c < 128)
                {
                    return res;
                }
            }
            m_offset = -1;
            m_size = 0;
            return 0;
        }

        ulong ZigZagUInt64(ulong value)
        {
            return ((value >> 1) ^ (~(value & 1) + 1));
        }

        T[]? ParseArrayFixedUInt32<T>()
        {
            if (m_offset == -1)
            {
                return null;
            }
            WireType wireType = (WireType)(m_tag & 0x7);
            uint tag = m_tag;
            m_tag = 0;
            T[]? arr = null;
            switch (wireType)
            {
                case WireType.WIRETYPE_FIXED32:
                    ArrayBuilder<T> arrayBuilder = new ArrayBuilder<T>();
                    do
                    {
                        uint value = ParseFixedUInt32();
                        if (m_offset != -1)
                        {
                            arrayBuilder.Add((T)(dynamic)value);
                            if (m_size > 0)
                            {
                                m_tag = (uint)ParseVarint();
                            }
                            else
                            {
                                m_tag = 0;
                                break;
                            }
                        }
                        else
                        {
                            m_offset = -1;
                            m_size = 0;
                        }
                    } while ((m_tag == tag) && (m_offset != -1));
                    if (m_offset != -1)
                    {
                        arr = arrayBuilder.ToArray();
                    }
                    break;

                case WireType.WIRETYPE_LENGTH_DELIMITED:
                    int sizeBuffer = (int)ParseVarint();
                    if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && (m_offset != -1))
                    {
                        int sizeElements = sizeBuffer / sizeof(uint);
                        T[] array = new T[sizeElements];

                        if (BitConverter.IsLittleEndian)
                        {
                            Buffer.BlockCopy(m_buffer, m_offset, array, 0, sizeElements * sizeof(uint));
                            m_offset += sizeBuffer;
                            m_size -= sizeBuffer;
                        }
                        else
                        {
                            for (int i = 0; i < sizeElements; i++)
                            {
                                array[i] = (T)(dynamic)ParseFixedUInt32();
                            }
                        }
                        arr = array;
                    }
                    else
                    {
                        m_offset = -1;
                        m_size = 0;
                    }
                    break;
                default:
                    Skip(wireType);
                    break;
            }

            return arr;
        }

        T[]? ParseArrayFixedUInt64<T>()
        {
            if (m_offset == -1)
            {
                return null;
            }
            WireType wireType = (WireType)(m_tag & 0x7);
            uint tag = m_tag;
            m_tag = 0;
            T[]? arr = null;
            switch (wireType)
            {
                case WireType.WIRETYPE_FIXED64:
                    ArrayBuilder<T> arrayBuilder = new ArrayBuilder<T>();
                    do
                    {
                        ulong value = ParseFixedUInt64();
                        if (m_offset != -1)
                        {
                            arrayBuilder.Add((T)(dynamic)value);
                            if (m_size > 0)
                            {
                                m_tag = (uint)ParseVarint();
                            }
                            else
                            {
                                m_tag = 0;
                                break;
                            }
                        }
                        else
                        {
                            m_offset = -1;
                            m_size = 0;
                        }
                    } while ((m_tag == tag) && (m_offset != -1));
                    if (m_offset != -1)
                    {
                        arr = arrayBuilder.ToArray();
                    }
                    break;

                case WireType.WIRETYPE_LENGTH_DELIMITED:
                    int sizeBuffer = (int)ParseVarint();
                    if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && (m_offset != -1))
                    {
                        int sizeElements = sizeBuffer / sizeof(ulong);
                        T[] array = new T[sizeElements];

                        if (BitConverter.IsLittleEndian)
                        {
                            Buffer.BlockCopy(m_buffer, m_offset, array, 0, sizeElements * sizeof(ulong));
                            m_offset += sizeBuffer;
                            m_size -= sizeBuffer;
                        }
                        else
                        {
                            for (int i = 0; i < sizeElements; i++)
                            {
                                array[i] = (T)(dynamic)ParseFixedUInt64();
                            }
                        }
                        arr = array;
                    }
                    else
                    {
                        m_offset = -1;
                        m_size = 0;
                    }
                    break;
                default:
                    Skip(wireType);
                    break;
            }

            return arr;
        }

        float[]? ParseArrayFixedFloat()
        {
            if (m_offset == -1)
            {
                return null;
            }
            WireType wireType = (WireType)(m_tag & 0x7);
            uint tag = m_tag;
            m_tag = 0;
            float[]? arr = null;
            switch (wireType)
            {
                case WireType.WIRETYPE_FIXED32:
                    ArrayBuilder<float> arrayBuilder = new ArrayBuilder<float>();
                    do
                    {
                        uint value = ParseFixedUInt32();
                        if (m_offset != -1)
                        {
                            float v = BitConverter.UInt32BitsToSingle(value);
                            arrayBuilder.Add(v);
                            if (m_size > 0)
                            {
                                m_tag = (uint)ParseVarint();
                            }
                            else
                            {
                                m_tag = 0;
                                break;
                            }
                        }
                        else
                        {
                            m_offset = -1;
                            m_size = 0;
                        }
                    } while ((m_tag == tag) && (m_offset != -1));
                    if (m_offset != -1)
                    {
                        arr = arrayBuilder.ToArray();
                    }
                    break;

                case WireType.WIRETYPE_LENGTH_DELIMITED:
                    int sizeBuffer = (int)ParseVarint();
                    if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && (m_offset != -1))
                    {
                        int sizeElements = sizeBuffer / sizeof(float);
                        float[] array = new float[sizeElements];

                        if (BitConverter.IsLittleEndian)
                        {
                            Buffer.BlockCopy(m_buffer, m_offset, array, 0, sizeElements * sizeof(float));
                            m_offset += sizeBuffer;
                            m_size -= sizeBuffer;
                        }
                        else
                        {
                            for (int i = 0; i < sizeElements; i++)
                            {
                                float v = BitConverter.UInt32BitsToSingle(ParseFixedUInt32());
                                array[i] = v;
                            }
                        }
                        arr = array;
                    }
                    else
                    {
                        m_offset = -1;
                        m_size = 0;
                    }
                    break;
                default:
                    Skip(wireType);
                    break;
            }

            return arr;
        }

        double[]? ParseArrayFixedDouble()
        {
            if (m_offset == -1)
            {
                return null;
            }
            WireType wireType = (WireType)(m_tag & 0x7);
            uint tag = m_tag;
            m_tag = 0;
            double[]? arr = null;
            switch (wireType)
            {
                case WireType.WIRETYPE_FIXED64:
                    ArrayBuilder<double> arrayBuilder = new ArrayBuilder<double>();
                    do
                    {
                        ulong value = ParseFixedUInt64();
                        if (m_offset != -1)
                        {
                            double v = BitConverter.UInt64BitsToDouble(value);
                            arrayBuilder.Add(v);
                            if (m_size > 0)
                            {
                                m_tag = (uint)ParseVarint();
                            }
                            else
                            {
                                m_tag = 0;
                                break;
                            }
                        }
                        else
                        {
                            m_offset = -1;
                            m_size = 0;
                        }
                    } while ((m_tag == tag) && (m_offset != -1));
                    if (m_offset != -1)
                    {
                        arr = arrayBuilder.ToArray();
                    }
                    break;

                case WireType.WIRETYPE_LENGTH_DELIMITED:
                    int sizeBuffer = (int)ParseVarint();
                    if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && (m_offset != -1))
                    {
                        int sizeElements = sizeBuffer / sizeof(double);
                        double[] array = new double[sizeElements];

                        if (BitConverter.IsLittleEndian)
                        {
                            Buffer.BlockCopy(m_buffer, m_offset, array, 0, sizeElements * sizeof(double));
                            m_offset += sizeBuffer;
                            m_size -= sizeBuffer;
                        }
                        else
                        {
                            for (int i = 0; i < sizeElements; i++)
                            {
                                double v = BitConverter.UInt64BitsToDouble(ParseFixedUInt64());
                                array[i] = v;
                            }
                        }
                        arr = array;
                    }
                    else
                    {
                        m_offset = -1;
                        m_size = 0;
                    }
                    break;
                default:
                    Skip(wireType);
                    break;
            }

            return arr;
        }

        T[]? ParseArrayVarint<T>(bool zigzag = false)
        {
            WireType wireType = (WireType)(m_tag & 0x7);
            uint tag = m_tag;
            m_tag = 0;
            ArrayBuilder<T>? array = null;
            switch (wireType)
            {
                case WireType.WIRETYPE_VARINT:
                    array = new ArrayBuilder<T>();
                    do
                    {
                        ulong value = ParseVarint();
                        if (m_offset != -1)
                        {
                            ulong v = (zigzag) ? ZigZagUInt64(value) : value;
                            array.Add((T)(dynamic)v);
                            if (m_size > 0)
                            {
                                m_tag = (uint)ParseVarint();
                            }
                            else
                            {
                                m_tag = 0;
                                break;
                            }
                        }
                        else
                        {
                            m_offset = -1;
                            m_size = 0;
                            array = null;
                            break;
                        }
                    } while ((m_tag == tag) && (m_offset != -1));
                    break;
                case WireType.WIRETYPE_LENGTH_DELIMITED:
                    {
                        array = new ArrayBuilder<T>();
                        int sizeBuffer = (int)ParseVarint();
                        if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && (m_offset != -1))
                        {
                            int sizeEnd = m_size - sizeBuffer;
                            sizeEnd = Math.Max(sizeEnd, 0);
                            while (m_size > sizeEnd)
                            {
                                ulong value = ParseVarint();
                                ulong v = (zigzag) ? ZigZagUInt64(value) : value;
                                array.Add((T)(dynamic)v);
                            }
                        }
                        else
                        {
                            m_offset = -1;
                            m_size = 0;
                            array = null;
                        }
                    }
                    break;
                default:
                    Skip(wireType);
                    break;
            }

            return array?.ToArray();
        }

        bool[]? ParseArrayVarintBool()
        {
            WireType wireType = (WireType)(m_tag & 0x7);
            uint tag = m_tag;
            m_tag = 0;
            ArrayBuilder<bool>? array = null;
            switch (wireType)
            {
                case WireType.WIRETYPE_VARINT:
                    array = new ArrayBuilder<bool>();
                    do
                    {
                        ulong value = ParseVarint();
                        if (m_offset != -1)
                        {
                            array.Add(value != 0);
                            if (m_size > 0)
                            {
                                m_tag = (uint)ParseVarint();
                            }
                            else
                            {
                                m_tag = 0;
                                break;
                            }
                        }
                        else
                        {
                            m_offset = -1;
                            m_size = 0;
                            array = null;
                            break;
                        }
                    } while ((m_tag == tag) && (m_offset != -1));
                    break;
                case WireType.WIRETYPE_LENGTH_DELIMITED:
                    {
                        array = new ArrayBuilder<bool>();
                        int sizeBuffer = (int)ParseVarint();
                        if ((sizeBuffer >= 0 && sizeBuffer <= m_size) && (m_offset != -1))
                        {
                            int sizeEnd = m_size - sizeBuffer;
                            sizeEnd = Math.Max(sizeEnd, 0);
                            while (m_size > sizeEnd)
                            {
                                ulong value = ParseVarint();
                                array.Add(value != 0);
                            }
                        }
                        else
                        {
                            m_offset = -1;
                            m_size = 0;
                            array = null;
                        }
                    }
                    break;
                default:
                    Skip(wireType);
                    break;
            }

            return array?.ToArray();
        }

        void Skip(WireType wireType)
        {
            switch (wireType)
            {
                case WireType.WIRETYPE_VARINT:
                    ParseVarint();
                    break;
                case WireType.WIRETYPE_FIXED64:
                    if (m_offset != -1)
                    {
                        m_offset += sizeof(ulong);
                        m_size -= sizeof(ulong);
                    }
                    break;
                case WireType.WIRETYPE_LENGTH_DELIMITED:
                    {
                        int len = (int)ParseVarint();
                        if (m_offset != -1)
                        {
                            m_offset += len;
                            m_size -= len;
                        }
                    }
                    break;
                case WireType.WIRETYPE_FIXED32:
                    if (m_offset != -1)
                    {
                        m_offset += sizeof(uint);
                        m_size -= sizeof(uint);
                    }
                    break;
                default:
                    m_offset = -1;
                    m_size = 0;
                    break;
            }
            if (m_size < 0)
            {
                m_offset = -1;
                m_size = 0;
            }
        }

        byte[] m_buffer;
        int m_offset;
        int m_size;
        IParserVisitor m_visitor;

        uint m_tag = 0;
    };

}
