using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace finalmq
{
    
    public class SerializerProto : ParserConverter
    {
        enum WireType
        {
            WIRETYPE_VARINT = 0,
            WIRETYPE_FIXED64 = 1,
            WIRETYPE_LENGTH_DELIMITED = 2,
            WIRETYPE_START_GROUP = 3,
            WIRETYPE_END_GROUP = 4,
            WIRETYPE_FIXED32 = 5,
        };

        static readonly int INDEX2ID = 1;
        static readonly int MAX_VARINT_SIZE = 10;
        static readonly int STRUCT_SIZE_COPY = 128;
        static readonly int RESERVE_STRUCT_SIZE = 8;
        static readonly uint DUMMY_ID = 2047;
        static readonly uint tagDummy = (DUMMY_ID << 3) | (uint)WireType.WIRETYPE_VARINT;

        public SerializerProto(IZeroCopyBuffer buffer, int maxBlockSize = 512)
        {
            m_internal = new Internal(buffer, maxBlockSize);
            SetVisitor(m_internal);
        }

        class Internal : IParserVisitor
        {
            public Internal(IZeroCopyBuffer buffer, int maxBlockSize)
            {
                m_zeroCopybuffer = buffer;
                m_maxBlockSize = maxBlockSize;
            }


            // IParserVisitor

            public void NotifyError(string str, string message) 
            { 
            }
            public void StartStruct(MetaStruct stru)
            {
            }
            public void Finished()
            {
                ResizeBuffer();
            }

            public void EnterStruct(MetaField field)
            {
                int id = field.Index + INDEX2ID;
                int offsetStructStart = SerializeStruct(id);
                bool arrayEntry = (m_stackStruct.Count != 0) ? m_stackStruct.Last().arrayParent : m_arrayParent;
                m_stackStruct.Add(new StructData(m_buffer, offsetStructStart, m_offset, m_offset + RESERVE_STRUCT_SIZE, arrayEntry));
                m_offset += RESERVE_STRUCT_SIZE;
            }
            public void ExitStruct(MetaField field)
            {
                Debug.Assert(m_stackStruct.Count != 0);
                StructData structData = m_stackStruct.Last();
                Debug.Assert(structData.offset != -1);
                int structSize = (m_offset - structData.offset) + structData.size;
                Debug.Assert(structSize >= 0);
                if (structSize == 0 && !structData.allocateNextDataBuffer && !structData.arrayEntry)
                {
                    m_buffer = structData.bufferStructStart;
                    m_offset = structData.offsetStructStart;
                }
                else if (structSize <= STRUCT_SIZE_COPY && !structData.allocateNextDataBuffer)
                {
                    m_offset = structData.offsetStructSize;
                    SerializeVarint((ulong)structSize);
                    Buffer.BlockCopy(m_buffer, structData.offsetStructSize + RESERVE_STRUCT_SIZE, m_buffer, m_offset, structSize);
                    m_offset += structSize;
                }
                else
                {
                    int remainingSize = CalculateStructSize(ref structSize);

                    byte[] bufferCurrent = m_buffer;
                    int offsetCurrent = m_offset;
                    m_buffer = structData.bufferStructStart;
                    m_offset = structData.offsetStructSize;

                    SerializeVarint((ulong)structSize);
                    Debug.Assert(remainingSize <= 7 && remainingSize >= 3);
                    int remainingSizeFromBuffer = structData.offsetStructSize + RESERVE_STRUCT_SIZE - m_offset;
                    if (remainingSizeFromBuffer != remainingSize)
                    {
//todo                        streamFatal << "Struct calculations are wrong";
                    }
                    Debug.Assert(remainingSizeFromBuffer == remainingSize);
                    SerializeVarint(tagDummy);

                    remainingSize -= 2;
                    FillRemainingStruct(remainingSize);
                    Debug.Assert(m_offset == structData.offsetStructSize + RESERVE_STRUCT_SIZE);
                    m_offset = offsetCurrent;
                    m_buffer = bufferCurrent;
                }
                m_stackStruct.RemoveAt(m_stackStruct.Count - 1);
            }

            public void EnterStructNull(MetaField field)
            {
            }

            public void EnterArrayStruct(MetaField field)
            {
                if (m_stackStruct.Count != 0)
                {
                    m_stackStruct.Last().arrayParent = true;
                }
                else
                {
                    m_arrayParent = true;
                }
            }
            public void ExitArrayStruct(MetaField field)
            {
                if (m_stackStruct.Count != 0)
                {
                    m_stackStruct.Last().arrayParent = false;
                }
                else
                {
                    m_arrayParent = false;
                }
            }

            public void EnterBool(MetaField field, bool value)
            {
                int id = field.Index + INDEX2ID;
                SerializeVarintValue(id, value ? 1UL : 0UL);
            }
            public void EnterInt32(MetaField field, int value)
            {
                int id = field.Index + INDEX2ID;
                if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                {
                    SerializeVarintValue(id, (ulong)value);
                }
                else if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_ZIGZAG) != 0)
                {
                    SerializeZigZagValue(id, value);
                }
                else
                {
                    SerializeFixedValueUInt32(id, (uint)value);
                }
            }
            public void EnterUInt32(MetaField field, uint value)
            {
                int id = field.Index + INDEX2ID;
                if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                {
                    SerializeVarintValue(id, value);
                }
                else
                {
                    SerializeFixedValueUInt32(id, value);
                }
            }
            public void EnterInt64(MetaField field, long value)
            {
                int id = field.Index + INDEX2ID;
                if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                {
                    SerializeVarintValue(id, (ulong)value);
                }
                else if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_ZIGZAG) != 0)
                {
                    SerializeZigZagValue(id, value);
                }
                else
                {
                    SerializeFixedValueUInt64(id, (ulong)value);
                }
            }
            public void EnterUInt64(MetaField field, ulong value)
            {
                int id = field.Index + INDEX2ID;
                if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                {
                    SerializeVarintValue(id, value);
                }
                else
                {
                    SerializeFixedValueUInt64(id, value);
                }
            }
            public void EnterFloat(MetaField field, float value)
            {
                int id = field.Index + INDEX2ID;
                uint v = BitConverter.SingleToUInt32Bits(value);
                SerializeFixedValueUInt32(id, v);
            }
            public void EnterDouble(MetaField field, double value)
            {
                int id = field.Index + INDEX2ID;
                ulong v = BitConverter.DoubleToUInt64Bits(value);
                SerializeFixedValueUInt64(id, v);
            }
            public void EnterString(MetaField field, string value)
            {
                int id = field.Index + INDEX2ID;
                SerializeString(id, value, true);
            }
            public void EnterString(MetaField field, byte[] buffer, int offset, int size)
            {
                int id = field.Index + INDEX2ID;
                SerializeString(id, buffer, offset, size, true);
            }
            public void EnterBytes(MetaField field, byte[] value, int offset, int size)
            {
                int id = field.Index + INDEX2ID;
                SerializeString(id, value, offset, size, true);
            }
            public void EnterEnum(MetaField field, int value)
            {
                int id = field.Index + INDEX2ID;
                SerializeVarintValue(id, (ulong)value);
            }
            public void EnterEnum(MetaField field, string value)
            {
                int enumValue = MetaDataGlobal.Instance.GetEnumValueByName(field, value);
                EnterEnum(field, enumValue);
            }

            public void EnterArrayBool(MetaField field, bool[] value)
            {
                int id = field.Index + INDEX2ID;
                SerializeArrayBool(id, value);
            }
            public void EnterArrayInt32(MetaField field, int[] value)
            {
                int id = field.Index + INDEX2ID;
                if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                {
                    SerializeArrayVarint(id, value);
                }
                else if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_ZIGZAG) != 0)
                {
                    SerializeArrayZigZag(id, value);
                }
                else
                {
                    SerializeArrayFixedUInt32(id, value);
                }
            }
            public void EnterArrayUInt32(MetaField field, uint[] value)
            {
                int id = field.Index + INDEX2ID;
                if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                {
                    SerializeArrayVarint(id, value);
                }
                else
                {
                    SerializeArrayFixedUInt32(id, value);
                }
            }
            public void EnterArrayInt64(MetaField field, long[] value)
            {
                int id = field.Index + INDEX2ID;
                if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                {
                    SerializeArrayVarint(id, value);
                }
                else if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_ZIGZAG) != 0)
                {
                    SerializeArrayZigZag(id, value);
                }
                else
                {
                    SerializeArrayFixedUInt64(id, value);
                }
            }
            public void EnterArrayUInt64(MetaField field, ulong[] value)
            {
                int id = field.Index + INDEX2ID;
                if ((field.Flags & (int)MetaFieldFlags.METAFLAG_PROTO_VARINT) != 0)
                {
                    SerializeArrayVarint(id, value);
                }
                else
                {
                    SerializeArrayFixedUInt64(id, value);
                }
            }
            public void EnterArrayFloat(MetaField field, float[] value)
            {
                int id = field.Index + INDEX2ID;
                SerializeArrayFixedFloat(id, value);
            }
            public void EnterArrayDouble(MetaField field, double[] value)
            {
                int id = field.Index + INDEX2ID;
                SerializeArrayFixedDouble(id, value);
            }
            public void EnterArrayString(MetaField field, IList<string> value)
            {
                int id = field.Index + INDEX2ID;
                SerializeArrayString(id, value);
            }
            public void EnterArrayBytes(MetaField field, IList<byte[]> value)
            {
                int id = field.Index + INDEX2ID;
                SerializeArrayBytes(id, value);
            }
            public void EnterArrayEnum(MetaField field, int[] value)
            {
                int id = field.Index + INDEX2ID;
                SerializeArrayVarint(id, value);
            }
            public void EnterArrayEnum(MetaField field, IList<string> value)
            {
                MetaEnum? metaEnum = MetaDataGlobal.Instance.GetEnum(field);
                if (metaEnum != null)
                {
                    if (value.Count <= 0)
                    {
                        return;
                    }

                    ReserveSpace(value.Count * 2 * MAX_VARINT_SIZE);

                    int id = field.Index + INDEX2ID;
                    uint tag = (((uint)id << 3) | (uint)WireType.WIRETYPE_VARINT);

                    foreach (var v in value)
                    {
                        int intValue = 0;
                        MetaEnumEntry? entry = metaEnum.GetEntryByName(v);
                        if (entry != null)
                        {
                            intValue = entry.Id;
                        }
                        SerializeVarint(tag);
                        SerializeVarint((ulong)intValue);
                    }
                }
            }


            void SerializeString(int id, string value, bool ignoreZeroLength)
            {
                Debug.Assert(value.Length >= 0);
                if (value.Length == 0 && ignoreZeroLength)
                {
                    return;
                }
                
                byte[] stringAsBytes = Encoding.UTF8.GetBytes(value);
                int size = stringAsBytes.Length;

                ReserveSpace(MAX_VARINT_SIZE + MAX_VARINT_SIZE + size);

                uint tag = ((uint)id << 3) | (uint)WireType.WIRETYPE_LENGTH_DELIMITED;
                SerializeVarint(tag);
                SerializeVarint((ulong)size);
                Buffer.BlockCopy(stringAsBytes, 0, m_buffer, m_offset, size);
                m_offset += size;
            }
            void SerializeString(int id, byte[] value, int offset, int size, bool ignoreZeroLength)
            {
                Debug.Assert(size >= 0);
                if (size == 0 && ignoreZeroLength)
                {
                    return;
                }

                ReserveSpace(MAX_VARINT_SIZE + MAX_VARINT_SIZE + size);

                uint tag = ((uint)id << 3) | (uint)WireType.WIRETYPE_LENGTH_DELIMITED;
                SerializeVarint(tag);
                SerializeVarint((ulong)size);
                Buffer.BlockCopy(value, offset, m_buffer, m_offset, size);
                m_offset += size;
            }

            int SerializeStruct(int id)
            {
                ReserveSpace(MAX_VARINT_SIZE + RESERVE_STRUCT_SIZE);

                int bufferStructStart = m_offset;
                uint tag = ((uint)id << 3) | (uint)WireType.WIRETYPE_LENGTH_DELIMITED;
                SerializeVarint(tag);
                return bufferStructStart;
            }

            void SerializeVarintValue(int id, ulong value)
            {
                if (value == 0)
                {
                    return;
                }

                ReserveSpace(MAX_VARINT_SIZE + MAX_VARINT_SIZE);

                uint tag = ((uint)id << 3) | (uint)WireType.WIRETYPE_VARINT;
                SerializeVarint(tag);
                SerializeVarint(value);
            }

            void SerializeZigZagValue(int id, long value)
            {
                if (value == 0)
                {
                    return;
                }

                ulong v = zigzag(value);
                SerializeVarintValue(id, v);
            }

            ulong zigzag(long value)
            {
                return ((ulong)(value) << 1) ^ (ulong)(value >> 63);
            }


            void SerializeFixedUInt32(uint value)
            {
                m_buffer[m_offset] = (byte)value;
                ++m_offset;
                value >>= 8;
                m_buffer[m_offset] = (byte)value;
                ++m_offset;
                value >>= 8;
                m_buffer[m_offset] = (byte)value;
                ++m_offset;
                value >>= 8;
                m_buffer[m_offset] = (byte)value;
                ++m_offset;
            }

            void SerializeFixedUInt64(ulong value)
            {
                m_buffer[m_offset] = (byte)value;
                ++m_offset;
                value >>= 8;
                m_buffer[m_offset] = (byte)value;
                ++m_offset;
                value >>= 8;
                m_buffer[m_offset] = (byte)value;
                ++m_offset;
                value >>= 8;
                m_buffer[m_offset] = (byte)value;
                ++m_offset;
                value >>= 8;
                m_buffer[m_offset] = (byte)value;
                ++m_offset;
                value >>= 8;
                m_buffer[m_offset] = (byte)value;
                ++m_offset;
                value >>= 8;
                m_buffer[m_offset] = (byte)value;
                ++m_offset;
                value >>= 8;
                m_buffer[m_offset] = (byte)value;
                ++m_offset;
            }

            void SerializeFixedValueUInt32(int id, uint value)
            {
                if (value == 0)
                {
                    return;
                }

                ReserveSpace(MAX_VARINT_SIZE + sizeof(uint));

                uint tag = ((uint)id << 3) | (uint)WireType.WIRETYPE_FIXED32;
                SerializeVarint(tag);
                SerializeFixedUInt32(value);
            }

            void SerializeFixedValueUInt64(int id, ulong value)
            {
                if (value == 0)
                {
                    return;
                }

                ReserveSpace(MAX_VARINT_SIZE + sizeof(ulong));

                uint tag = ((uint)id << 3) | (uint)WireType.WIRETYPE_FIXED64;
                SerializeVarint(tag);
                SerializeFixedUInt64(value);
            }

            void SerializeArrayFixedUInt32<T>(int id, T[] value) where T : notnull
            {
                int size = value.Length;
                if (size <= 0)
                {
                    return;
                }

                int sizeByte = size * sizeof(uint);
                ReserveSpace(MAX_VARINT_SIZE + sizeByte);

                uint tag = ((uint)id << 3) | (uint)WireType.WIRETYPE_LENGTH_DELIMITED;
                SerializeVarint(tag);
                SerializeVarint((ulong)sizeByte);
                Debug.Assert(sizeByte <= m_bufferEnd - m_offset);
                if (BitConverter.IsLittleEndian)
                {
                    Buffer.BlockCopy(value, 0, m_buffer, m_offset, sizeByte);
                    m_offset += sizeByte;
                }
                else
                {
                    foreach (var v in value)
                    {
                        SerializeFixedUInt32((uint)(dynamic)v);
                    }
                }
            }

            void SerializeArrayFixedUInt64<T>(int id, T[] value) where T : notnull
            {
                int size = value.Length;
                if (size <= 0)
                {
                    return;
                }

                int sizeByte = size * sizeof(ulong);
                ReserveSpace(MAX_VARINT_SIZE + sizeByte);

                uint tag = ((uint)id << 3) | (uint)WireType.WIRETYPE_LENGTH_DELIMITED;
                SerializeVarint(tag);
                SerializeVarint((ulong)sizeByte);
                Debug.Assert(sizeByte <= m_bufferEnd - m_offset);
                if (BitConverter.IsLittleEndian)
                {
                    Buffer.BlockCopy(value, 0, m_buffer, m_offset, sizeByte);
                    m_offset += sizeByte;
                }
                else
                {
                    foreach (var v in value)
                    {
                        SerializeFixedUInt64((ulong)(dynamic)v);
                    }
                }
            }

            void SerializeArrayFixedFloat(int id, float[] value)
            {
                int size = value.Length;
                if (size <= 0)
                {
                    return;
                }

                int sizeByte = size * sizeof(float);
                ReserveSpace(MAX_VARINT_SIZE + sizeByte);

                uint tag = ((uint)id << 3) | (uint)WireType.WIRETYPE_LENGTH_DELIMITED;
                SerializeVarint(tag);
                SerializeVarint((ulong)sizeByte);
                Debug.Assert(sizeByte <= m_bufferEnd - m_offset);
                foreach (var v in value)
                {
                    uint fv = BitConverter.SingleToUInt32Bits(v);
                    SerializeFixedUInt32(fv);
                }
            }

            void SerializeArrayFixedDouble(int id, double[] value)
            {
                int size = value.Length;
                if (size <= 0)
                {
                    return;
                }

                int sizeByte = size * sizeof(double);
                ReserveSpace(MAX_VARINT_SIZE + sizeByte);

                uint tag = ((uint)id << 3) | (uint)WireType.WIRETYPE_LENGTH_DELIMITED;
                SerializeVarint(tag);
                SerializeVarint((ulong)sizeByte);
                Debug.Assert(sizeByte <= m_bufferEnd - m_offset);
                foreach (var v in value)
                {
                    ulong dv = BitConverter.DoubleToUInt64Bits(v);
                    SerializeFixedUInt64(dv);
                }
            }

            void SerializeArrayBool(int id, bool[] value)
            {
                if (value.Length <= 0)
                {
                    return;
                }

                int sizeByte = value.Length * 1;
                ReserveSpace(MAX_VARINT_SIZE + sizeByte);

                uint tag = ((uint)id << 3) | (uint)WireType.WIRETYPE_LENGTH_DELIMITED;
                SerializeVarint(tag);
                SerializeVarint((ulong)sizeByte);
                Debug.Assert(sizeByte <= m_bufferEnd - m_offset);
                foreach (var v in value)
                {
                    SerializeVarint((ulong)(v ? 1 : 0));
                }
            }
            void SerializeArrayString(int id, IList<string> value)
            {
                int size = value.Count;
                if (size <= 0)
                {
                    return;
                }

                foreach (var v in value)
                {
                    SerializeString(id, v, false);
                }
            }
            void SerializeArrayBytes(int id, IList<byte[]> value)
            {
                int size = value.Count;
                if (size <= 0)
                {
                    return;
                }

                foreach (var v in value)
                {
                    SerializeString(id, v, 0, v.Length, false);
                }
            }

            void SerializeArrayVarint<T>(int id, T[] value) where T : notnull
            {
                int size = value.Length;
                if (size <= 0)
                {
                    return;
                }

                ReserveSpace(size * 2 * MAX_VARINT_SIZE);

                uint tag = ((uint)id << 3) | (uint)WireType.WIRETYPE_VARINT;

                foreach (var v in value)
                {
                    SerializeVarint(tag);
                    SerializeVarint((ulong)(dynamic)v);
                }
            }

            void SerializeArrayZigZag<T>(int id, T[] value) where T : notnull
            {
                int size = value.Length;
                if (size <= 0)
                {
                    return;
                }

                ReserveSpace(size * 2 * MAX_VARINT_SIZE);

                uint tag = ((uint)id << 3) | (uint)WireType.WIRETYPE_VARINT;

                foreach (var v in value)
                {
                    SerializeVarint(tag);
                    ulong vzigzag = zigzag((long)(dynamic)v);
                    SerializeVarint(vzigzag);
                }
            }

            void SerializeVarint(ulong value)
            {
                while (value >= 0x80)
                {
                    m_buffer[m_offset] = (byte)(value | 0x80);
                    value >>= 7;
                    ++m_offset;
                }
                m_buffer[m_offset] = (byte)value;
                ++m_offset;
            }
            void ReserveSpace(int space)
            {
                int sizeRemaining = m_bufferEnd - m_offset;
                if (sizeRemaining < space)
                {
                    if (m_offset != -1)
                    {
                        int size = m_offset - m_bufferStart;
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

                    if (m_offset != -1)
                    {
                        foreach (var structData in m_stackStruct)
                        {
                            if (structData.offset != -1)
                            {
                                structData.allocateNextDataBuffer = true;
                                int sizeStruct = m_offset - structData.offset;
                                Debug.Assert(sizeStruct >= 0);
                                structData.size += sizeStruct;
                                structData.offset = bufferStartNew.Offset;
                            }
                        }
                    }

                    m_bufferStart = bufferStartNew.Offset;
                    m_bufferEnd = m_bufferStart + sizeNew;
                    m_offset = m_bufferStart;
                    m_buffer = bufferStartNew.Buffer;
                }
            }
            void ResizeBuffer()
            {
                if (m_offset != -1)
                {
                    int size = m_offset - m_bufferStart;
                    Debug.Assert(size >= 0);
                    m_zeroCopybuffer.DownsizeLastBuffer(size);
                    m_bufferStart = -1;
                    m_bufferEnd = -1;
                    m_offset = -1;
                }
            }
            int CalculateStructSize(ref int structSize)
            {
                int remainingSize = RESERVE_STRUCT_SIZE - 1;
                if (structSize + (RESERVE_STRUCT_SIZE - 1) < 128)
                {
                    remainingSize = RESERVE_STRUCT_SIZE - 1;
                    structSize += RESERVE_STRUCT_SIZE - 1;
                }
                else if (structSize + (RESERVE_STRUCT_SIZE - 1) == 128)
                {
                    SerializeVarint(tagDummy);
                    SerializeVarint(1);
                    remainingSize = RESERVE_STRUCT_SIZE - 2;
                    structSize += 3 + RESERVE_STRUCT_SIZE - 2;
                }
                else if (structSize + (RESERVE_STRUCT_SIZE - 2) < 16384)
                {
                    remainingSize = RESERVE_STRUCT_SIZE - 2;
                    structSize += RESERVE_STRUCT_SIZE - 2;
                }
                else if (structSize + (RESERVE_STRUCT_SIZE - 2) == 16384)
                {
                    SerializeVarint(tagDummy);
                    SerializeVarint(1);
                    remainingSize = RESERVE_STRUCT_SIZE - 3;
                    structSize += 3 + RESERVE_STRUCT_SIZE - 3;
                }
                else if (structSize + (RESERVE_STRUCT_SIZE - 3) < 2097152)
                {
                    remainingSize = RESERVE_STRUCT_SIZE - 3;
                    structSize += RESERVE_STRUCT_SIZE - 3;
                }
                else if (structSize + (RESERVE_STRUCT_SIZE - 3) == 2097152)
                {
                    SerializeVarint(tagDummy);
                    SerializeVarint(1);
                    remainingSize = RESERVE_STRUCT_SIZE - 4;
                    structSize += 3 + RESERVE_STRUCT_SIZE - 4;
                }
                else if (structSize + (RESERVE_STRUCT_SIZE - 4) < 268435456)
                {
                    remainingSize = RESERVE_STRUCT_SIZE - 4;
                    structSize += RESERVE_STRUCT_SIZE - 4;
                }
                else if (structSize + (RESERVE_STRUCT_SIZE - 4) == 268435456)
                {
                    SerializeVarint(tagDummy);
                    SerializeVarint(1);
                    remainingSize = RESERVE_STRUCT_SIZE - 5;
                    structSize += 3 + RESERVE_STRUCT_SIZE - 5;
                }
                else
                {
                    remainingSize = RESERVE_STRUCT_SIZE - 5;
                    structSize += RESERVE_STRUCT_SIZE - 5;
                }

                return remainingSize;
            }
            void FillRemainingStruct(int remainingSize)
            {
                switch (remainingSize)
                {
                    case 1:
                        SerializeVarint(1);
                        break;
                    case 2:
                        SerializeVarint(128);
                        break;
                    case 3:
                        SerializeVarint(16384);
                        break;
                    case 4:
                        SerializeVarint(2097152);
                        break;
                    case 5:
                        SerializeVarint(268435456);
                        break;
                    default:
                        Debug.Assert(false);
                        break;
                }
            }

            class StructData
            {
                public StructData(byte[] bufstart, int offstart, int offsize, int offs, bool ae)
                {
                    bufferStructStart = bufstart;
                    offsetStructStart = offstart;
                    offsetStructSize = offsize;
                    offset = offs;
                    arrayEntry = ae;
                }
                public byte[] bufferStructStart;
                public int offsetStructStart;
                public int offsetStructSize;
                public int offset;
                public int size = 0;
                public bool allocateNextDataBuffer = false;
                public bool arrayParent = false;
                public bool arrayEntry;
            };

            readonly IZeroCopyBuffer m_zeroCopybuffer;
            readonly int m_maxBlockSize = 512;
            int m_bufferStart = -1;
            byte[] m_buffer = Array.Empty<byte>();
            int m_offset = -1;
            int m_bufferEnd = -1;
            bool m_arrayParent = false;
            readonly IList<StructData> m_stackStruct = new List<StructData>();            
        }

        readonly Internal m_internal;
    }

}
