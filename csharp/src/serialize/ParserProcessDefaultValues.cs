﻿using System;
using System.Collections.Generic;
using System.Diagnostics;


namespace finalmq
{

    class ParserProcessDefaultValues : IParserVisitor
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
        public void resetVarValueActive()
        {
            m_varValueActive = 0;
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
            m_stackFieldsDone.Add(new bool[stru.FieldsSize]);
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
            m_visitor.Finished();
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
                        m_stackFieldsDone.Add(EmptyArrayBool);
                    }
                }
                else
                {
                    m_varValueActive++;
                }
            }

            m_visitor.EnterStruct(field);
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

            m_visitor.ExitStruct(field);
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
                            m_visitor.EnterString(field, EmptyString);
                            break;
                        case MetaTypeId.TYPE_BYTES:
                            m_visitor.EnterBytes(field, EmptyBytes);
                            break;
                        case MetaTypeId.TYPE_STRUCT:
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
                            break;
                        case MetaTypeId.TYPE_ENUM:
                            m_visitor.EnterEnum(field, 0);
                            break;
                        case MetaTypeId.TYPE_ARRAY_BOOL:
                            m_visitor.EnterArrayBool(field, EmptyArrayBool);
                            break;
                        case MetaTypeId.TYPE_ARRAY_INT32:
                            m_visitor.EnterArrayInt32(field, EmptyArrayInt32);
                            break;
                        case MetaTypeId.TYPE_ARRAY_UINT32:
                            m_visitor.EnterArrayUInt32(field, EmptyArrayUInt32);
                            break;
                        case MetaTypeId.TYPE_ARRAY_INT64:
                            m_visitor.EnterArrayInt64(field, EmptyArrayInt64);
                            break;
                        case MetaTypeId.TYPE_ARRAY_UINT64:
                            m_visitor.EnterArrayUInt64(field, EmptyArrayUInt64);
                            break;
                        case MetaTypeId.TYPE_ARRAY_FLOAT:
                            m_visitor.EnterArrayFloat(field, EmptyArrayFloat);
                            break;
                        case MetaTypeId.TYPE_ARRAY_DOUBLE:
                            m_visitor.EnterArrayDouble(field, EmptyArrayDouble);
                            break;
                        case MetaTypeId.TYPE_ARRAY_STRING:
                            m_visitor.EnterArrayString(field, EmptyArrayString);
                            break;
                        case MetaTypeId.TYPE_ARRAY_BYTES:
                            m_visitor.EnterArrayBytes(field, EmptyArrayBytes);
                            break;
                        case MetaTypeId.TYPE_ARRAY_STRUCT:
                            m_visitor.EnterArrayStruct(field);
                            m_visitor.ExitArrayStruct(field);
                            break;
                        case MetaTypeId.TYPE_ARRAY_ENUM:
                            m_visitor.EnterArrayEnum(field, EmptyArrayInt32);
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
            m_visitor.EnterArrayStruct(field);
        }
        public void ExitArrayStruct(MetaField field)
        {
            Debug.Assert(m_visitor != null);
            m_visitor.ExitArrayStruct(field);
        }

        public void EnterBool(MetaField field, bool value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != false || !m_skipDefaultValues)
            {
                m_visitor.EnterBool(field, value);
            }
        }
        public void EnterInt32(MetaField field, int value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterInt32(field, value);
            }
        }
        public void EnterUInt32(MetaField field, uint value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterUInt32(field, value);
            }
        }
        public void EnterInt64(MetaField field, long value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterInt64(field, value);
            }
        }
        public void EnterUInt64(MetaField field, ulong value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterUInt64(field, value);
            }
        }
        public void EnterFloat(MetaField field, float value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0.0f || !m_skipDefaultValues)
            {
                m_visitor.EnterFloat(field, value);
            }
        }
        public void EnterDouble(MetaField field, double value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0.0 || !m_skipDefaultValues)
            {
                m_visitor.EnterDouble(field, value);
            }
        }
        public void EnterString(MetaField field, string value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterString(field, value);
            }
        }
        public void EnterBytes(MetaField field, byte[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterBytes(field, value);
            }
        }
        public void EnterEnum(MetaField field, int value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value != 0 || !m_skipDefaultValues)
            {
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
                m_visitor.EnterEnum(field, value);
            }
        }

        public void EnterArrayBool(MetaField field, bool[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterArrayBool(field, value);
            }
        }
        public void EnterArrayInt32(MetaField field, int[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterArrayInt32(field, value);
            }
        }
        public void EnterArrayUInt32(MetaField field, uint[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterArrayUInt32(field, value);
            }
        }
        public void EnterArrayInt64(MetaField field, long[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterArrayInt64(field, value);
            }
        }
        public void EnterArrayUInt64(MetaField field, ulong[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterArrayUInt64(field, value);
            }
        }
        public void EnterArrayFloat(MetaField field, float[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterArrayFloat(field, value);
            }
        }
        public void EnterArrayDouble(MetaField field, double[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterArrayDouble(field, value);
            }
        }
        public void EnterArrayString(MetaField field, string[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterArrayString(field, value);
            }
        }
        public void EnterArrayBytes(MetaField field, byte[][] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterArrayBytes(field, value);
            }
        }
        public void EnterArrayEnum(MetaField field, int[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterArrayEnum(field, value);
            }
        }
        public void EnterArrayEnum(MetaField field, string[] value)
        {
            Debug.Assert(m_visitor != null);
            MarkAsDone(field);
            if (value.Length != 0 || !m_skipDefaultValues)
            {
                m_visitor.EnterArrayEnum(field, value);
            }
        }

        IParserVisitor? m_visitor = null;
        bool m_skipDefaultValues = true;
        MetaStruct? m_struct = null;
        readonly IList<bool[]> m_stackFieldsDone = new List<bool[]>();
        int m_varValueActive = 0;

        static readonly string EmptyString = "";
        static readonly byte[] EmptyBytes = new byte[0];
        static readonly bool[] EmptyArrayBool = new bool[0];
        static readonly int[] EmptyArrayInt32 = new int[0];
        static readonly uint[] EmptyArrayUInt32 = new uint[0];
        static readonly long[] EmptyArrayInt64 = new long[0];
        static readonly ulong[] EmptyArrayUInt64 = new ulong[0];
        static readonly float[] EmptyArrayFloat = new float[0];
        static readonly double[] EmptyArrayDouble = new double[0];
        static readonly string[] EmptyArrayString = new string[0];
        static readonly byte[][] EmptyArrayBytes = new byte[0][];
    }

}