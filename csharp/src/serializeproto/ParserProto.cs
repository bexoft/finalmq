using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Reflection;

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

        bool ParseStruct(string typeName)
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
                m_visitor.Finished();
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
                    MetaField field = stru.GetFieldByIndex(index);
                    if (field != null)
                    {
                        switch (field.TypeId)
                        {
                            case MetaTypeId.TYPE_NONE:
                                break;
                            case MetaTypeId.TYPE_BOOL:
                                {
                                    ulong value = 0;
                                    bool ok = ParseVarintUInt64(out value);
                                    if (ok)
                                    {
                                        m_visitor.EnterBool(field, (value != 0));
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_INT32:
                                {
                                    ulong value = 0;
                                    bool ok = false;
                                    if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                                    {
                                        ok = ParseVarintUInt64(out value);
                                    }
                                    else if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_ZIGZAG) != 0)
                                    {
                                        ok = ParseZigZagUInt64(out value);
                                    }
                                    else
                                    {
                                        ok = ParseFixedValueUInt32(out value);
                                    }
                                    if (ok)
                                    {
                                        m_visitor.EnterInt32(field, (int)value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_UINT32:
                                {
                                    ulong value = 0;
                                    bool ok = false;
                                    if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                                    {
                                        ok = ParseVarintUInt64(out value);
                                    }
                                    else
                                    {
                                        ok = ParseFixedValueUInt32(out value);
                                    }
                                    if (ok)
                                    {
                                        m_visitor.EnterUInt32(field, (uint)value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_INT64:
                                {
                                    ulong value = 0;
                                    bool ok = false;
                                    if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                                    {
                                        ok = ParseVarintUInt64(out value);
                                    }
                                    else if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_ZIGZAG) != 0)
                                    {
                                        ok = ParseZigZagUInt64(out value);
                                    }
                                    else
                                    {
                                        ok = ParseFixedValueUInt64(out value);
                                    }
                                    if (ok)
                                    {
                                        m_visitor.EnterInt64(field, (long)value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_UINT64:
                                {
                                    ulong value = 0;
                                    bool ok = false;
                                    if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                                    {
                                        ok = ParseVarintUInt64(out value);
                                    }
                                    else
                                    {
                                        ok = ParseFixedValueUInt64(out value);
                                    }
                                    if (ok)
                                    {
                                        m_visitor.EnterUInt64(field, value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_FLOAT:
                                {
                                    ulong value = 0;
                                    bool ok = ParseFixedValueUInt32(out value);
                                    if (ok)
                                    {
                                        float v = BitConverter.UInt32BitsToSingle((uint)value);
                                        m_visitor.EnterFloat(field, v);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_DOUBLE:
                                {
                                    ulong value = 0;
                                    bool ok = ParseFixedValueUInt64(out value);
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
                                    ulong value = 0;
                                    bool ok = ParseVarintUInt64(out value);
                                    if (ok)
                                    {
                                        MetaEnum? metaEnum = MetaDataGlobal.Instance.GetEnum(field);
                                        if (metaEnum != null)
                                        {
                                            if (!metaEnum.IsId((int)value))
                                            {
                                                value = 0;
                                            }
                                            m_visitor.EnterEnum(field, (int)value);
                                        }
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_ARRAY_BOOL:
                                {
                                    bool[]? array = ParseArrayVarint<bool>();
                                    if (array != null)
                                    {
                                        m_visitor.EnterArrayBool(field, array);
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
                                        array = ParseArrayFixed<int>(WireType.WIRETYPE_FIXED32);
                                    }
                                    if (array != null)
                                    {
                                        m_visitor.EnterArrayInt32(field, array);
                                    }
                                }
                                break;
                            //case MetaTypeId.TYPE_ARRAY_UINT32:
                            //    {
                            //        std::vector<std::uint32_t> array;
                            //        bool ok = false;
                            //        if (field->flags & METAFLAG_PROTO_VARINT)
                            //        {
                            //            ok = parseArrayVarint(array);
                            //        }
                            //        else
                            //        {
                            //            ok = parseArrayFixed<std::uint32_t, WIRETYPE_FIXED32>(array);
                            //        }
                            //        if (ok)
                            //        {
                            //            m_visitor.enterArrayUInt32(*field, std::move(array));
                            //        }
                            //    }
                            //    break;
                            //case MetaTypeId.TYPE_ARRAY_INT64:
                            //    {
                            //        std::vector<std::int64_t> array;
                            //        bool ok = false;
                            //        if (field->flags & METAFLAG_PROTO_VARINT)
                            //        {
                            //            ok = parseArrayVarint(array);
                            //        }
                            //        else if (field->flags & METAFLAG_PROTO_ZIGZAG)
                            //        {
                            //            ok = parseArrayVarint < std::int64_t, true > (array);
                            //        }
                            //        else
                            //        {
                            //            ok = parseArrayFixed<std::int64_t, WIRETYPE_FIXED64>(array);
                            //        }
                            //        if (ok)
                            //        {
                            //            m_visitor.enterArrayInt64(*field, std::move(array));
                            //        }
                            //    }
                            //    break;
                            //case MetaTypeId.TYPE_ARRAY_UINT64:
                            //    {
                            //        std::vector<std::uint64_t> array;
                            //        bool ok = false;
                            //        if (field->flags & METAFLAG_PROTO_VARINT)
                            //        {
                            //            ok = parseArrayVarint(array);
                            //        }
                            //        else
                            //        {
                            //            ok = parseArrayFixed<std::uint64_t, WIRETYPE_FIXED64>(array);
                            //        }
                            //        if (ok)
                            //        {
                            //            m_visitor.enterArrayUInt64(*field, std::move(array));
                            //        }
                            //    }
                            //    break;
                            //case MetaTypeId.TYPE_ARRAY_FLOAT:
                            //    {
                            //        std::vector<float> array;
                            //        bool ok = parseArrayFixed<float, WIRETYPE_FIXED32>(array);
                            //        if (ok)
                            //        {
                            //            m_visitor.enterArrayFloat(*field, std::move(array));
                            //        }
                            //    }
                            //    break;
                            //case MetaTypeId.TYPE_ARRAY_DOUBLE:
                            //    {
                            //        std::vector<double> array;
                            //        bool ok = parseArrayFixed<double, WIRETYPE_FIXED64>(array);
                            //        if (ok)
                            //        {
                            //            m_visitor.enterArrayDouble(*field, std::move(array));
                            //        }
                            //    }
                            //    break;
                            //case MetaTypeId.TYPE_ARRAY_STRING:
                            //    {
                            //        std::vector < std::string> array;
                            //        bool ok = parseArrayString(array);
                            //        if (ok)
                            //        {
                            //            m_visitor.enterArrayString(*field, std::move(array));
                            //        }
                            //    }
                            //    break;
                            //case MetaTypeId.TYPE_ARRAY_BYTES:
                            //    {
                            //        std::vector<Bytes> array;
                            //        bool ok = parseArrayString(array);
                            //        if (ok)
                            //        {
                            //            m_visitor.enterArrayBytes(*field, std::move(array));
                            //        }
                            //    }
                            //    break;
                            //case MetaTypeId.TYPE_ARRAY_STRUCT:
                            //    parseArrayStruct(*field);
                            //    break;
                            //case MetaTypeId.TYPE_ARRAY_ENUM:
                            //    {
                            //        std::vector<std::int32_t> array;
                            //        bool ok = parseArrayVarint(array);
                            //        if (ok)
                            //        {
                            //            const MetaEnum* metaEnum = MetaDataGlobal::instance().getEnum(*field);
                            //            if (metaEnum)
                            //            {
                            //                for (size_t i = 0; i < array.size(); ++i)
                            //                {
                            //                    if (!metaEnum->isId(array[i]))
                            //                    {
                            //                        array[i] = 0;
                            //                    }
                            //                }
                            //                m_visitor.enterArrayEnum(*field, std::move(array));
                            //            }
                            //        }
                            //    }
                            //    break;
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


        void ParseArrayStruct(MetaField field, int offset, int size)
        {

        }

        bool parseArrayString<T>(IList<T> array)
        {
            return false;
        }

        bool ParseFixedValueUInt32(out ulong value)
        {
            WireType wireType = (WireType)(m_tag & 0x7);
            m_tag = 0;
            if (wireType == WireType.WIRETYPE_FIXED32)
            {
                value = ParseFixedUInt32();
                return true;
            }
            Skip(wireType);
            value = 0;
            return false;
        }

        bool ParseFixedValueUInt64(out ulong value)
        {
            WireType wireType = (WireType)(m_tag & 0x7);
            m_tag = 0;
            if (wireType == WireType.WIRETYPE_FIXED64)
            {
                value = ParseFixedUInt64();
                return true;
            }
            Skip(wireType);
            value = 0;
            return false;
        }

        ulong ParseFixedUInt32()
        {
            ulong value = 0;
            if (m_size >= sizeof(uint))
            {
                value |= m_buffer[m_offset];
                ++m_offset;
                value <<= 8;
                value |= m_buffer[m_offset];
                ++m_offset;
                value <<= 8;
                value |= m_buffer[m_offset];
                ++m_offset;
                value <<= 8;
                value |= m_buffer[m_offset];
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
                value <<= 8;
                value |= m_buffer[m_offset];
                ++m_offset;
                value <<= 8;
                value |= m_buffer[m_offset];
                ++m_offset;
                value <<= 8;
                value |= m_buffer[m_offset];
                ++m_offset;
                value <<= 8;
                value |= m_buffer[m_offset];
                ++m_offset;
                value <<= 8;
                value |= m_buffer[m_offset];
                ++m_offset;
                value <<= 8;
                value |= m_buffer[m_offset];
                ++m_offset;
                value <<= 8;
                value |= m_buffer[m_offset];
                ++m_offset;
                m_size -= sizeof(ulong);
                return value;
            }
            m_offset = -1;
            m_size = 0;
            return 0;
        }

        bool ParseVarintUInt64(out ulong value)
        {
            WireType wireType = (WireType)(m_tag & 0x7);
            m_tag = 0;
            if (wireType == WireType.WIRETYPE_VARINT)
            {
                value = ParseVarint();
                return true;
            }
            value = 0;
            Skip(wireType);
            return false;
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


        bool ParseZigZagUInt64(out ulong value)
        {
            bool ok = ParseVarintUInt64(out value);
            value = ZigZagUInt64(value);
            return ok;
        }

        T[]? ParseArrayFixed<T>(WireType wireType)
        {
            return null;
        }

        T[]? ParseArrayVarint<T>(bool zigzag = false)
        {
            return null;
        }

        ulong ZigZagUInt64(ulong value)
        {
            return ((value >> 1) ^ (~(value & 1) + 1));
        }

        void Skip(WireType wireType)
        {
        }

        byte[] m_buffer;
        int m_offset;
        int m_size;
        IParserVisitor m_visitor;

        uint m_tag = 0;
    };

}
