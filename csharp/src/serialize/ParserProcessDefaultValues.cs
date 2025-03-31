using System;
using System.Collections.Generic;
using System.Diagnostics;


namespace finalmq
{

    public class ParserProcessDefaultValues : IParserVisitor
    {
        private readonly string STR_VARVALUE = "finalmq.variant.VarValue";
        public ParserProcessDefaultValues(bool skipDefaultValues, IParserVisitor? visitor = null)
        {
            m_visitor = visitor;
            m_skipDefaultValues = skipDefaultValues;
        }
        public void SetVisitor(IParserVisitor visitor)
        {
            m_visitor = visitor;
        }
        public void ResetVarValueActive()
        {
            if (m_varValueActive > 0)
            {
                m_stackFieldsDone.RemoveAt(m_stackFieldsDone.Count - 1);
                m_varValueActive = 0;
            }
        }

        // IParserVisitor
        public void NotifyError(string str, string message)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.NotifyError(str, message);
        }
        public void StartStruct(MetaStruct stru)
        {
            Debug.Assert(m_visitor != null);
            m_struct = stru;
            if (!m_skipDefaultValues)
            {
                m_stackFieldsDone.Add(new bool[stru.FieldsSize]);
            }
            else
            {
                m_stackSkipDefault.Add(new EntrySkipDefault(null, true));
            }
            m_visitor.StartStruct(stru);
        }
        public void Finished()
        {
            Debug.Assert(m_visitor != null);
            if (!m_skipDefaultValues && m_struct != null)
            {
                Debug.Assert(m_stackFieldsDone.Count != 0);
                bool[] fieldsDone = m_stackFieldsDone.Last();
                ProcessDefaultValues(m_struct, fieldsDone);
                m_stackFieldsDone.RemoveAt(m_stackFieldsDone.Count - 1);
            }

            if (m_stackSkipDefault.Count != 0)
            {
                m_stackSkipDefault.RemoveAt(m_stackSkipDefault.Count - 1);
            }

            m_visitor.Finished();
        }

        void ExecuteEnterStruct()
        {
            Debug.Assert(m_visitor != null);

            for (int i = m_stackSkipDefault.Count - 1; i >= 0; --i)
            {
                EntrySkipDefault entry1 = m_stackSkipDefault[i];
                if (entry1.EnterStructCalled)
                {
                    ++i;
                    for (; i < m_stackSkipDefault.Count; ++i)
                    {
                        EntrySkipDefault entry2 = m_stackSkipDefault[i];
                        Debug.Assert(entry2.Field != null);
                        m_visitor.EnterStruct(entry2.Field);
                        entry2.EnterStructCalled = true;
                    }
                    break;
                }
            }
        }

        public void EnterStruct(MetaField field)
        {
            Debug.Assert(m_visitor != null);

            if (!m_skipDefaultValues)
            {
                MarkAsDone(field);
                if (field.TypeName != STR_VARVALUE)
                {
                    MetaStruct? stru = MetaDataGlobal.Instance.GetStruct(field);
                    if (stru != null)
                    {
                        m_stackFieldsDone.Add(new bool[stru.FieldsSize]);
                    }
                    else
                    {
                        m_stackFieldsDone.Add(Array.Empty<bool>());
                    }
                }
                else
                {
                    m_varValueActive++;
                }
                m_visitor.EnterStruct(field);
            }
            else
            {
                if (m_stackSkipDefault.Count != 0)
                {
                    var entry = m_stackSkipDefault.Last();
                    if (entry.FieldArrayStruct != null)
                    {
                        if (!entry.EnterArrayStructCalled)
                        {
                            ExecuteEnterStruct();
                            m_visitor.EnterArrayStruct(entry.FieldArrayStruct);
                            entry.EnterArrayStructCalled = true;
                        }
                        m_visitor.EnterStruct(field);
                        m_stackSkipDefault.Add(new EntrySkipDefault(field, true));
                    }
                    else
                    {
                        m_stackSkipDefault.Add(new EntrySkipDefault(field, false));
                    }
                }
            }
        }
        
        public void ExitStruct(MetaField field)
        {
            Debug.Assert(m_visitor != null);

            if (!m_skipDefaultValues && m_varValueActive == 0)
            {
                Debug.Assert(m_stackFieldsDone.Count != 0);
                bool[] fieldsDone = m_stackFieldsDone.Last();
                MetaStruct? stru = MetaDataGlobal.Instance.GetStruct(field);
                if (stru != null)
                {
                    ProcessDefaultValues(stru, fieldsDone);
                }

                m_stackFieldsDone.RemoveAt(m_stackFieldsDone.Count - 1);
            }
            if (m_varValueActive > 0)
            {
                m_varValueActive--;
            }

            if (!m_skipDefaultValues)
            {
                m_visitor.ExitStruct(field);
            }
            else
            {
                if (m_stackSkipDefault.Count != 0)
                {
                    var entry = m_stackSkipDefault.Last();
                    if (entry.EnterStructCalled)
                    {
                        m_visitor.ExitStruct(field);
                    }
                    m_stackSkipDefault.RemoveAt(m_stackSkipDefault.Count - 1);
                }
            }
        }

        public void EnterStructNull(MetaField field)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (!m_skipDefaultValues)
            {
                m_visitor.EnterStructNull(field);
            }
        }

        void ProcessDefaultValues(MetaStruct stru, bool[] fieldsDone)
        {
            Debug.Assert(m_visitor != null);
            Debug.Assert(!m_skipDefaultValues);
            for (int index = 0; index<fieldsDone.Length; ++index)
            {
                if (!fieldsDone[index])
                {
                    MetaField? field = stru.GetFieldByIndex(index);
                    if (field != null)
                    {
                        switch (field.TypeId)
                        {
                        case MetaTypeId.TYPE_NONE:
                            break;
                        case MetaTypeId.TYPE_BOOL:
                            m_visitor.EnterBool(field, false);
                            break;
                        case MetaTypeId.TYPE_INT8:
                            m_visitor.EnterInt8(field, 0);
                            break;
                        case MetaTypeId.TYPE_UINT8:
                            m_visitor.EnterUInt8(field, 0);
                            break;
                        case MetaTypeId.TYPE_INT16:
                            m_visitor.EnterInt16(field, 0);
                            break;
                        case MetaTypeId.TYPE_UINT16:
                            m_visitor.EnterUInt16(field, 0);
                            break;
                        case MetaTypeId.TYPE_INT32:
                            m_visitor.EnterInt32(field, 0);
                            break;
                        case MetaTypeId.TYPE_UINT32:
                            m_visitor.EnterUInt32(field, 0);
                            break;
                        case MetaTypeId.TYPE_INT64:
                            m_visitor.EnterInt64(field, 0);
                            break;
                        case MetaTypeId.TYPE_UINT64:
                            m_visitor.EnterUInt64(field, 0);
                            break;
                        case MetaTypeId.TYPE_FLOAT:
                            m_visitor.EnterFloat(field, 0.0f);
                            break;
                        case MetaTypeId.TYPE_DOUBLE:
                            m_visitor.EnterDouble(field, 0.0);
                            break;
                        case MetaTypeId.TYPE_STRING:
                            m_visitor.EnterString(field, String.Empty);
                            break;
                        case MetaTypeId.TYPE_BYTES:
                            m_visitor.EnterBytes(field, Array.Empty<byte>(), 0, 0);
                            break;
                        case MetaTypeId.TYPE_STRUCT:
                            if ((field.Flags & (int)MetaFieldFlags.METAFLAG_NULLABLE) == 0)
                            {
                                m_visitor.EnterStruct(field);
                                MetaStruct? substru = MetaDataGlobal.Instance.GetStruct(field);
                                if ((substru != null) && (field.TypeName != STR_VARVALUE))
                                {
                                    bool[] subfieldsDone = new bool[substru.FieldsSize];
                                    ProcessDefaultValues(substru, subfieldsDone);
                                }
                                m_visitor.ExitStruct(field);
                            }
                            else
                            {
                                m_visitor.EnterStructNull(field);
                            }
                            break;
                        case MetaTypeId.TYPE_ENUM:
                            m_visitor.EnterEnum(field, 0);
                            break;
                        case MetaTypeId.TYPE_ARRAY_BOOL:
                            m_visitor.EnterArrayBool(field, Array.Empty<bool>());
                            break;
                        case MetaTypeId.TYPE_ARRAY_INT8:
                            m_visitor.EnterArrayInt8(field, Array.Empty<sbyte>());
                            break;
                        case MetaTypeId.TYPE_ARRAY_INT16:
                            m_visitor.EnterArrayInt16(field, Array.Empty<short>());
                            break;
                        case MetaTypeId.TYPE_ARRAY_UINT16:
                            m_visitor.EnterArrayUInt16(field, Array.Empty<ushort>());
                            break;
                        case MetaTypeId.TYPE_ARRAY_INT32:
                            m_visitor.EnterArrayInt32(field, Array.Empty<int>());
                            break;
                        case MetaTypeId.TYPE_ARRAY_UINT32:
                            m_visitor.EnterArrayUInt32(field, Array.Empty<uint>());
                            break;
                        case MetaTypeId.TYPE_ARRAY_INT64:
                            m_visitor.EnterArrayInt64(field, Array.Empty<long>());
                            break;
                        case MetaTypeId.TYPE_ARRAY_UINT64:
                            m_visitor.EnterArrayUInt64(field, Array.Empty<ulong>());
                            break;
                        case MetaTypeId.TYPE_ARRAY_FLOAT:
                            m_visitor.EnterArrayFloat(field, Array.Empty<float>());
                            break;
                        case MetaTypeId.TYPE_ARRAY_DOUBLE:
                            m_visitor.EnterArrayDouble(field, Array.Empty<double>());
                            break;
                        case MetaTypeId.TYPE_ARRAY_STRING:
                            m_visitor.EnterArrayString(field, new List<string>());
                            break;
                        case MetaTypeId.TYPE_ARRAY_BYTES:
                            m_visitor.EnterArrayBytes(field, new List<byte[]>());
                            break;
                        case MetaTypeId.TYPE_ARRAY_STRUCT:
                            m_visitor.EnterArrayStruct(field);
                            m_visitor.ExitArrayStruct(field);
                            break;
                        case MetaTypeId.TYPE_ARRAY_ENUM:
                            m_visitor.EnterArrayEnum(field, Array.Empty<int>());
                            break;
                        case MetaTypeId.OFFSET_ARRAY_FLAG:
                            Debug.Assert(false);
                            break;
                        default:
                            Debug.Assert(false);
                            break;
                        }
                    }
                }
            }
        }


        void MarkAsDone(MetaField field)
        {
            if (!m_skipDefaultValues && m_varValueActive == 0)
            {
                Debug.Assert(m_stackFieldsDone.Count != 0);
                bool[] fieldsDone = m_stackFieldsDone.Last();
                int index = field.Index;
                if (index >= 0 && index < fieldsDone.Length)
                {
                    fieldsDone[index] = true;
                }
            }
        }

        public void EnterArrayStruct(MetaField field)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (!m_skipDefaultValues)
            {
                m_visitor.EnterArrayStruct(field);
            }
            else
            {
                if (m_stackSkipDefault.Count != 0)
                {
                    // call enterArrayStruct before the first element at enterStruct
                    var entry = m_stackSkipDefault.Last();
                    entry.FieldArrayStruct = field;
                    entry.EnterArrayStructCalled = false;
                }
            }
        }
        public void ExitArrayStruct(MetaField field)
        {
            Debug.Assert(m_visitor != null);
            if (!m_skipDefaultValues)
            {
                m_visitor.ExitArrayStruct(field);
            }
            else
            {
                if (m_stackSkipDefault.Count != 0)
                {
                    var entry = m_stackSkipDefault.Last();
                    if (entry.EnterArrayStructCalled)
                    {
                        m_visitor.ExitArrayStruct(field);
                    }
                    entry.FieldArrayStruct = null;
                    entry.EnterArrayStructCalled = false;
                }
            }
        }

        public void EnterBool(MetaField field, bool value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != false || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterBool(field, value);
            }
        }
        public void EnterInt8(MetaField field, sbyte value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterInt8(field, value);
            }
        }
        public void EnterUInt8(MetaField field, byte value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterUInt8(field, value);
            }
        }
        public void EnterInt16(MetaField field, short value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterInt16(field, value);
            }
        }
        public void EnterUInt16(MetaField field, ushort value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterUInt16(field, value);
            }
        }
        public void EnterInt32(MetaField field, int value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterInt32(field, value);
            }
        }
        public void EnterUInt32(MetaField field, uint value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterUInt32(field, value);
            }
        }
        public void EnterInt64(MetaField field, long value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterInt64(field, value);
            }
        }
        public void EnterUInt64(MetaField field, ulong value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterUInt64(field, value);
            }
        }
        public void EnterFloat(MetaField field, float value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0.0f || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterFloat(field, value);
            }
        }
        public void EnterDouble(MetaField field, double value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0.0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterDouble(field, value);
            }
        }
        public void EnterString(MetaField field, string value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterString(field, value);
            }
        }
        public void EnterString(MetaField field, byte[] buffer, int offset, int size)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (size != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterString(field, buffer, offset, size);
            }
        }
        public void EnterBytes(MetaField field, byte[] value, int offset, int size)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (size != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterBytes(field, value, offset, size);
            }
        }
        public void EnterEnum(MetaField field, int value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterEnum(field, value);
            }
        }
        public void EnterEnum(MetaField field, string value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            int v = MetaDataGlobal.Instance.GetEnumValueByName(field, value);
            if (v != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterEnum(field, value);
            }
        }

        public void EnterArrayBool(MetaField field, bool[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterArrayBool(field, value);
            }
        }
        public void EnterArrayInt8(MetaField field, sbyte[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterArrayInt8(field, value);
            }
        }
        public void EnterArrayInt16(MetaField field, short[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterArrayInt16(field, value);
            }
        }
        public void EnterArrayUInt16(MetaField field, ushort[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterArrayUInt16(field, value);
            }
        }
        public void EnterArrayInt32(MetaField field, int[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterArrayInt32(field, value);
            }
        }
        public void EnterArrayUInt32(MetaField field, uint[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterArrayUInt32(field, value);
            }
        }
        public void EnterArrayInt64(MetaField field, long[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterArrayInt64(field, value);
            }
        }
        public void EnterArrayUInt64(MetaField field, ulong[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterArrayUInt64(field, value);
            }
        }
        public void EnterArrayFloat(MetaField field, float[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterArrayFloat(field, value);
            }
        }
        public void EnterArrayDouble(MetaField field, double[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterArrayDouble(field, value);
            }
        }
        public void EnterArrayString(MetaField field, IList<string> value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Count != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterArrayString(field, value);
            }
        }
        public void EnterArrayBytes(MetaField field, IList<byte[]> value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Count != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterArrayBytes(field, value);
            }
        }
        public void EnterArrayEnum(MetaField field, int[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterArrayEnum(field, value);
            }
        }
        public void EnterArrayEnum(MetaField field, IList<string> value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Count != 0 || !m_skipDefaultValues)
            {
                if (m_skipDefaultValues)
                {
                    ExecuteEnterStruct();
                }
                m_visitor.EnterArrayEnum(field, value);
            }
        }

        class EntrySkipDefault
        {
            public EntrySkipDefault(MetaField? field, bool enterStructCalled)
            {
                m_field = field;
                m_fieldArrayStruct = null;
                m_enterArrayStructCalled = false;
                m_countMember = 0;
                m_enterStructCalled = enterStructCalled;
            }

            public MetaField? Field
            {
                get { return m_field; }
                set { m_field = value; }
            }

            public MetaField? FieldArrayStruct
            {
                get { return m_fieldArrayStruct; }
                set { m_fieldArrayStruct = value; }
            }

            public bool EnterArrayStructCalled
            {
                get { return m_enterArrayStructCalled; }
                set { m_enterArrayStructCalled = value; }
            }

            public int CountMember
            {
                get { return m_countMember; }
                set { m_countMember = value; }
            }

            public bool EnterStructCalled
            {
                get { return m_enterStructCalled; }
                set { m_enterStructCalled = value; }
            }

            MetaField? m_field;
            MetaField? m_fieldArrayStruct;
            bool m_enterArrayStructCalled;
            int m_countMember;
            bool m_enterStructCalled;
        }
        
        IParserVisitor? m_visitor = null;
        bool m_skipDefaultValues = true;
        MetaStruct? m_struct = null;
        readonly IList<bool[]> m_stackFieldsDone = new List<bool[]>();
        int m_varValueActive = 0;
        readonly IList<EntrySkipDefault> m_stackSkipDefault = new List<EntrySkipDefault>();
    }

}
