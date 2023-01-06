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
                                    bool value = false;
                                    bool ok = ParseVarint(value);
                                    if (ok)
                                    {
                                        m_visitor.EnterBool(field, value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_INT32:
                                {
                                    int value = 0;
                                    bool ok = false;
                                    if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                                    {
                                        ok = ParseVarint(value);
                                    }
                                    else if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_ZIGZAG) != 0)
                                    {
                                        ok = ParseZigZag<int, uint>(value);
                                    }
                                    else
                                    {
                                        ok = ParseFixedValue<int>(value, WireType.WIRETYPE_FIXED32);
                                    }
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
                                    if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                                    {
                                        ok = ParseVarint(value);
                                    }
                                    else
                                    {
                                        ok = ParseFixedValue<uint>(value, WireType.WIRETYPE_FIXED32);
                                    }
                                    if (ok)
                                    {
                                        m_visitor.EnterUInt32(field, value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_INT64:
                                {
                                    long value = 0;
                                    bool ok = false;
                                    if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                                    {
                                        ok = ParseVarint(value);
                                    }
                                    else if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_ZIGZAG) != 0)
                                    {
                                        ok = ParseZigZag<long, ulong>(value);
                                    }
                                    else
                                    {
                                        ok = ParseFixedValue<long>(value, WireType.WIRETYPE_FIXED64);
                                    }
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
                                    if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                                    {
                                        ok = ParseVarint(value);
                                    }
                                    else
                                    {
                                        ok = ParseFixedValue<ulong>(value, WireType.WIRETYPE_FIXED64);
                                    }
                                    if (ok)
                                    {
                                        m_visitor.EnterUInt64(field, value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_FLOAT:
                                {
                                    float value = 0.0f;
                                    bool ok = ParseFixedValue<float>(value, WireType.WIRETYPE_FIXED32);
                                    if (ok)
                                    {
                                        m_visitor.EnterFloat(field, value);
                                    }
                                }
                                break;
                            case MetaTypeId.TYPE_DOUBLE:
                                {
                                    double value = 0.0;
                                    bool ok = ParseFixedValue<double>(value, WireType.WIRETYPE_FIXED64);
                                    if (ok)
                                    {
                                        m_visitor.EnterDouble(field, value);
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
                            //case MetaTypeId.TYPE_ENUM:
                            //    {
                            //        std::int32_t value = 0;
                            //        bool ok = parseVarint(value);
                            //        if (ok)
                            //        {
                            //            const MetaEnum* metaEnum = MetaDataGlobal::instance().getEnum(*field);
                            //            if (metaEnum)
                            //            {
                            //                if (!metaEnum->isId(value))
                            //                {
                            //                    value = 0;
                            //                }
                            //                m_visitor.enterEnum(*field, value);
                            //            }
                            //        }
                            //    }
                            //    break;
                            //case MetaTypeId.TYPE_ARRAY_BOOL:
                            //    {
                            //        std::vector<bool> array;
                            //        bool ok = parseArrayVarint(array);
                            //        if (ok)
                            //        {
                            //            m_visitor.enterArrayBool(*field, std::move(array));
                            //        }
                            //    }
                            //    break;
                            //case MetaTypeId.TYPE_ARRAY_INT32:
                            //    {
                            //        std::vector<std::int32_t> array;
                            //        bool ok = false;
                            //        if (field.Flags & MetaFieldFlags.METAFLAG_PROTO_VARINT)
                            //        {
                            //            ok = parseArrayVarint(array);
                            //        }
                            //        else if (field->flags & METAFLAG_PROTO_ZIGZAG)
                            //        {
                            //            ok = parseArrayVarint < std::int32_t, true > (array);
                            //        }
                            //        else
                            //        {
                            //            ok = parseArrayFixed<std::int32_t, WIRETYPE_FIXED32>(array);
                            //        }
                            //        if (ok)
                            //        {
                            //            m_visitor.enterArrayInt32(*field, std::move(array));
                            //        }
                            //    }
                            //    break;
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

        bool ParseFixedValue<T>(T value, WireType wireType)
        {
            return false;
        }

        bool ParseVarint<T>(T value)
        {
            return false;
        }

        bool ParseZigZag<T, D>(T value)
        {
            return false;
        }

        bool ParseArrayFixed<T>(T[] array, WireType wireType)
        {
            return false;
        }

        bool ParseArrayVarint<T>(T[] array, bool zigzag = false)
        {
            return false;
        }

        ulong ParseVarint()
        {
            return 0;
        }

        T ParseFixed<T>()
        {
            return default(T);
        }

        int ZigZag(uint value)
        {
            return 0;
        }
        long ZigZag(ulong value)
        {
            return 0;
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
